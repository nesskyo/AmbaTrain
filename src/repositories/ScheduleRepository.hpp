#pragma once

#include <string>
#include <vector>
#include <optional>
#include <pqxx/pqxx>
#include "../models/Schedule.hpp"
#include "../config/DatabasePool.hpp"

namespace ambatrain::repositories {

class ScheduleRepository {
public:
    explicit ScheduleRepository(std::shared_ptr<config::DatabasePool> db_pool)
        : db_pool_(std::move(db_pool)) {}

    std::vector<models::ScheduleSearchResult> search(
        int64_t origin_id, 
        int64_t destination_id, 
        const std::string& date_str,
        int pax = 1
    ) {
        std::vector<models::ScheduleSearchResult> results;
        if (!db_pool_) return results;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        std::string query = 
            "SELECT "
            "    sc.id AS schedule_id, "
            "    t.name AS train_name, "
            "    t.class AS train_class, "
            "    st_orig.name AS origin_name, "
            "    st_orig.code AS origin_code, "
            "    st_orig.city AS origin_city, "
            "    st_dest.name AS destination_name, "
            "    st_dest.code AS destination_code, "
            "    st_dest.city AS destination_city, "
            "    to_char(sc.departure_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS departure_at, "
            "    to_char(sc.arrival_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS arrival_at, "
            "    to_char(sc.arrival_at - sc.departure_at, 'HH24\"j \"MI\"m\"') AS duration, "
            "    sc.price::float8 AS price, "
            "    ( "
            "        SELECT COUNT(s.id) "
            "        FROM public.seats s "
            "        JOIN public.carriages c ON s.carriage_id = c.id "
            "        WHERE c.train_id = t.id "
            "    ) - ( "
            "        SELECT COUNT(bp.id) "
            "        FROM public.booking_passengers bp "
            "        JOIN public.bookings b ON bp.booking_id = b.id "
            "        WHERE bp.schedule_id = sc.id "
            "          AND bp.seat_active = TRUE "
            "          AND (b.status = 'paid' OR (b.status = 'held' AND b.expires_at > NOW())) "
            "    ) AS available_seats "
            "FROM public.schedules sc "
            "JOIN public.trains t ON sc.train_id = t.id "
            "JOIN public.stations st_orig ON sc.origin_id = st_orig.id "
            "JOIN public.stations st_dest ON sc.destination_id = st_dest.id "
            "WHERE sc.origin_id = " + std::to_string(origin_id) + " "
            "  AND sc.destination_id = " + std::to_string(destination_id) + " "
            "  AND sc.departure_at::DATE = " + txn.quote(date_str) + "::DATE "
            "ORDER BY sc.departure_at ASC;";

        pqxx::result res = txn.exec(query);
        txn.commit();

        for (const auto& row : res) {
            int seats_left = row["available_seats"].as<int>();
            if (seats_left >= pax) {
                models::ScheduleSearchResult item;
                item.schedule_id = row["schedule_id"].as<int64_t>();
                item.train_name = row["train_name"].as<std::string>();
                item.train_class = row["train_class"].as<std::string>();
                item.origin_name = row["origin_name"].as<std::string>();
                item.origin_code = row["origin_code"].as<std::string>();
                item.origin_city = row["origin_city"].as<std::string>();
                item.destination_name = row["destination_name"].as<std::string>();
                item.destination_code = row["destination_code"].as<std::string>();
                item.destination_city = row["destination_city"].as<std::string>();
                item.departure_at = row["departure_at"].as<std::string>();
                item.arrival_at = row["arrival_at"].as<std::string>();
                item.duration = row["duration"].as<std::string>();
                item.price = row["price"].as<double>();
                item.available_seats = seats_left;

                results.push_back(item);
            }
        }

        return results;
    }

    std::optional<models::ScheduleSearchResult> get_by_id(int64_t schedule_id) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        std::string query = 
            "SELECT "
            "    sc.id AS schedule_id, "
            "    t.name AS train_name, "
            "    t.class AS train_class, "
            "    st_orig.name AS origin_name, "
            "    st_orig.code AS origin_code, "
            "    st_orig.city AS origin_city, "
            "    st_dest.name AS destination_name, "
            "    st_dest.code AS destination_code, "
            "    st_dest.city AS destination_city, "
            "    to_char(sc.departure_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS departure_at, "
            "    to_char(sc.arrival_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS arrival_at, "
            "    to_char(sc.arrival_at - sc.departure_at, 'HH24\"j \"MI\"m\"') AS duration, "
            "    sc.price::float8 AS price, "
            "    ( "
            "        SELECT COUNT(s.id) "
            "        FROM public.seats s "
            "        JOIN public.carriages c ON s.carriage_id = c.id "
            "        WHERE c.train_id = t.id "
            "    ) - ( "
            "        SELECT COUNT(bp.id) "
            "        FROM public.booking_passengers bp "
            "        JOIN public.bookings b ON bp.booking_id = b.id "
            "        WHERE bp.schedule_id = sc.id "
            "          AND bp.seat_active = TRUE "
            "          AND (b.status = 'paid' OR (b.status = 'held' AND b.expires_at > NOW())) "
            "    ) AS available_seats "
            "FROM public.schedules sc "
            "JOIN public.trains t ON sc.train_id = t.id "
            "JOIN public.stations st_orig ON sc.origin_id = st_orig.id "
            "JOIN public.stations st_dest ON sc.destination_id = st_dest.id "
            "WHERE sc.id = " + std::to_string(schedule_id) + " LIMIT 1;";

        pqxx::result res = txn.exec(query);
        txn.commit();

        if (res.empty()) return std::nullopt;

        const auto& row = res[0];
        models::ScheduleSearchResult item;
        item.schedule_id = row["schedule_id"].as<int64_t>();
        item.train_name = row["train_name"].as<std::string>();
        item.train_class = row["train_class"].as<std::string>();
        item.origin_name = row["origin_name"].as<std::string>();
        item.origin_code = row["origin_code"].as<std::string>();
        item.origin_city = row["origin_city"].as<std::string>();
        item.destination_name = row["destination_name"].as<std::string>();
        item.destination_code = row["destination_code"].as<std::string>();
        item.destination_city = row["destination_city"].as<std::string>();
        item.departure_at = row["departure_at"].as<std::string>();
        item.arrival_at = row["arrival_at"].as<std::string>();
        item.duration = row["duration"].as<std::string>();
        item.price = row["price"].as<double>();
        item.available_seats = row["available_seats"].as<int>();

        return item;
    }

private:
    std::shared_ptr<config::DatabasePool> db_pool_;
};

} // namespace ambatrain::repositories
