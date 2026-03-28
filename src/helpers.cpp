/**
 * @file helpers.cpp
 * @brief Utility function implementations.
 */

#include "helpers.h"
#include "constants.h"
#include "types.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

std::string formatTime(int totalSec) {
    int m = totalSec / 60, s = totalSec % 60;
    std::ostringstream o;
    o << (m < 10 ? "0" : "") << m << ":" << (s < 10 ? "0" : "") << s;
    return o.str();
}

float randf(float lo, float hi) {
    return lo + static_cast<float>(std::rand()) / RAND_MAX * (hi - lo);
}

const char* diffName(int d) {
    return DIFFICULTIES[d].label;
}

bool containsCheatCode(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.find(CHEAT_CODE) != std::string::npos;
}

std::string stripCheatCode(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    std::string result = name;
    size_t pos = lower.find(CHEAT_CODE);
    while (pos != std::string::npos) {
        result.erase(pos, CHEAT_CODE.size());
        lower.erase(pos, CHEAT_CODE.size());
        pos = lower.find(CHEAT_CODE);
    }
    size_t start = result.find_first_not_of(' ');
    size_t end   = result.find_last_not_of(' ');
    if (start == std::string::npos) return result;
    return result.substr(start, end - start + 1);
}
