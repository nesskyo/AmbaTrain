#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>
#include "../config/DatabasePool.hpp"
#include "../models/Payment.hpp"

namespace ambatrain::repositories {

struct RoyalPassDetail {
    int64_t booking_id = 0;
    std::string sigil_code;
    std::string status;
    double total_price = 0.0;
    std::string booked_at;
    std::string expires_at;
    std::string user_name;
    std::string user_email;
    std::string train_name;
    std::string train_class;
    std::string origin_station;
    std::string origin_code;
    std::string origin_city;
    std::string destination_station;
    std::string destination_code;
    std::string destination_city;
    std::string departure_at;
    std::string arrival_at;
    std::string payment_method;
    std::string paid_at;
    nlohmann::json passengers = nlohmann::json::array();
};

struct ChronicleItem {
    int64_t booking_id = 0;
    std::string sigil_code;
    std::string status;
    double total_price = 0.0;
    std::string booked_at;
    std::string train_name;
    std::string train_class;
    std::string origin_city;
    std::string destination_city;
    std::string departure_at;
    std::string arrival_at;
    int passenger_count = 0;
};

class PaymentRepository {
public:
    explicit PaymentRepository(std::shared_ptr<config::DatabasePool> db_pool)
        : db_pool_(std::move(db_pool)) {}

    // 1. Pay Booking (Simulasi Tribute)
    models::PaymentResult pay_booking(
        int64_t booking_id, 
        const std::string& user_id, 
        const std::string& method
    ) {
        if (!db_pool_) throw std::runtime_error("Database pool offline.");

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        // Check if booking exists, owned by user, and currently held
        std::string check_q = 
            "SELECT id, total_price::float8, status, expires_at "
            "FROM public.bookings "
            "WHERE id = " + std::to_string(booking_id) + " "
            "  AND user_id = " + txn.quote(user_id) + "::uuid "
            "FOR UPDATE;";

        pqxx::result check_res = txn.exec(check_q);
        if (check_res.empty()) {
            txn.abort();
            throw std::runtime_error("Pesanan tiada ditemukan dalam catatanmu.");
        }

        std::string status = check_res[0]["status"].as<std::string>();
        double amount = check_res[0]["total_price"].as<double>();

        if (status == "paid") {
            txn.abort();
            throw std::runtime_error("Pesanan ini telah terbayar lunas sebelumnya (Sealed).");
        }
        if (status == "cancelled") {
            txn.abort();
            throw std::runtime_error("Pesanan ini telah dibatalkan (Forsaken).");
        }
        if (status == "expired") {
            txn.abort();
            throw std::runtime_error("Waktu pembayaran telah habis (Faded to Dust).");
        }

        // Update booking status to 'paid'
        std::string update_booking = 
            "UPDATE public.bookings "
            "SET status = 'paid', updated_at = NOW() "
            "WHERE id = " + std::to_string(booking_id) + ";";
        txn.exec0(update_booking);

        // Insert payment record
        std::string insert_payment = 
            "INSERT INTO public.payments (booking_id, method, status, amount, paid_at) "
            "VALUES ("
            + std::to_string(booking_id) + ", "
            + txn.quote(method) + ", "
            + "'completed', "
            + std::to_string(amount) + ", "
            + "NOW()) "
            "RETURNING id, booking_id, method, status, amount::float8, to_char(paid_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS paid_at;";

        pqxx::result pay_res = txn.exec(insert_payment);
        txn.commit();

        models::PaymentResult result;
        result.id = pay_res[0]["id"].as<int64_t>();
        result.booking_id = pay_res[0]["booking_id"].as<int64_t>();
        result.method = pay_res[0]["method"].as<std::string>();
        result.status = pay_res[0]["status"].as<std::string>();
        result.amount = pay_res[0]["amount"].as<double>();
        result.paid_at = pay_res[0]["paid_at"].as<std::string>();

        return result;
    }

    // 2. Cancel Booking
    void cancel_booking(int64_t booking_id, const std::string& user_id) {
        if (!db_pool_) throw std::runtime_error("Database pool offline.");

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        // Update status to 'cancelled'. Database trigger trg_booking_status_sync_seats
        // will automatically free all passenger seats!
        std::string q = 
            "UPDATE public.bookings "
            "SET status = 'cancelled', updated_at = NOW() "
            "WHERE id = " + std::to_string(booking_id) + " "
            "  AND user_id = " + txn.quote(user_id) + "::uuid "
            "  AND status IN ('held', 'paid');";

        pqxx::result res = txn.exec(q);
        if (res.affected_rows() == 0) {
            txn.abort();
            throw std::runtime_error("Pesanan tidak dapat dibatalkan atau tidak ditemukan.");
        }

        txn.commit();
    }

    // 3. Get Royal Pass Detail (E-Ticket)
    std::optional<RoyalPassDetail> get_royal_pass(const std::string& sigil_code, const std::string& user_id, bool is_admin = false) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        std::string query = 
            "SELECT "
            "    b.id AS booking_id, "
            "    b.code AS sigil_code, "
            "    b.status, "
            "    b.total_price::float8 AS total_price, "
            "    to_char(b.created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS booked_at, "
            "    to_char(b.expires_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS expires_at, "
            "    u.name AS user_name, "
            "    u.email AS user_email, "
            "    t.name AS train_name, "
            "    t.class AS train_class, "
            "    orig.name AS origin_station, "
            "    orig.code AS origin_code, "
            "    orig.city AS origin_city, "
            "    dest.name AS destination_station, "
            "    dest.code AS destination_code, "
            "    dest.city AS destination_city, "
            "    to_char(sc.departure_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS departure_at, "
            "    to_char(sc.arrival_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS arrival_at, "
            "    COALESCE(p.method, 'unpaid') AS payment_method, "
            "    to_char(p.paid_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS paid_at, "
            "    COALESCE( "
            "        json_agg( "
            "            json_build_object( "
            "                'passenger_name', bp.name, "
            "                'id_number', bp.id_number, "
            "                'carriage_number', c.carriage_number, "
            "                'seat_number', s.seat_number "
            "            ) "
            "        ) FILTER (WHERE bp.id IS NOT NULL), '[]'::json "
            "    ) AS passengers "
            "FROM public.bookings b "
            "JOIN public.users u ON b.user_id = u.id "
            "JOIN public.schedules sc ON b.schedule_id = sc.id "
            "JOIN public.trains t ON sc.train_id = t.id "
            "JOIN public.stations orig ON sc.origin_id = orig.id "
            "JOIN public.stations dest ON sc.destination_id = dest.id "
            "LEFT JOIN public.payments p ON p.booking_id = b.id "
            "LEFT JOIN public.booking_passengers bp ON bp.booking_id = b.id "
            "LEFT JOIN public.seats s ON bp.seat_id = s.id "
            "LEFT JOIN public.carriages c ON s.carriage_id = c.id "
            "WHERE b.code = " + txn.quote(sigil_code) + " "
            + (is_admin ? "" : "  AND b.user_id = " + txn.quote(user_id) + "::uuid ") +
            "GROUP BY "
            "    b.id, b.code, b.status, b.total_price, b.created_at, b.expires_at, "
            "    u.name, u.email, t.name, t.class, "
            "    orig.name, orig.code, orig.city, "
            "    dest.name, dest.code, dest.city, "
            "    sc.departure_at, sc.arrival_at, p.method, p.paid_at;";

        pqxx::result res = txn.exec(query);
        txn.commit();

        if (res.empty()) return std::nullopt;

        const auto& row = res[0];
        RoyalPassDetail detail;
        detail.booking_id = row["booking_id"].as<int64_t>();
        detail.sigil_code = row["sigil_code"].as<std::string>();
        detail.status = row["status"].as<std::string>();
        detail.total_price = row["total_price"].as<double>();
        detail.booked_at = row["booked_at"].as<std::string>();
        detail.expires_at = row["expires_at"].as<std::string>();
        detail.user_name = row["user_name"].as<std::string>();
        detail.user_email = row["user_email"].as<std::string>();
        detail.train_name = row["train_name"].as<std::string>();
        detail.train_class = row["train_class"].as<std::string>();
        detail.origin_station = row["origin_station"].as<std::string>();
        detail.origin_code = row["origin_code"].as<std::string>();
        detail.origin_city = row["origin_city"].as<std::string>();
        detail.destination_station = row["destination_station"].as<std::string>();
        detail.destination_code = row["destination_code"].as<std::string>();
        detail.destination_city = row["destination_city"].as<std::string>();
        detail.departure_at = row["departure_at"].as<std::string>();
        detail.arrival_at = row["arrival_at"].as<std::string>();
        detail.payment_method = row["payment_method"].as<std::string>();
        detail.paid_at = row["paid_at"].is_null() ? "" : row["paid_at"].as<std::string>();
        detail.passengers = nlohmann::json::parse(row["passengers"].as<std::string>());

        return detail;
    }

    // 4. Get User Journey Chronicles
    std::vector<ChronicleItem> get_user_chronicles(const std::string& user_id) {
        std::vector<ChronicleItem> list;
        if (!db_pool_) return list;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        std::string query = 
            "SELECT "
            "    b.id AS booking_id, "
            "    b.code AS sigil_code, "
            "    b.status, "
            "    b.total_price::float8 AS total_price, "
            "    to_char(b.created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS booked_at, "
            "    t.name AS train_name, "
            "    t.class AS train_class, "
            "    orig.city AS origin_city, "
            "    dest.city AS destination_city, "
            "    to_char(sc.departure_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS departure_at, "
            "    to_char(sc.arrival_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS arrival_at, "
            "    COUNT(bp.id)::int AS passenger_count "
            "FROM public.bookings b "
            "JOIN public.schedules sc ON b.schedule_id = sc.id "
            "JOIN public.trains t ON sc.train_id = t.id "
            "JOIN public.stations orig ON sc.origin_id = orig.id "
            "JOIN public.stations dest ON sc.destination_id = dest.id "
            "LEFT JOIN public.booking_passengers bp ON bp.booking_id = b.id "
            "WHERE b.user_id = " + txn.quote(user_id) + "::uuid "
            "GROUP BY b.id, b.code, b.status, b.total_price, b.created_at, "
            "         t.name, t.class, orig.city, dest.city, sc.departure_at, sc.arrival_at "
            "ORDER BY b.created_at DESC;";

        pqxx::result res = txn.exec(query);
        txn.commit();

        for (const auto& row : res) {
            ChronicleItem item;
            item.booking_id = row["booking_id"].as<int64_t>();
            item.sigil_code = row["sigil_code"].as<std::string>();
            item.status = row["status"].as<std::string>();
            item.total_price = row["total_price"].as<double>();
            item.booked_at = row["booked_at"].as<std::string>();
            item.train_name = row["train_name"].as<std::string>();
            item.train_class = row["train_class"].as<std::string>();
            item.origin_city = row["origin_city"].as<std::string>();
            item.destination_city = row["destination_city"].as<std::string>();
            item.departure_at = row["departure_at"].as<std::string>();
            item.arrival_at = row["arrival_at"].as<std::string>();
            item.passenger_count = row["passenger_count"].as<int>();
            list.push_back(item);
        }

        return list;
    }

private:
    std::shared_ptr<config::DatabasePool> db_pool_;
};

} // namespace ambatrain::repositories
