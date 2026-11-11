#pragma once

#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

namespace ambatrain::config {

struct Config {
    std::string database_url;
    std::string server_host = "0.0.0.0";
    uint16_t port = 8080;
    std::string cors_origin = "*";
    std::string jwt_secret = "ambatrain-default-gothic-secret-key-123456";
    int jwt_expires_in = 86400; // 24 hours in seconds
    int hold_minutes = 15;

    static void load_dotenv(const std::string& path = ".env") {
        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            // Trim whitespace
            auto start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) continue;
            if (line[start] == '#') continue; // Skip comments

            auto eq_pos = line.find('=', start);
            if (eq_pos == std::string::npos) continue;

            std::string key = line.substr(start, eq_pos - start);
            std::string val = line.substr(eq_pos + 1);

            // Trim key
            auto key_end = key.find_last_not_of(" \t\r\n");
            if (key_end != std::string::npos) key = key.substr(0, key_end + 1);

            // Trim val
            auto val_start = val.find_first_not_of(" \t\r\n");
            if (val_start != std::string::npos) {
                auto val_end = val.find_last_not_of(" \t\r\n");
                val = val.substr(val_start, val_end - val_start + 1);
            } else {
                val.clear();
            }

            // Remove surrounding quotes if any
            if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') || 
                                    (val.front() == '\'' && val.back() == '\''))) {
                val = val.substr(1, val.size() - 2);
            }

#ifdef _WIN32
            _putenv_s(key.c_str(), val.c_str());
#else
            setenv(key.c_str(), val.c_str(), 1);
#endif
        }
    }

    static Config from_env() {
        load_dotenv();

        Config cfg;
        const char* db_env = std::getenv("DATABASE_URL");
        if (db_env) cfg.database_url = db_env;

        const char* host_env = std::getenv("SERVER_HOST");
        if (host_env) cfg.server_host = host_env;

        const char* port_env = std::getenv("PORT");
        if (port_env) cfg.port = static_cast<uint16_t>(std::stoi(port_env));

        const char* cors_env = std::getenv("CORS_ORIGIN");
        if (cors_env) cfg.cors_origin = cors_env;

        const char* jwt_secret_env = std::getenv("JWT_SECRET");
        if (jwt_secret_env) cfg.jwt_secret = jwt_secret_env;

        const char* jwt_exp_env = std::getenv("JWT_EXPIRES_IN");
        if (jwt_exp_env) cfg.jwt_expires_in = std::stoi(jwt_exp_env);

        const char* hold_env = std::getenv("HOLD_MINUTES");
        if (hold_env) cfg.hold_minutes = std::stoi(hold_env);

        return cfg;
    }
};

} // namespace ambatrain::config

