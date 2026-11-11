#pragma once

#include <crow.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../services/BookingService.hpp"
#include "../middleware/AuthGuard.hpp"
#include "../utils/ResponseUtils.hpp"

namespace ambatrain::controllers {

template <typename AppType>
class BookingController {
public:
    BookingController(std::shared_ptr<services::BookingService> booking_service, config::Config config)
        : booking_service_(std::move(booking_service)), config_(std::move(config)) {}

    void register_routes(AppType& app) {
        // --- POST /api/bookings (Protected by AuthGuard) ---
        CROW_ROUTE(app, "/api/bookings").methods(crow::HTTPMethod::Post)
        ([this](const crow::request& req) {
            auto auth = middleware::AuthGuard::authenticate(req, config_.jwt_secret);
            if (!auth.is_valid) {
                return *auth.error_response;
            }

            try {
                auto body = nlohmann::json::parse(req.body);
                int64_t schedule_id = body.value("schedule_id", int64_t(0));
                
                std::vector<models::PassengerInput> passengers;
                if (body.contains("passengers") && body["passengers"].is_array()) {
                    for (const auto& p_json : body["passengers"]) {
                        models::PassengerInput p;
                        p.name = p_json.value("name", "");
                        p.id_number = p_json.value("id_number", "");
                        p.seat_id = p_json.value("seat_id", int64_t(0));
                        passengers.push_back(p);
                    }
                }

                auto booking_res = booking_service_->create_booking(
                    auth.claims.user_id,
                    schedule_id,
                    passengers
                );

                nlohmann::json response_body = {
                    {"booking", booking_res}
                };

                return utils::ResponseUtils::success(response_body, 201);
            } catch (const services::BookingCollisionException& e) {
                // Return HTTP 409 Conflict strictly per PRD Section 11.3 & 12
                return utils::ResponseUtils::conflict(e.what());
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::internal_error(e.what());
            }
        });
    }

private:
    std::shared_ptr<services::BookingService> booking_service_;
    config::Config config_;
};

} // namespace ambatrain::controllers
