#include "domain/TimeUtil.hpp"

#include <cstdio>
#include <ctime>

namespace lms::domain {
namespace {

// Howard Hinnant's civil-calendar algorithms (public domain).
Timestamp daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2 ? 1 : 0;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<Timestamp>(era) * 146097 + static_cast<Timestamp>(doe) - 719468;
}

void civilFromDays(Timestamp z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const Timestamp era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    if (m <= 2) ++y;
}

}  // namespace

std::optional<Timestamp> parseDateTime(const std::string& text) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, consumed = 0;
    if (std::sscanf(text.c_str(), "%4d-%2d-%2d %2d:%2d%n", &y, &mo, &d, &h, &mi, &consumed) != 5 ||
        static_cast<std::size_t>(consumed) != text.size()) {
        return std::nullopt;
    }
    if (y < 2000 || y > 2100 || mo < 1 || mo > 12 || d < 1 || d > 31 || h < 0 || h > 23 ||
        mi < 0 || mi > 59) {
        return std::nullopt;
    }
    const Timestamp t = daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d)) *
                            kSecondsPerDay + h * 3600 + mi * 60;
    if (formatDateTime(t) != text) return std::nullopt;  // rejects e.g. 2026-02-31
    return t;
}

std::string formatDateTime(Timestamp t) {
    Timestamp days = t / kSecondsPerDay;
    Timestamp secs = t % kSecondsPerDay;
    if (secs < 0) {
        secs += kSecondsPerDay;
        --days;
    }
    int y = 0;
    unsigned m = 0, d = 0;
    civilFromDays(days, y, m, d);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%04d-%02u-%02u %02d:%02d", y, m, d, static_cast<int>(secs / 3600),
                  static_cast<int>((secs % 3600) / 60));
    return buf;
}

Timestamp SystemClock::now() const {
    const std::time_t raw = std::time(nullptr);
    const std::tm* local = std::localtime(&raw);
    return daysFromCivil(local->tm_year + 1900, static_cast<unsigned>(local->tm_mon + 1),
                         static_cast<unsigned>(local->tm_mday)) * kSecondsPerDay +
           local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
}

}  // namespace lms::domain
