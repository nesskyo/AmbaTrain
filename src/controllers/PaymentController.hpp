#pragma once

#include <crow.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../services/PaymentService.hpp"
#include "../middleware/AuthGuard.hpp"
#include "../utils/ResponseUtils.hpp"

namespace ambatrain::controllers {

template <typename AppType>
class PaymentController {
public:
    PaymentController(std::shared_ptr<services::PaymentService> payment_service, config::Config config)
        : payment_service_(std::move(payment_service)), config_(std::move(config)) {}

    void register_routes(AppType& app) {
        // --- POST /api/bookings/:id/pay ---
        CROW_ROUTE(app, "/api/bookings/<int>/pay").methods(crow::HTTPMethod::Post)
        ([this](const crow::request& req, int booking_id) {
            auto auth = middleware::AuthGuard::authenticate(req, config_.jwt_secret);
            if (!auth.is_valid) {
                return utils::ResponseUtils::unauthorized(auth.error_message);
            }

            try {
                std::string method = "tribute_crystal";
                if (!req.body.empty()) {
                    auto body = nlohmann::json::parse(req.body);
                    method = body.value("method", "tribute_crystal");
                }

                auto payment_res = payment_service_->pay(booking_id, auth.claims.user_id, method);
                nlohmann::json response_body = {
                    {"payment", payment_res},
                    {"message", "Takdirmu telah tersegel sempurna. Royal Pass siap diunduh."}
                };

                return utils::ResponseUtils::success(response_body, 200);
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::bad_request(e.what());
            }
        });

        // --- POST /api/bookings/:id/cancel ---
        CROW_ROUTE(app, "/api/bookings/<int>/cancel").methods(crow::HTTPMethod::Post)
        ([this](const crow::request& req, int booking_id) {
            auto auth = middleware::AuthGuard::authenticate(req, config_.jwt_secret);
            if (!auth.is_valid) {
                return utils::ResponseUtils::unauthorized(auth.error_message);
            }

            try {
                payment_service_->cancel(booking_id, auth.claims.user_id);
                nlohmann::json response_body = {
                    {"message", "Perjalanan telah engkau lepaskan. Kursi kembali ke pangkuan kabut."}
                };
                return utils::ResponseUtils::success(response_body, 200);
            } catch (const std::exception& e) {
                return utils::ResponseUtils::bad_request(e.what());
            }
        });

        // --- GET /api/bookings/:code (Royal Pass / E-Ticket Detail) ---
        CROW_ROUTE(app, "/api/bookings/<string>").methods(crow::HTTPMethod::Get)
        ([this](const crow::request& req, std::string code) {
            auto auth = middleware::AuthGuard::authenticate(req, config_.jwt_secret);
            if (!auth.is_valid) {
                return utils::ResponseUtils::unauthorized(auth.error_message);
            }

            bool is_admin = (auth.claims.role == "admin");

            try {
                auto pass = payment_service_->get_royal_pass(code, auth.claims.user_id, is_admin);

                nlohmann::json res_body = {
                    {"booking_id", pass.booking_id},
                    {"sigil_code", pass.sigil_code},
                    {"status", pass.status},
                    {"total_price", pass.total_price},
                    {"booked_at", pass.booked_at},
                    {"expires_at", pass.expires_at},
                    {"traveler", {
                        {"name", pass.user_name},
                        {"email", pass.user_email}
                    }},
                    {"train", {
                        {"name", pass.train_name},
                        {"class", pass.train_class}
                    }},
                    {"origin", {
                        {"name", pass.origin_station},
                        {"code", pass.origin_code},
                        {"city", pass.origin_city}
                    }},
                    {"destination", {
                        {"name", pass.destination_station},
                        {"code", pass.destination_code},
                        {"city", pass.destination_city}
                    }},
                    {"schedule", {
                        {"departure_at", pass.departure_at},
                        {"arrival_at", pass.arrival_at}
                    }},
                    {"payment", {
                        {"method", pass.payment_method},
                        {"paid_at", pass.paid_at}
                    }},
                    {"passengers", pass.passengers}
                };

                return utils::ResponseUtils::success(res_body, 200);
            } catch (const std::exception& e) {
                return utils::ResponseUtils::not_found(e.what());
            }
        });

        // --- GET /api/bookings (Kronik Riwayat Pesanan Pengguna) ---
        CROW_ROUTE(app, "/api/bookings").methods(crow::HTTPMethod::Get)
        ([this](const crow::request& req) {
            auto auth = middleware::AuthGuard::authenticate(req, config_.jwt_secret);
            if (!auth.is_valid) {
                return utils::ResponseUtils::unauthorized(auth.error_message);
            }

            try {
                auto chronicles = payment_service_->get_chronicles(auth.claims.user_id);
                nlohmann::json list = nlohmann::json::array();

                for (const auto& c : chronicles) {
                    list.push_back({
                        {"booking_id", c.booking_id},
                        {"sigil_code", c.sigil_code},
                        {"status", c.status},
                        {"total_price", c.total_price},
                        {"booked_at", c.booked_at},
                        {"train_name", c.train_name},
                        {"train_class", c.train_class},
                        {"origin_city", c.origin_city},
                        {"destination_city", c.destination_city},
                        {"departure_at", c.departure_at},
                        {"arrival_at", c.arrival_at},
                        {"passenger_count", c.passenger_count}
                    });
                }

                return utils::ResponseUtils::success(list, 200);
            } catch (const std::exception& e) {
                return utils::ResponseUtils::internal_error(e.what());
            }
        });
    }

private:
    std::shared_ptr<services::PaymentService> payment_service_;
    config::Config config_;
};

} // namespace ambatrain::controllers
