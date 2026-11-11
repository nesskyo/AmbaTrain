#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct ScheduleSearchResult {
    int64_t schedule_id = 0;
    std::string train_name;
    std::string train_class;
    std::string origin_name;
    std::string origin_code;
    std::string origin_city;
    std::string destination_name;
    std::string destination_code;
    std::string destination_city;
    std::string departure_at;
    std::string arrival_at;
    std::string duration;
    double price = 0.0;
    int available_seats = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        ScheduleSearchResult,
        schedule_id, train_name, train_class,
        origin_name, origin_code, origin_city,
        destination_name, destination_code, destination_city,
        departure_at, arrival_at, duration,
        price, available_seats
    )
};

} // namespace ambatrain::models

