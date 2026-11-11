#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct PaymentRequest {
    std::string method = "tribute_crystal"; // qris, bank_transfer, credit_card, tribute_crystal

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PaymentRequest, method)
};

struct PaymentResult {
    int64_t id = 0;
    int64_t booking_id = 0;
    std::string method;
    std::string status;
    double amount = 0.0;
    std::string paid_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PaymentResult, id, booking_id, method, status, amount, paid_at)
};

} // namespace ambatrain::models

