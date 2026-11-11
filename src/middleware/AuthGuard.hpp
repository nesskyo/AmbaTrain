#pragma once

#include <crow.h>
#include <optional>
#include <string>
#include "../utils/JwtUtils.hpp"
#include "../utils/ResponseUtils.hpp"

namespace ambatrain::middleware {

class AuthGuard {
public:
    static std::optional<std::string> extract_bearer_token(const crow::request& req) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) return std::nullopt;

        const std::string prefix = "Bearer ";
        if (auth_header.rfind(prefix, 0) == 0) {
            return auth_header.substr(prefix.length());
        }
        return std::nullopt;
    }

    struct AuthResult {
        bool is_valid = false;
        utils::JwtClaims claims;
        int status_code = 0;
        std::string error_message;
    };

    static AuthResult authenticate(const crow::request& req, const std::string& jwt_secret) {
        auto token_opt = extract_bearer_token(req);
        if (!token_opt) {
            return {false, {}, 401, "Segel masuk (token) tiada ditemukan. Silakan masuk terlebih dahulu."};
        }

        auto claims_opt = utils::JwtUtils::verify_token(*token_opt, jwt_secret);
        if (!claims_opt) {
            return {false, {}, 401, "Segel masuk (token) telah pudar atau tiada sah."};
        }

        return {true, *claims_opt, 0, ""};
    }

    static AuthResult require_admin(const crow::request& req, const std::string& jwt_secret) {
        auto auth = authenticate(req, jwt_secret);
        if (!auth.is_valid) return auth;

        if (auth.claims.role != "admin") {
            return {false, auth.claims, 403, "Hanya sang Conductor (Admin) yang berhak menginjak ruangan ini."};
        }

        return auth;
    }
};

} // namespace ambatrain::middleware
