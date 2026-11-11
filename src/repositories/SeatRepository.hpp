#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <pqxx/pqxx>
#include "../models/Seat.hpp"
#include "../config/DatabasePool.hpp"

namespace ambatrain::repositories {

class SeatRepository {
public:
    explicit SeatRepository(std::shared_ptr<config::DatabasePool> db_pool)
        : db_pool_(std::move(db_pool)) {}

    std::optional<models::ScheduleSeatMapResponse> get_seat_map(int64_t schedule_id) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        std::string query =
            "SELECT "
            "    c.id AS carriage_id, "
            "    c.carriage_number, "
            "    s.id AS seat_id, "
            "    s.seat_number, "
            "    t.name AS train_name, "
            "    t.class AS train_class, "
            "    CASE "
            "        WHEN bp.id IS NULL THEN 'available' "
            "        WHEN b.status = 'paid' THEN 'booked' "
            "        WHEN b.status = 'held' AND b.expires_at > NOW() THEN 'held' "
            "        ELSE 'available' "
            "    END AS seat_status "
            "FROM public.schedules sc "
            "JOIN public.trains t ON sc.train_id = t.id "
            "JOIN public.carriages c ON c.train_id = sc.train_id "
            "JOIN public.seats s ON s.carriage_id = c.id "
            "LEFT JOIN public.booking_passengers bp "
            "    ON bp.seat_id = s.id "
            "    AND bp.schedule_id = sc.id "
            "    AND bp.seat_active = TRUE "
            "LEFT JOIN public.bookings b "
            "    ON bp.booking_id = b.id "
            "    AND b.status IN ('held', 'paid') "
            "WHERE sc.id = " + std::to_string(schedule_id) + " "
            "ORDER BY c.carriage_number ASC, s.seat_number ASC;";

        pqxx::result res = txn.exec(query);
        txn.commit();

        if (res.empty()) return std::nullopt;

        models::ScheduleSeatMapResponse response;
        response.schedule_id = schedule_id;
        response.train_name = res[0]["train_name"].as<std::string>();
        response.train_class = res[0]["train_class"].as<std::string>();

        // Group seats by carriage
        std::map<int, models::CarriageLayout> carriage_map;
        for (const auto& row : res) {
            int c_num = row["carriage_number"].as<int>();
            int64_t c_id = row["carriage_id"].as<int64_t>();

            if (carriage_map.find(c_num) == carriage_map.end()) {
                models::CarriageLayout layout;
                layout.carriage_id = c_id;
                layout.carriage_number = c_num;
                carriage_map[c_num] = layout;
            }

            models::SeatStatus seat;
            seat.seat_id = row["seat_id"].as<int64_t>();
            seat.seat_number = row["seat_number"].as<std::string>();
            seat.status = row["seat_status"].as<std::string>();

            carriage_map[c_num].seats.push_back(seat);
        }

        for (auto& pair : carriage_map) {
            response.carriages.push_back(std::move(pair.second));
        }

        return response;
    }

private:
    std::shared_ptr<config::DatabasePool> db_pool_;
};

} // namespace ambatrain::repositories
