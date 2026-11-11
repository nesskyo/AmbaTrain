#pragma once

#include <string>
#include <optional>
#include <pqxx/pqxx>
#include "../models/User.hpp"
#include "../config/DatabasePool.hpp"

namespace ambatrain::repositories {

class UserRepository {
public:
    explicit UserRepository(std::shared_ptr<config::DatabasePool> db_pool)
        : db_pool_(std::move(db_pool)) {}

    std::optional<models::User> find_by_email(const std::string& email) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        // Parameterized query via pqxx::params
        pqxx::result res = txn.exec(
            "SELECT id::text, name, email, password_hash, role, created_at::text "
            "FROM public.users WHERE email = " + txn.quote(email) + " LIMIT 1;"
        );
        txn.commit();

        if (res.empty()) return std::nullopt;

        models::User user;
        user.id = res[0]["id"].as<std::string>();
        user.name = res[0]["name"].as<std::string>();
        user.email = res[0]["email"].as<std::string>();
        user.password_hash = res[0]["password_hash"].as<std::string>();
        user.role = res[0]["role"].as<std::string>();
        user.created_at = res[0]["created_at"].as<std::string>();

        return user;
    }

    std::optional<models::User> find_by_id(const std::string& user_id) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        pqxx::result res = txn.exec(
            "SELECT id::text, name, email, password_hash, role, created_at::text "
            "FROM public.users WHERE id = " + txn.quote(user_id) + "::uuid LIMIT 1;"
        );
        txn.commit();

        if (res.empty()) return std::nullopt;

        models::User user;
        user.id = res[0]["id"].as<std::string>();
        user.name = res[0]["name"].as<std::string>();
        user.email = res[0]["email"].as<std::string>();
        user.password_hash = res[0]["password_hash"].as<std::string>();
        user.role = res[0]["role"].as<std::string>();
        user.created_at = res[0]["created_at"].as<std::string>();

        return user;
    }

    models::User create(const std::string& name, const std::string& email, const std::string& password_hash, const std::string& role = "user") {
        if (!db_pool_) throw std::runtime_error("Database pool is offline.");

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        pqxx::result res = txn.exec(
            "INSERT INTO public.users (name, email, password_hash, role) VALUES ("
            + txn.quote(name) + ", "
            + txn.quote(email) + ", "
            + txn.quote(password_hash) + ", "
            + txn.quote(role) + ") "
            "RETURNING id::text, name, email, role, created_at::text;"
        );
        txn.commit();

        if (res.empty()) throw std::runtime_error("Failed to insert user.");

        models::User user;
        user.id = res[0]["id"].as<std::string>();
        user.name = res[0]["name"].as<std::string>();
        user.email = res[0]["email"].as<std::string>();
        user.role = res[0]["role"].as<std::string>();
        user.created_at = res[0]["created_at"].as<std::string>();

        return user;
    }

private:
    std::shared_ptr<config::DatabasePool> db_pool_;
};

} // namespace ambatrain::repositories

