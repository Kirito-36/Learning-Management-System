#pragma once
#include <stdexcept>

#include "domain/Types.hpp"

namespace lms::domain {

class InvalidTransition : public std::logic_error {
public:
    using std::logic_error::logic_error;
};

// Encodes the Submission life cycle from the Phase 3 state machine diagram.
class SubmissionStateMachine {
public:
    static bool canTransition(SubmissionStatus from, SubmissionStatus to) noexcept;
    // Returns `to` when legal; throws InvalidTransition otherwise.
    static SubmissionStatus transition(SubmissionStatus from, SubmissionStatus to);
};

}  // namespace lms::domain
