#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct User {
    std::string id;
    std::string name;
    std::string email;
    std::string password_hash;
    std::string role = "user"; // 'user' or 'admin'
    std::string created_at;
};

inline void to_json(nlohmann::json& j, const User& u) {
    j = nlohmann::json{
        {"id", u.id},
        {"name", u.name},
        {"email", u.email},
        {"role", u.role},
        {"created_at", u.created_at}
        // password_hash intentionally omitted for safety
    };
}

inline void from_json(const nlohmann::json& j, User& u) {
    if (j.contains("id")) u.id = j.at("id").get<std::string>();
    if (j.contains("name")) u.name = j.at("name").get<std::string>();
    if (j.contains("email")) u.email = j.at("email").get<std::string>();
    if (j.contains("password_hash")) u.password_hash = j.at("password_hash").get<std::string>();
    if (j.contains("role")) u.role = j.at("role").get<std::string>();
    if (j.contains("created_at")) u.created_at = j.at("created_at").get<std::string>();
}

} // namespace ambatrain::models

