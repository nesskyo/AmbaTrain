#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct Station {
    int64_t id = 0;
    std::string code;
    std::string name;
    std::string city;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Station, id, code, name, city)
};

} // namespace ambatrain::models

