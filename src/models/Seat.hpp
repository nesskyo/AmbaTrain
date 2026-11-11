#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct SeatStatus {
    int64_t seat_id = 0;
    std::string seat_number;
    std::string status = "available"; // 'available', 'held', 'booked'

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SeatStatus, seat_id, seat_number, status)
};

struct CarriageLayout {
    int64_t carriage_id = 0;
    int carriage_number = 1;
    std::vector<SeatStatus> seats;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(CarriageLayout, carriage_id, carriage_number, seats)
};

struct ScheduleSeatMapResponse {
    int64_t schedule_id = 0;
    std::string train_name;
    std::string train_class;
    std::vector<CarriageLayout> carriages;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ScheduleSeatMapResponse, schedule_id, train_name, train_class, carriages)
};

} // namespace ambatrain::models
