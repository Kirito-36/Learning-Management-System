#pragma once
// School-local wall-clock time is stored as-is in a Timestamp (no time-zone conversion):
// the system runs on one school computer or LAN, so one clock and one zone apply.
#include <optional>
#include <string>

#include "domain/IClock.hpp"
#include "domain/Types.hpp"

namespace lms::domain {

// "YYYY-MM-DD HH:MM" -> Timestamp; std::nullopt for malformed or impossible dates.
std::optional<Timestamp> parseDateTime(const std::string& text);
std::string formatDateTime(Timestamp t);

class SystemClock final : public IClock {
public:
    Timestamp now() const override;  // local wall-clock time of the host computer
};

}  // namespace lms::domain
