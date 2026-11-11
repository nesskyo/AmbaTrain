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
#include "repositories/ScheduleRepository.hpp"
#include "repositories/SeatRepository.hpp"
#include "repositories/BookingRepository.hpp"
#include "repositories/PaymentRepository.hpp"

// Services
#include "services/AuthService.hpp"
#include "services/ScheduleService.hpp"
#include "services/BookingService.hpp"
#include "services/PaymentService.hpp"
#include "services/ExpiryWorker.hpp"

// Controllers
#include "controllers/AuthController.hpp"
#include "controllers/StationController.hpp"
#include "controllers/ScheduleController.hpp"
#include "controllers/BookingController.hpp"
#include "controllers/PaymentController.hpp"

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

    // 1. Inisialisasi Kriptografi
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
            db_pool = std::make_shared<config::DatabasePool>(config.database_url, 10);
        } catch (const std::exception& e) {
            std::cerr << "[AmbaTrain] Warning: Database connection initialization error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "[AmbaTrain] Notice: DATABASE_URL contains placeholders in .env." << std::endl;
        std::cout << "[AmbaTrain] Running in standalone development mode." << std::endl;
    }

    // 4. Inisialisasi Repositories
    auto user_repo = std::make_shared<repositories::UserRepository>(db_pool);
    auto station_repo = std::make_shared<repositories::StationRepository>(db_pool);
    auto schedule_repo = std::make_shared<repositories::ScheduleRepository>(db_pool);
    auto seat_repo = std::make_shared<repositories::SeatRepository>(db_pool);
    auto booking_repo = std::make_shared<repositories::BookingRepository>(db_pool);
    auto payment_repo = std::make_shared<repositories::PaymentRepository>(db_pool);

    // 5. Inisialisasi Services
    auto auth_service = std::make_shared<services::AuthService>(user_repo, config);
    auto schedule_service = std::make_shared<services::ScheduleService>(schedule_repo, seat_repo);
    auto booking_service = std::make_shared<services::BookingService>(booking_repo, config);
    auto payment_service = std::make_shared<services::PaymentService>(payment_repo);

    // 6. Jalankan Background Expiry Worker (Melepas Kursi Kedaluwarsa tiap 60 detik)
    std::unique_ptr<services::ExpiryWorker> expiry_worker;
    if (db_pool) {
        expiry_worker = std::make_unique<services::ExpiryWorker>(booking_repo, 60);
        expiry_worker->start();
    }

    // 7. Inisialisasi Crow Web Application
    using AppType = crow::App<middleware::CorsMiddleware>;
    AppType app;

    // --- Root Welcome Route ---
    CROW_ROUTE(app, "/")
    ([]() {
        nlohmann::json resp = {
            {"realm", "AmbaTrain"},
            {"tagline", "All the world's a station, and all the men and women merely passengers."},
            {"phase", "Phase 5 Complete (Payment Simulation & Royal Pass Active)"},
            {"endpoints", {
                {"healthcheck", "/api/health"},
                {"register", "POST /api/auth/register"},
                {"login", "POST /api/auth/login"},
                {"profile", "GET /api/auth/me"},
                {"stations", "GET /api/stations"},
                {"schedules", "GET /api/schedules?from=1&to=8&date=YYYY-MM-DD&pax=2"},
                {"seat_map", "GET /api/schedules/:id/seats"},
                {"bookings", "POST /api/bookings"},
                {"pay_tribute", "POST /api/bookings/:id/pay"},
                {"cancel_booking", "POST /api/bookings/:id/cancel"},
                {"royal_pass", "GET /api/bookings/:code"},
                {"chronicles", "GET /api/bookings"}
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
            {"anti_double_booking", "Active (Row Locking + Partial Unique Index)"},
            {"payment_engine", "Mock Simulation (Tribute)"},
            {"database", db_status},
            {"config", {
                {"port", config.port},
                {"hold_minutes", config.hold_minutes},
                {"cors_origin", config.cors_origin}
            }},
            {"timestamp", get_iso_timestamp()}
        };

        return utils::ResponseUtils::success(response, 200);
    });

    // 8. Daftarkan Controller Routes
    controllers::AuthController<AppType> auth_controller(auth_service, config);
    auth_controller.register_routes(app);

    controllers::StationController<AppType> station_controller(station_repo);
    station_controller.register_routes(app);

    controllers::ScheduleController<AppType> schedule_controller(schedule_service);
    schedule_controller.register_routes(app);

    controllers::BookingController<AppType> booking_controller(booking_service, config);
    booking_controller.register_routes(app);

    controllers::PaymentController<AppType> payment_controller(payment_service, config);
    payment_controller.register_routes(app);

    // 9. Start Crow Server
    std::cout << "[AmbaTrain] Phase 1 through 5 routes registered successfully." << std::endl;
    std::cout << "[AmbaTrain] Embarking on port " << config.port << "..." << std::endl;
    app.port(config.port).multithreaded().run();

    // Clean stop worker on exit
    if (expiry_worker) {
        expiry_worker->stop();
    }

    return 0;
}
