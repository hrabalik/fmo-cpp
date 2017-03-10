#define _CRT_SECURE_NO_WARNINGS // using std::localtime is insecure
#include "calendar.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

std::string safeTimestamp() {
    time_t time = std::time(nullptr);
    std::tm* ltm = std::localtime(&time);
    std::ostringstream result;
    result << std::setfill('0');
    result << (ltm->tm_year + 1900) << '-' << std::setw(2) << (ltm->tm_mon + 1) << '-'
           << std::setw(2) << (ltm->tm_mday) << '-' << std::setw(2) << (ltm->tm_hour)
           << std::setw(2) << (ltm->tm_min) << std::setw(2) << (ltm->tm_sec);
    return result.str();
}

std::string timestamp() {
    time_t time = std::time(nullptr);
    std::tm* ltm = std::localtime(&time);
    std::ostringstream result;
    result << std::setfill('0');
    result << (ltm->tm_year + 1900) << '-' << std::setw(2) << (ltm->tm_mon + 1) << '-'
           << std::setw(2) << (ltm->tm_mday) << ' ' << std::setw(2) << (ltm->tm_hour) << ':'
           << std::setw(2) << (ltm->tm_min) << ':' << std::setw(2) << (ltm->tm_sec);
    return result.str();
}
