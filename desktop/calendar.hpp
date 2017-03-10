#ifndef FMO_DESKTOP_CALENDAR_HPP
#define FMO_DESKTOP_CALENDAR_HPP

#include <iostream>

/// Timestamp with second precision safe to be used in filenames.
std::string safeTimestamp();

/// Timestamp with second precision unsafe to be used in filenames, but more readable.
std::string timestamp();

#endif // FMO_DESKTOP_CALENDAR_HPP
