#pragma once

#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <string>
#include <iostream>
#include <chrono>

namespace ambatrain::config {

class DatabasePool {
public:
    using ConnectionPtr = std::unique_ptr<pqxx::connection>;

    class ScopedConnection {
    public:
        ScopedConnection(DatabasePool& pool, ConnectionPtr conn)
            : pool_(pool), conn_(std::move(conn)) {}

        ~ScopedConnection() {
            if (conn_) {
                pool_.release_connection(std::move(conn_));
            }
        }

        ScopedConnection(const ScopedConnection&) = delete;
        ScopedConnection& operator=(const ScopedConnection&) = delete;

        ScopedConnection(ScopedConnection&& other) noexcept
            : pool_(other.pool_), conn_(std::move(other.conn_)) {}

        ScopedConnection& operator=(ScopedConnection&& other) noexcept {
            if (this != &other) {
                if (conn_) {
                    pool_.release_connection(std::move(conn_));
                }
                conn_ = std::move(other.conn_);
            }
            return *this;
        }

        pqxx::connection& get() { return *conn_; }
        pqxx::connection* operator->() { return conn_.get(); }

    private:
        DatabasePool& pool_;
        ConnectionPtr conn_;
    };

    explicit DatabasePool(std::string connection_string, size_t pool_size = 8)
        : conn_str_(std::move(connection_string)), pool_size_(pool_size), stop_(false) {
        for (size_t i = 0; i < pool_size_; ++i) {
            try {
                auto conn = std::make_unique<pqxx::connection>(conn_str_);
                if (conn->is_open()) {
                    pool_.push(std::move(conn));
                }
            } catch (const std::exception& e) {
                std::cerr << "[DatabasePool] Error creating initial connection: " << e.what() << std::endl;
            }
        }
        std::cout << "[DatabasePool] Initialized with " << pool_.size() << " / " << pool_size_ << " connections." << std::endl;
    }

    ~DatabasePool() {
        stop_ = true;
        cv_.notify_all();
        std::lock_guard<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            pool_.pop();
        }
    }

    ScopedConnection acquire(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, timeout, [this] { return !pool_.empty() || stop_; })) {
            throw std::runtime_error("Database connection pool timeout: no connection available.");
        }

        if (stop_) {
            throw std::runtime_error("Database pool is shutting down.");
        }

        auto conn = std::move(pool_.front());
        pool_.pop();

        // Verify connection is healthy
        if (!conn || !conn->is_open()) {
            try {
                conn = std::make_unique<pqxx::connection>(conn_str_);
            } catch (const std::exception& e) {
                std::cerr << "[DatabasePool] Reconnection failed: " << e.what() << std::endl;
                throw;
            }
        }

        return ScopedConnection(*this, std::move(conn));
    }

    bool check_health() {
        try {
            auto conn = acquire(std::chrono::milliseconds(2000));
            pqxx::work txn(conn.get());
            pqxx::result res = txn.exec("SELECT 1;");
            txn.commit();
            return !res.empty();
        } catch (const std::exception& e) {
            std::cerr << "[DatabasePool] Health check failed: " << e.what() << std::endl;
            return false;
        }
    }

private:
    void release_connection(ConnectionPtr conn) {
        if (!conn) return;
        std::lock_guard<std::mutex> lock(mutex_);
        if (!stop_) {
            pool_.push(std::move(conn));
            cv_.notify_one();
        }
    }

    std::string conn_str_;
    size_t pool_size_;
    std::queue<ConnectionPtr> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

} // namespace ambatrain::config

