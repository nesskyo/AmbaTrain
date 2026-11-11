#pragma once

#include <crow.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../repositories/StationRepository.hpp"
#include "../utils/ResponseUtils.hpp"

namespace ambatrain::controllers {

template <typename AppType>
class StationController {
public:
    explicit StationController(std::shared_ptr<repositories::StationRepository> station_repo)
        : station_repo_(std::move(station_repo)) {}

    void register_routes(AppType& app) {
        // --- GET /api/stations ---
        CROW_ROUTE(app, "/api/stations").methods(crow::HTTPMethod::Get)
        ([this]() {
            try {
                auto stations = station_repo_->get_all();
                nlohmann::json list = nlohmann::json::array();
                for (const auto& st : stations) {
                    list.push_back(st);
                }
                return utils::ResponseUtils::success(list);
            } catch (const std::exception& e) {
                return utils::ResponseUtils::internal_error(std::string("Gagal mengambil gerbang: ") + e.what());
            }
        });

        // --- GET /api/stations/:id ---
        CROW_ROUTE(app, "/api/stations/<int>").methods(crow::HTTPMethod::Get)
        ([this](int id) {
            try {
                auto st_opt = station_repo_->get_by_id(id);
                if (!st_opt) {
                    return utils::ResponseUtils::not_found("Gerbang stasiun ini tiada tercatat di peta kerajaan.");
                }
                return utils::ResponseUtils::success(*st_opt);
            } catch (const std::exception& e) {
                return utils::ResponseUtils::internal_error(e.what());
            }
        });
    }

private:
    std::shared_ptr<repositories::StationRepository> station_repo_;
};

} // namespace ambatrain::controllers

