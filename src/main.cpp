#include <crow.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <memory>

#include "config/Config.hpp"
#include "config/DatabasePool.hpp"
#include "middleware/CorsMiddleware.hpp"
#include "utils/ResponseUtils.hpp"

// Repositories
#include "repositories/UserRepository.hpp"
#include "repositories/StationRepository.hpp"

// Services
#include "services/AuthService.hpp"

// Controllers
#include "controllers/AuthController.hpp"
#include "controllers/StationController.hpp"

using namespace ambatrain;

std::string get_iso_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

int main() {
    std::cout << R"(
========================================================================
   ___        __          _____          _      
  / _ \      / /         /__   \_ __ __ _(_)_ __ 
 / /_\/____ / /   _____    / /\/ '__/ _` | | '_ \
/ /_\\/____/ /___/_____/  / /  | | | (_| | | | | |
\____/     \____/         \/   |_|  \__,_|_|_| |_|

  "All the world's a station, and all the men and women merely passengers."
========================================================================
)" << std::endl;

    // 1. Inisialisasi Kriptografi (libsodium jika aktif)
    utils::CryptoUtils::init();

    // 2. Load Konfigurasi Lingkungan
    auto config = config::Config::from_env();
    std::cout << "[AmbaTrain] Configuration loaded." << std::endl;
    std::cout << "[AmbaTrain] Server target: " << config.server_host << ":" << config.port << std::endl;

    // 3. Inisialisasi Database Connection Pool
    std::shared_ptr<config::DatabasePool> db_pool;
    if (!config.database_url.empty() && config.database_url.find("[YOUR-PASSWORD]") == std::string::npos) {
        try {
            std::cout << "[AmbaTrain] Connecting to Supabase PostgreSQL..." << std::endl;
            db_pool = std::make_shared<config::DatabasePool>(config.database_url, 6);
        } catch (const std::exception& e) {
            std::cerr << "[AmbaTrain] Warning: Database connection initialization error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "[AmbaTrain] Notice: DATABASE_URL contains placeholders in .env." << std::endl;
        std::cout << "[AmbaTrain] Running in standalone development mode." << std::endl;
    }

    // 4. Inisialisasi Layer Repositories & Services
    auto user_repo = std::make_shared<repositories::UserRepository>(db_pool);
    auto station_repo = std::make_shared<repositories::StationRepository>(db_pool);

    auto auth_service = std::make_shared<services::AuthService>(user_repo, config);

    // 5. Inisialisasi Crow Web Application
    using AppType = crow::App<middleware::CorsMiddleware>;
    AppType app;

    // --- Root Welcome Route ---
    CROW_ROUTE(app, "/")
    ([]() {
        nlohmann::json resp = {
            {"realm", "AmbaTrain"},
            {"tagline", "All the world's a station, and all the men and women merely passengers."},
            {"documentation", "/docs"},
            {"endpoints", {
                {"healthcheck", "/api/health"},
                {"register", "POST /api/auth/register"},
                {"login", "POST /api/auth/login"},
                {"profile", "GET /api/auth/me"},
                {"stations", "GET /api/stations"}
            }}
        };
        return utils::ResponseUtils::success(resp);
    });

    // --- Phase 1 Healthcheck Route ---
    CROW_ROUTE(app, "/api/health")
    ([db_pool, &config]() {
        nlohmann::json db_status = {
            {"status", "disconnected"},
            {"message", "Database pool is not configured or offline."}
        };

        bool is_healthy = false;
        double latency_ms = -1.0;

        if (db_pool) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                auto conn = db_pool->acquire(std::chrono::milliseconds(2000));
                pqxx::work txn(conn.get());
                pqxx::result res = txn.exec("SELECT 1;");
                txn.commit();

                auto end = std::chrono::high_resolution_clock::now();
                latency_ms = std::chrono::duration<double, std::milli>(end - start).count();

                if (!res.empty()) {
                    is_healthy = true;
                    db_status = {
                        {"status", "connected"},
                        {"latency_ms", latency_ms},
                        {"provider", "Supabase PostgreSQL"}
                    };
                }
            } catch (const std::exception& e) {
                db_status = {
                    {"status", "error"},
                    {"error", e.what()}
                };
            }
        }

        nlohmann::json response = {
            {"status", is_healthy ? "healthy" : (db_pool ? "degraded" : "standalone")},
            {"service", "AmbaTrain C++ REST Engine"},
            {"version", "1.0.0"},
            {"phase", "Phase 2 Complete (Auth & Master Data)"},
            {"database", db_status},
            {"config", {
                {"port", config.port},
                {"hold_minutes", config.hold_minutes},
                {"cors_origin", config.cors_origin}
            }},
            {"timestamp", get_iso_timestamp()}
        };

        return utils::ResponseUtils::success(response, is_healthy ? 200 : 200);
    });

    // 6. Daftarkan Controller Routes
    controllers::AuthController<AppType> auth_controller(auth_service, config);
    auth_controller.register_routes(app);

    controllers::StationController<AppType> station_controller(station_repo);
    station_controller.register_routes(app);

    // 7. Start Crow Server
    std::cout << "[AmbaTrain] Routes registered successfully." << std::endl;
    std::cout << "[AmbaTrain] Ready to embark on port " << config.port << "..." << std::endl;
    app.port(config.port).multithreaded().run();

    return 0;
}
