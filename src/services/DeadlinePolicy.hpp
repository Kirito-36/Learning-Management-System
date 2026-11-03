#pragma once
#include "data/IAssignmentRepository.hpp"

namespace lms::services {

// Deadline rule: a student's effective deadline is the assignment due date, or the latest
// approved extension for that student when it is later.
class DeadlinePolicy {
public:
    explicit DeadlinePolicy(const data::IExtensionRepository& extensions) : extensions_(extensions) {}
    domain::Timestamp effectiveDueAt(const domain::Assignment& assignment, int studentId) const;

private:
    const data::IExtensionRepository& extensions_;
};

}  // namespace lms::services
