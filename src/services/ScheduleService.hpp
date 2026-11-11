#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <regex>
#include "../repositories/ScheduleRepository.hpp"
#include "../repositories/SeatRepository.hpp"
#include "../models/Seat.hpp"

namespace ambatrain::services {

class ScheduleService {
public:
    ScheduleService(
        std::shared_ptr<repositories::ScheduleRepository> schedule_repo,
        std::shared_ptr<repositories::SeatRepository> seat_repo
    ) : schedule_repo_(std::move(schedule_repo)), seat_repo_(std::move(seat_repo)) {}

    std::vector<models::ScheduleSearchResult> search(
        int64_t from_id, 
        int64_t to_id, 
        const std::string& date_str, 
        int pax = 1
    ) {
        if (from_id <= 0 || to_id <= 0) {
            throw std::invalid_argument("Gerbang asal dan tujuan harus dipilih.");
        }
        if (from_id == to_id) {
            throw std::invalid_argument("Gerbang asal dan tujuan tiada boleh sama; perjalanan membutuhkan jarak.");
        }
        if (pax < 1 || pax > 4) {
            throw std::invalid_argument("Rombongan pengelana dibatasi minimal 1 dan maksimal 4 jiwa per pesanan.");
        }

        // Validate date format (YYYY-MM-DD)
        std::regex date_pattern(R"(^\d{4}-\d{2}-\d{2}$)");
        if (!std::regex_match(date_str, date_pattern)) {
            throw std::invalid_argument("Format tanggal tidak sah. Gunakan format YYYY-MM-DD.");
        }

        return schedule_repo_->search(from_id, to_id, date_str, pax);
    }

    models::ScheduleSearchResult get_schedule(int64_t schedule_id) {
        if (schedule_id <= 0) {
            throw std::invalid_argument("ID jadwal tidak sah.");
        }

        auto res = schedule_repo_->get_by_id(schedule_id);
        if (!res) {
            throw std::runtime_error("Jadwal perjalanan tiada ditemukan pada kronik.");
        }
        return *res;
    }

    models::ScheduleSeatMapResponse get_seat_map(int64_t schedule_id) {
        if (schedule_id <= 0) {
            throw std::invalid_argument("ID jadwal tidak sah.");
        }

        auto res = seat_repo_->get_seat_map(schedule_id);
        if (!res) {
            throw std::runtime_error("Denah gerbong tiada ditemukan untuk jadwal ini.");
        }
        return *res;
    }

private:
    std::shared_ptr<repositories::ScheduleRepository> schedule_repo_;
    std::shared_ptr<repositories::SeatRepository> seat_repo_;
};

} // namespace ambatrain::services
