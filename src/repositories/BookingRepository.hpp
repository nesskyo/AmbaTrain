#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <pqxx/pqxx>
#include "../models/Booking.hpp"
#include "../config/DatabasePool.hpp"

namespace ambatrain::repositories {

class BookingRepository {
public:
    explicit BookingRepository(std::shared_ptr<config::DatabasePool> db_pool)
        : db_pool_(std::move(db_pool)) {}

    // Generate unique Sigil code (e.g. AMB-7K3QX9)
    static std::string generate_sigil_code() {
        const char charset[] = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ"; // exclude easily confused chars like 0, 1, I, O
        std::random_device rd;
        std::mt19937 gen(rd() ^ std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<size_t> dis(0, sizeof(charset) - 2);

        std::string code = "AMB-";
        for (int i = 0; i < 6; ++i) {
            code += charset[dis(gen)];
        }
        return code;
    }

    // Anti Double-Booking Transactional Hold (PRD Section 11.3)
    models::BookingResponse create_booking_hold(
        const std::string& user_id,
        int64_t schedule_id,
        const std::vector<models::PassengerInput>& passengers,
        int hold_minutes = 15
    ) {
        if (!db_pool_) throw std::runtime_error("Database pool is offline.");
        if (passengers.empty()) throw std::invalid_argument("Daftar penumpang tidak boleh kosong.");

        auto conn = db_pool_->acquire();
        // Open isolated database transaction
        pqxx::work txn(conn.get());

        // 1. Get schedule price
        pqxx::result sched_res = txn.exec(
            "SELECT price::float8 FROM public.schedules WHERE id = " + std::to_string(schedule_id) + " LIMIT 1;"
        );
        if (sched_res.empty()) {
            throw std::runtime_error("Jadwal perjalanan tidak ditemukan.");
        }
        double unit_price = sched_res[0]["price"].as<double>();
        double total_price = unit_price * passengers.size();

        // 2. Build list of requested seat IDs
        std::string seat_ids_csv;
        for (size_t i = 0; i < passengers.size(); ++i) {
            if (i > 0) seat_ids_csv += ",";
            seat_ids_csv += std::to_string(passengers[i].seat_id);
        }

        // 3. PESSIMISTIC ROW-LEVEL LOCKING: Lock candidate seat rows
        // This forces competing transactions for the same seats to wait until this transaction commits/aborts!
        txn.exec("SELECT id FROM public.seats WHERE id IN (" + seat_ids_csv + ") FOR UPDATE;");

        // 4. Verify none of the seats are actively held or paid
        std::string check_query = 
            "SELECT bp.seat_id, s.seat_number "
            "FROM public.booking_passengers bp "
            "JOIN public.seats s ON bp.seat_id = s.id "
            "JOIN public.bookings b ON bp.booking_id = b.id "
            "WHERE bp.schedule_id = " + std::to_string(schedule_id) + " "
            "  AND bp.seat_id IN (" + seat_ids_csv + ") "
            "  AND bp.seat_active = TRUE "
            "  AND (b.status = 'paid' OR (b.status = 'held' AND b.expires_at > NOW())) "
            "LIMIT 1;";

        pqxx::result conflict_res = txn.exec(check_query);
        if (!conflict_res.empty()) {
            std::string taken_seat = conflict_res[0]["seat_number"].as<std::string>();
            txn.abort(); // Rollback immediately
            throw std::runtime_error("SEAT_COLLISION: Kursi " + taken_seat + " telah direbut takdir lain.");
        }

        // 5. Generate Sigil & Insert booking
        std::string sigil_code = generate_sigil_code();
        std::string insert_booking = 
            "INSERT INTO public.bookings (code, user_id, schedule_id, status, total_price, expires_at) "
            "VALUES (" 
            + txn.quote(sigil_code) + ", "
            + txn.quote(user_id) + "::uuid, "
            + std::to_string(schedule_id) + ", "
            + "'held', "
            + std::to_string(total_price) + ", "
            + "NOW() + INTERVAL '" + std::to_string(hold_minutes) + " minutes') "
            "RETURNING id, code, status, total_price::float8, to_char(expires_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS expires_at;";

        pqxx::result book_res = txn.exec(insert_booking);
        int64_t booking_id = book_res[0]["id"].as<int64_t>();

        // 6. Insert passengers and lock active seats
        // Partial unique index 'uq_active_seat_per_schedule' acts as ultimate hardware-level guarantee!
        try {
            for (const auto& pax : passengers) {
                std::string insert_pax = 
                    "INSERT INTO public.booking_passengers (booking_id, schedule_id, seat_id, name, id_number, seat_active) "
                    "VALUES ("
                    + std::to_string(booking_id) + ", "
                    + std::to_string(schedule_id) + ", "
                    + std::to_string(pax.seat_id) + ", "
                    + txn.quote(pax.name) + ", "
                    + txn.quote(pax.id_number) + ", "
                    + "TRUE);";
                txn.exec0(insert_pax);
            }
        } catch (const pqxx::unique_violation& e) {
            txn.abort();
            throw std::runtime_error("SEAT_COLLISION: Kursi telah diambil bersamaan oleh musafir lain.");
        }

        // Commit transaction
        txn.commit();

        models::BookingResponse response;
        response.id = booking_id;
        response.code = book_res[0]["code"].as<std::string>();
        response.status = book_res[0]["status"].as<std::string>();
        response.total_price = book_res[0]["total_price"].as<double>();
        response.expires_at = book_res[0]["expires_at"].as<std::string>();

        return response;
    }

    // Call stored procedure to release expired holds
    int expire_held_bookings() {
        if (!db_pool_) return 0;
        try {
            auto conn = db_pool_->acquire();
            pqxx::work txn(conn.get());
            pqxx::result res = txn.exec("SELECT public.expire_held_bookings() AS count;");
            txn.commit();
            return res.empty() ? 0 : res[0]["count"].as<int>();
        } catch (...) {
            return 0;
        }
    }

private:
    std::shared_ptr<config::DatabasePool> db_pool_;
};

} // namespace ambatrain::repositories
