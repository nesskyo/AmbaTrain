#pragma once

#include <crow.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../services/ScheduleService.hpp"
#include "../utils/ResponseUtils.hpp"

namespace ambatrain::controllers {

template <typename AppType>
class ScheduleController {
public:
    explicit ScheduleController(std::shared_ptr<services::ScheduleService> schedule_service)
        : schedule_service_(std::move(schedule_service)) {}

    void register_routes(AppType& app) {
        // --- GET /api/schedules?from=X&to=Y&date=Z&pax=N ---
        CROW_ROUTE(app, "/api/schedules").methods(crow::HTTPMethod::Get)
        ([this](const crow::request& req) {
            try {
                auto from_param = req.url_params.get("from");
                auto to_param = req.url_params.get("to");
                auto date_param = req.url_params.get("date");
                auto pax_param = req.url_params.get("pax");

                if (!from_param || !to_param || !date_param) {
                    return utils::ResponseUtils::bad_request("Parameter 'from', 'to', dan 'date' wajib disertakan.");
                }

                int64_t from_id = std::stoll(from_param);
                int64_t to_id = std::stoll(to_param);
                std::string date_str = date_param;
                int pax = pax_param ? std::stoi(pax_param) : 1;

                auto schedules = schedule_service_->search(from_id, to_id, date_str, pax);

                nlohmann::json list = nlohmann::json::array();
                for (const auto& s : schedules) {
                    list.push_back(s);
                }

                return utils::ResponseUtils::success(list);
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::internal_error(e.what());
            }
        });

        // --- GET /api/schedules/:id ---
        CROW_ROUTE(app, "/api/schedules/<int>").methods(crow::HTTPMethod::Get)
        ([this](int id) {
            try {
                auto schedule = schedule_service_->get_schedule(id);
                return utils::ResponseUtils::success(schedule);
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::not_found(e.what());
            }
        });

        // --- GET /api/schedules/:id/seats ---
        CROW_ROUTE(app, "/api/schedules/<int>/seats").methods(crow::HTTPMethod::Get)
        ([this](int id) {
            try {
                auto seat_map = schedule_service_->get_seat_map(id);
                return utils::ResponseUtils::success(seat_map);
            } catch (const std::invalid_argument& e) {
                return utils::ResponseUtils::bad_request(e.what());
            } catch (const std::exception& e) {
                return utils::ResponseUtils::not_found(e.what());
            }
        });
    }

private:
    std::shared_ptr<services::ScheduleService> schedule_service_;
};

} // namespace ambatrain::controllers
