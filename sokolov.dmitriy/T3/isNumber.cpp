#include "isNumber.hpp"
#include <string>

bool isNumber(const std::string &str) {

    if (str.empty()) {
        return false;
    }

    for (const char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }

    return true;
}
