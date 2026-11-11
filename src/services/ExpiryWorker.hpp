#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include "../repositories/BookingRepository.hpp"

namespace ambatrain::services {

class ExpiryWorker {
public:
    explicit ExpiryWorker(std::shared_ptr<repositories::BookingRepository> booking_repo, int interval_seconds = 60)
        : booking_repo_(std::move(booking_repo)), interval_seconds_(interval_seconds), running_(false) {}

    ~ExpiryWorker() {
        stop();
    }

    void start() {
        if (running_) return;
        running_ = true;
        worker_thread_ = std::thread(&ExpiryWorker::run_loop, this);
        std::cout << "[ExpiryWorker] Background hold expiry worker started (Interval: " 
                  << interval_seconds_ << "s)." << std::endl;
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        cv_.notify_all();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        std::cout << "[ExpiryWorker] Background hold expiry worker stopped." << std::endl;
    }

private:
    void run_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (cv_.wait_for(lock, std::chrono::seconds(interval_seconds_), [this] { return !running_; })) {
                break; // Stop requested
            }

            try {
                int expired_count = booking_repo_->expire_held_bookings();
                if (expired_count > 0) {
                    std::cout << "[ExpiryWorker] Expired " << expired_count 
                              << " held bookings. Kursi otomatis dilepas ke kabut." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "[ExpiryWorker] Error during hold cleanup: " << e.what() << std::endl;
            }
        }
    }

    std::shared_ptr<repositories::BookingRepository> booking_repo_;
    int interval_seconds_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace ambatrain::services
