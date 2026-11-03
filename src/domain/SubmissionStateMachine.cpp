#include "domain/SubmissionStateMachine.hpp"

#include <string>

namespace lms::domain {

bool SubmissionStateMachine::canTransition(SubmissionStatus from, SubmissionStatus to) noexcept {
    using S = SubmissionStatus;
    switch (from) {
        case S::Draft:
            return to == S::Submitted;
        case S::Submitted:
            return to == S::UnderReview || to == S::Superseded;
        case S::UnderReview:
            return to == S::MarkedPendingModeration || to == S::Superseded;
        case S::MarkedPendingModeration:
            return to == S::Approved || to == S::ChangesRequested;
        case S::ChangesRequested:
            return to == S::UnderReview;
        case S::Approved:
            return to == S::Released;
        case S::Released:    // terminal
        case S::Superseded:  // terminal
            return false;
    }
    return false;
}

SubmissionStatus SubmissionStateMachine::transition(SubmissionStatus from, SubmissionStatus to) {
    if (!canTransition(from, to)) {
        throw InvalidTransition("illegal submission transition " + toString(from) + " -> " +
                                toString(to));
    }
    return to;
}

}  // namespace lms::domain
