#define _CRT_SECURE_NO_WARNINGS // using std::localtime is insecure
#include "calendar.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

std::string safeTimestamp() {
    time_t time = std::time(nullptr);
    std::tm* ltm = std::localtime(&time);
    std::ostringstream result;
    result << std::put_time(ltm, "%F-%H%M%S");
    return result.str();
}

std::string timestamp() {
    time_t time = std::time(nullptr);
    std::tm* ltm = std::localtime(&time);
    std::ostringstream result;
    result << std::put_time(ltm, "%F %T %z");
    return result.str();
}
