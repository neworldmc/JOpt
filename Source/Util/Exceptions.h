#pragma once

#include <stdexcept>

namespace Utils {
    struct DownCastFailure final : std::exception {
        
    };

    struct NullPointerException final : std::exception {
        
    };
}