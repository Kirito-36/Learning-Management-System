#include "services/ResultsService.hpp"

#include "services/Guards.hpp"

using namespace lms::domain;

namespace lms::services {

std::vector<std::pair<Submission, Mark>> ResultsService::releasedFor(const User& student) const {
    requirePermission(student, Permission::ViewOwnMarks, "view marks");
    std::vector<std::pair<Submission, Mark>> out;
    for (const auto& s : submissions_.listByStudent(student.id())) {
        const auto mark = marks_.findBySubmission(s.id());
        if (mark && mark->status == MarkStatus::Released) out.emplace_back(s, *mark);
    }
    return out;
}

}  // namespace lms::services
