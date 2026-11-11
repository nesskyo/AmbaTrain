#pragma once

#include <string>
#include <vector>
#include <optional>
#include <pqxx/pqxx>
#include "../models/Station.hpp"
#include "../config/DatabasePool.hpp"

namespace ambatrain::repositories {

class StationRepository {
public:
    explicit StationRepository(std::shared_ptr<config::DatabasePool> db_pool)
        : db_pool_(std::move(db_pool)) {}

    std::vector<models::Station> get_all() {
        std::vector<models::Station> list;
        if (!db_pool_) return list;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        pqxx::result res = txn.exec(
            "SELECT id, code, name, city FROM public.stations ORDER BY city ASC, name ASC;"
        );
        txn.commit();

        for (const auto& row : res) {
            models::Station st;
            st.id = row["id"].as<int64_t>();
            st.code = row["code"].as<std::string>();
            st.name = row["name"].as<std::string>();
            st.city = row["city"].as<std::string>();
            list.push_back(st);
        }

        return list;
    }

    std::optional<models::Station> get_by_id(int64_t id) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        pqxx::result res = txn.exec(
            "SELECT id, code, name, city FROM public.stations WHERE id = " + std::to_string(id) + " LIMIT 1;"
        );
        txn.commit();

        if (res.empty()) return std::nullopt;

        models::Station st;
        st.id = res[0]["id"].as<int64_t>();
        st.code = res[0]["code"].as<std::string>();
        st.name = res[0]["name"].as<std::string>();
        st.city = res[0]["city"].as<std::string>();

        return st;
    }

    std::optional<models::Station> get_by_code(const std::string& code) {
        if (!db_pool_) return std::nullopt;

        auto conn = db_pool_->acquire();
        pqxx::work txn(conn.get());

        pqxx::result res = txn.exec(
            "SELECT id, code, name, city FROM public.stations WHERE code = " + txn.quote(code) + " LIMIT 1;"
        );
        txn.commit();

        if (res.empty()) return std::nullopt;

        models::Station st;
        st.id = res[0]["id"].as<int64_t>();
        st.code = res[0]["code"].as<std::string>();
        st.name = res[0]["name"].as<std::string>();
        st.city = res[0]["city"].as<std::string>();

        return st;
    }

private:
    std::shared_ptr<config::DatabasePool> db_pool_;
};

} // namespace ambatrain::repositories

