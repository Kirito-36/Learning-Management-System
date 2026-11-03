#pragma once
#include "domain/ILatePenaltyStrategy.hpp"

namespace lms::domain {

// HoD rule: lose penaltyPerDay of the available mark for each day late, up to maxLateDays;
// later than that the work receives zero. A partial day counts as a whole day.
class DailyLatePenaltyStrategy final : public ILatePenaltyStrategy {
public:
    // Throws std::invalid_argument unless 0 < penaltyPerDay <= 1 and maxLateDays >= 0.
    explicit DailyLatePenaltyStrategy(double penaltyPerDay = 0.10, int maxLateDays = 3);
    LateResult evaluate(Timestamp submittedAt, Timestamp effectiveDueAt) const override;

private:
    double penaltyPerDay_;
    int maxLateDays_;
};

}  // namespace lms::domain
