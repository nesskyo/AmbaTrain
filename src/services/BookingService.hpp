#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <stdexcept>
#include "../repositories/BookingRepository.hpp"
#include "../models/Booking.hpp"
#include "../config/Config.hpp"

namespace ambatrain::services {

class BookingCollisionException : public std::runtime_error {
public:
    explicit BookingCollisionException(const std::string& msg) : std::runtime_error(msg) {}
};

class BookingService {
public:
    BookingService(std::shared_ptr<repositories::BookingRepository> booking_repo, config::Config config)
        : booking_repo_(std::move(booking_repo)), config_(std::move(config)) {}

    models::BookingResponse create_booking(
        const std::string& user_id, 
        int64_t schedule_id, 
        const std::vector<models::PassengerInput>& passengers
    ) {
        if (user_id.empty()) {
            throw std::invalid_argument("Identitas pemesan tidak sah.");
        }
        if (schedule_id <= 0) {
            throw std::invalid_argument("ID jadwal tidak sah.");
        }
        if (passengers.empty() || passengers.size() > 4) {
            throw std::invalid_argument("Jumlah penumpang harus antara 1 dan 4 orang.");
        }

        // Check for duplicate seats within the same request
        std::set<int64_t> requested_seats;
        for (const auto& pax : passengers) {
            if (pax.name.empty()) {
                throw std::invalid_argument("Nama penumpang wajib diisi.");
            }
            if (pax.id_number.empty()) {
                throw std::invalid_argument("Nomor identitas (NIK) penumpang wajib diisi.");
            }
            if (pax.seat_id <= 0) {
                throw std::invalid_argument("Kursi yang dipilih tidak sah.");
            }

            if (requested_seats.count(pax.seat_id) > 0) {
                throw std::invalid_argument("Dua penumpang tidak boleh memilih kursi yang sama.");
            }
            requested_seats.insert(pax.seat_id);
        }

        try {
            return booking_repo_->create_booking_hold(
                user_id, 
                schedule_id, 
                passengers, 
                config_.hold_minutes
            );
        } catch (const std::runtime_error& e) {
            std::string msg = e.what();
            if (msg.rfind("SEAT_COLLISION:", 0) == 0) {
                throw BookingCollisionException(msg.substr(16)); // strip prefix
            }
            throw;
        }
    }

private:
    std::shared_ptr<repositories::BookingRepository> booking_repo_;
    config::Config config_;
};

} // namespace ambatrain::services
