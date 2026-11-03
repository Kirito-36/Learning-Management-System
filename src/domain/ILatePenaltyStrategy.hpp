#pragma once
#include "domain/LateResult.hpp"
#include "domain/Types.hpp"

namespace lms::domain {

// Strategy pattern: the late-penalty policy is replaceable without touching MarkingService.
class ILatePenaltyStrategy {
public:
    virtual ~ILatePenaltyStrategy() = default;
    virtual LateResult evaluate(Timestamp submittedAt, Timestamp effectiveDueAt) const = 0;
};

}  // namespace lms::domain
