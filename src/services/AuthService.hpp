#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <stdexcept>
#include "../repositories/UserRepository.hpp"
#include "../utils/CryptoUtils.hpp"
#include "../utils/JwtUtils.hpp"
#include "../config/Config.hpp"

namespace ambatrain::services {

struct AuthResult {
    std::string token;
    models::User user;
};

class AuthService {
public:
    AuthService(std::shared_ptr<repositories::UserRepository> user_repo, config::Config config)
        : user_repo_(std::move(user_repo)), config_(std::move(config)) {}

    AuthResult register_user(const std::string& name, const std::string& email, const std::string& password) {
        if (name.empty()) {
            throw std::invalid_argument("Nama pengelana tidak boleh kosong.");
        }
        if (email.empty() || email.find('@') == std::string::npos || email.find('.') == std::string::npos) {
            throw std::invalid_argument("Surat elang tak dapat terkirim; periksa alamat emailmu.");
        }
        if (password.length() < 6) {
            throw std::invalid_argument("Kata sandi terlalu ringkih; minimal 6 aksara diperlukan.");
        }

        // Check if email already registered
        auto existing = user_repo_->find_by_email(email);
        if (existing) {
            throw std::runtime_error("Email ini telah bersumpah setia sebelumnya.");
        }

        // Hash password
        std::string password_hash = utils::CryptoUtils::hash_password(password);

        // Save user
        auto user = user_repo_->create(name, email, password_hash, "user");

        // Generate JWT
        std::string token = create_token_for_user(user);

        return {token, user};
    }

    AuthResult login_user(const std::string& email, const std::string& password) {
        if (email.empty() || password.empty()) {
            throw std::invalid_argument("Email dan kata sandi wajib diisi.");
        }

        auto user_opt = user_repo_->find_by_email(email);
        if (!user_opt) {
            throw std::runtime_error("Pengelana tak dikenal atau kata sandi keliru.");
        }

        auto user = *user_opt;
        bool valid = utils::CryptoUtils::verify_password(password, user.password_hash);
        if (!valid) {
            throw std::runtime_error("Pengelana tak dikenal atau kata sandi keliru.");
        }

        std::string token = create_token_for_user(user);
        return {token, user};
    }

    models::User get_profile(const std::string& user_id) {
        auto user_opt = user_repo_->find_by_id(user_id);
        if (!user_opt) {
            throw std::runtime_error("Profil pengelana tiada ditemukan.");
        }
        return *user_opt;
    }

private:
    std::string create_token_for_user(const models::User& user) {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        utils::JwtClaims claims;
        claims.user_id = user.id;
        claims.email = user.email;
        claims.role = user.role;
        claims.exp = now + config_.jwt_expires_in;

        return utils::JwtUtils::generate_token(claims, config_.jwt_secret);
    }

    std::shared_ptr<repositories::UserRepository> user_repo_;
    config::Config config_;
};

} // namespace ambatrain::services

