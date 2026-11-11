#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct PassengerInput {
    std::string name;
    std::string id_number;
    int64_t seat_id = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PassengerInput, name, id_number, seat_id)
};

struct BookingRequest {
    int64_t schedule_id = 0;
    std::vector<PassengerInput> passengers;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BookingRequest, schedule_id, passengers)
};

struct BookingResponse {
    int64_t id = 0;
    std::string code;
    std::string status;
    double total_price = 0.0;
    std::string expires_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BookingResponse, id, code, status, total_price, expires_at)
};

} // namespace ambatrain::models

