#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "../repositories/PaymentRepository.hpp"
#include "../models/Payment.hpp"

namespace ambatrain::services {

class PaymentService {
public:
    explicit PaymentService(std::shared_ptr<repositories::PaymentRepository> payment_repo)
        : payment_repo_(std::move(payment_repo)) {}

    models::PaymentResult pay(int64_t booking_id, const std::string& user_id, const std::string& method) {
        if (booking_id <= 0) {
            throw std::invalid_argument("ID pesanan tidak sah.");
        }
        if (user_id.empty()) {
            throw std::invalid_argument("Identitas musafir tidak sah.");
        }

        std::string chosen_method = method.empty() ? "tribute_crystal" : method;
        if (chosen_method != "tribute_crystal" && chosen_method != "qris" && 
            chosen_method != "bank_transfer" && chosen_method != "credit_card") {
            throw std::invalid_argument("Metode tribute tiada dikenal di seluruh kerajaan.");
        }

        return payment_repo_->pay_booking(booking_id, user_id, chosen_method);
    }

    void cancel(int64_t booking_id, const std::string& user_id) {
        if (booking_id <= 0) {
            throw std::invalid_argument("ID pesanan tidak sah.");
        }
        payment_repo_->cancel_booking(booking_id, user_id);
    }

    repositories::RoyalPassDetail get_royal_pass(const std::string& code, const std::string& user_id, bool is_admin = false) {
        if (code.empty()) {
            throw std::invalid_argument("Kode Sigil tiket tidak sah.");
        }

        auto pass_opt = payment_repo_->get_royal_pass(code, user_id, is_admin);
        if (!pass_opt) {
            throw std::runtime_error("Royal Pass (e-ticket) tiada ditemukan atau engkau tiada berhak melihatnya.");
        }
        return *pass_opt;
    }

    std::vector<repositories::ChronicleItem> get_chronicles(const std::string& user_id) {
        if (user_id.empty()) {
            throw std::invalid_argument("Identitas musafir tidak sah.");
        }
        return payment_repo_->get_user_chronicles(user_id);
    }

private:
    std::shared_ptr<repositories::PaymentRepository> payment_repo_;
};

} // namespace ambatrain::services
