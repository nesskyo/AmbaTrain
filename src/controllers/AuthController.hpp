#pragma once

#include <crow.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../services/AuthService.hpp"
#include "../middleware/AuthGuard.hpp"
#include "../utils/ResponseUtils.hpp"

namespace ambatrain::controllers {

template <typename AppType>
class AuthController {
public:
    AuthController(std::shared_ptr<services::AuthService> auth_service, config::Config config)
        : auth_service_(std::move(auth_service)), config_(std::move(config)) {}

    void register_routes(AppType& app) {
        // --- POST /api/auth/register ---
        CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::Post)
        ([this](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string name = body.value("name", "");
                std::string email = body.value("email", "");
                std::string password = body.value("password", "");

                auto result = auth_service_->register_user(name, email, password);

                nlohmann::json res_body = {
                    {"token", result.token},
                    {"user", result.user}
                };

                return utils::ResponseUtils::success(res_body, 201);
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::conflict(e.what());
            }
        });

        // --- POST /api/auth/login ---
        CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::Post)
        ([this](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string email = body.value("email", "");
                std::string password = body.value("password", "");

                auto result = auth_service_->login_user(email, password);

                nlohmann::json res_body = {
                    {"token", result.token},
                    {"user", result.user}
                };

                return utils::ResponseUtils::success(res_body, 200);
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::unauthorized(e.what());
            }
        });

        // --- GET /api/auth/me (Protected Route) ---
        CROW_ROUTE(app, "/api/auth/me").methods(crow::HTTPMethod::Get)
        ([this](const crow::request& req) {
            auto auth = middleware::AuthGuard::authenticate(req, config_.jwt_secret);
            if (!auth.is_valid) {
                return utils::ResponseUtils::unauthorized(auth.error_message);
            }

            try {
                auto user = auth_service_->get_profile(auth.claims.user_id);
                return utils::ResponseUtils::success(user);
            } catch (const std::exception& e) {
                return utils::ResponseUtils::not_found(e.what());
            }
        });
    }

private:
    std::shared_ptr<services::AuthService> auth_service_;
    config::Config config_;
};

} // namespace ambatrain::controllers
