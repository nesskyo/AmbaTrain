#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>
#include "CryptoUtils.hpp"

namespace ambatrain::utils {

struct JwtClaims {
    std::string user_id;
    std::string email;
    std::string role = "user";
    int64_t exp = 0;
};

class JwtUtils {
public:
    // Base64Url encoder
    static std::string base64url_encode(const std::string& input) {
        static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string out;
        int val = 0, valb = -6;
        for (uint8_t c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(b64_table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
        return out;
    }

    // Base64Url decoder
    static std::string base64url_decode(const std::string& input) {
        std::string in = input;
        while (in.size() % 4 != 0) in.push_back('=');

        std::string out;
        std::vector<int> T(256, -1);
        const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        for (int i = 0; i < 64; i++) T[static_cast<uint8_t>(b64_table[i])] = i;

        int val = 0, valb = -8;
        for (uint8_t c : in) {
            if (c == '=') break;
            if (T[c] == -1) return "";
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }

    // Generate JWT Token
    static std::string generate_token(const JwtClaims& claims, const std::string& secret) {
        nlohmann::json header = {
            {"alg", "HS256"},
            {"typ", "JWT"}
        };

        nlohmann::json payload = {
            {"sub", claims.user_id},
            {"email", claims.email},
            {"role", claims.role},
            {"exp", claims.exp}
        };

        std::string header_b64 = base64url_encode(header.dump());
        std::string payload_b64 = base64url_encode(payload.dump());
        std::string to_sign = header_b64 + "." + payload_b64;

        std::string signature = CryptoUtils::sha256(to_sign + ":" + secret);
        std::string sig_b64 = base64url_encode(signature);

        return to_sign + "." + sig_b64;
    }

    // Verify and decode token
    static std::optional<JwtClaims> verify_token(const std::string& token, const std::string& secret) {
        auto first_dot = token.find('.');
        if (first_dot == std::string::npos) return std::nullopt;

        auto second_dot = token.find('.', first_dot + 1);
        if (second_dot == std::string::npos) return std::nullopt;

        std::string header_b64 = token.substr(0, first_dot);
        std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
        std::string sig_b64 = token.substr(second_dot + 1);

        std::string to_sign = header_b64 + "." + payload_b64;
        std::string expected_sig = base64url_encode(CryptoUtils::sha256(to_sign + ":" + secret));

        if (sig_b64 != expected_sig) {
            return std::nullopt; // Invalid signature
        }

        try {
            std::string payload_json_str = base64url_decode(payload_b64);
            auto payload = nlohmann::json::parse(payload_json_str);

            int64_t exp = payload.value("exp", int64_t(0));
            auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            if (exp > 0 && exp < now) {
                return std::nullopt; // Expired token
            }

            JwtClaims claims;
            claims.user_id = payload.value("sub", "");
            claims.email = payload.value("email", "");
            claims.role = payload.value("role", "user");
            claims.exp = exp;

            return claims;
        } catch (...) {
            return std::nullopt;
        }
    }
};

} // namespace ambatrain::utils

