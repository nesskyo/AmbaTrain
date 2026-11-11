#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace ambatrain::models {

struct Train {
    int64_t id = 0;
    std::string name;
    std::string train_class; // 'sovereign', 'noble', 'yeoman'

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Train, id, name, train_class)
};

} // namespace ambatrain::models

