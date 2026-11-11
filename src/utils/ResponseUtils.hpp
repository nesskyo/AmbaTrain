#pragma once

#include <crow.h>
#include <nlohmann/json.hpp>
#include <string>

namespace ambatrain::utils {

class ResponseUtils {
public:
    static crow::response json(int status_code, const nlohmann::json& body) {
        crow::response res(status_code, body.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    }

    static crow::response success(const nlohmann::json& data, int status_code = 200) {
        return json(status_code, data);
    }

    static crow::response error(
        int status_code, 
        const std::string& code, 
        const std::string& message, 
        const nlohmann::json& details = nlohmann::json::object()
    ) {
        nlohmann::json body = {
            {"error", {
                {"code", code},
                {"message", message},
                {"details", details}
            }}
        };
        return json(status_code, body);
    }

    static crow::response bad_request(const std::string& message, const std::string& code = "BAD_REQUEST") {
        return error(400, code, message);
    }

    static crow::response unauthorized(const std::string& message = "Sumpah janji (token) tidak valid atau kadaluwarsa.", const std::string& code = "UNAUTHORIZED") {
        return error(401, code, message);
    }

    static crow::response forbidden(const std::string& message = "Engkau tidak memiliki izin memasuki kamar ini.", const std::string& code = "FORBIDDEN") {
        return error(403, code, message);
    }

    static crow::response not_found(const std::string& message = "Jalan setapak ini tiada berujung.", const std::string& code = "NOT_FOUND") {
        return error(404, code, message);
    }

    static crow::response conflict(const std::string& message, const nlohmann::json& details = nlohmann::json::object(), const std::string& code = "SEAT_UNAVAILABLE") {
        return error(409, code, message, details);
    }

    static crow::response gone(const std::string& message = "Waktu telah habis. Kursimu kembali ke kabut.", const std::string& code = "BOOKING_EXPIRED") {
        return error(410, code, message);
    }

    static crow::response internal_error(const std::string& message = "Lokomotif mengalami kegagalan tungku. Pandai besi sedang bekerja.", const std::string& code = "INTERNAL_SERVER_ERROR") {
        return error(500, code, message);
    }
};

} // namespace ambatrain::utils

