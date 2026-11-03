#include "domain/DailyLatePenaltyStrategy.hpp"

#include <stdexcept>

namespace lms::domain {

DailyLatePenaltyStrategy::DailyLatePenaltyStrategy(double penaltyPerDay, int maxLateDays)
    : penaltyPerDay_(penaltyPerDay), maxLateDays_(maxLateDays) {
    if (!(penaltyPerDay > 0.0 && penaltyPerDay <= 1.0)) {
        throw std::invalid_argument("penaltyPerDay must be in (0, 1]");
    }
    if (maxLateDays < 0) {
        throw std::invalid_argument("maxLateDays must not be negative");
    }
}

LateResult DailyLatePenaltyStrategy::evaluate(Timestamp submittedAt, Timestamp effectiveDueAt) const {
    LateResult r;
    if (submittedAt <= effectiveDueAt) return r;  // on time (the deadline instant itself is on time)

    const Timestamp secondsLate = submittedAt - effectiveDueAt;
    r.isLate = true;
    r.daysLate = static_cast<int>((secondsLate + kSecondsPerDay - 1) / kSecondsPerDay);  // ceil
    if (r.daysLate > maxLateDays_) {
        r.zeroMark = true;
        r.penaltyFraction = 1.0;
    } else {
        r.penaltyFraction = r.daysLate * penaltyPerDay_;
    }
    return r;
}

}  // namespace lms::domain
