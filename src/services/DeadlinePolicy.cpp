#include "services/DeadlinePolicy.hpp"

#include <algorithm>

namespace lms::services {

domain::Timestamp DeadlinePolicy::effectiveDueAt(const domain::Assignment& assignment, int studentId) const {
    const auto extended = extensions_.latestApprovedDueAt(assignment.id, studentId);
    return extended ? std::max(assignment.dueAt, *extended) : assignment.dueAt;
}

}  // namespace lms::services
