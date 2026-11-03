#include "services/SubmissionService.hpp"

#include "services/Guards.hpp"

using namespace lms::domain;

namespace lms::services {

SubmissionService::SubmissionService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                                     data::ISubmissionRepository& submissions, const DeadlinePolicy& deadlines,
                                     const ILatePenaltyStrategy& penalty, AuditService& audit, data::IUnitOfWork& uow,
                                     const IClock& clock)
    : courses_(courses), assignments_(assignments), submissions_(submissions), deadlines_(deadlines),
      penalty_(penalty), audit_(audit), uow_(uow), clock_(clock) {}

SubmitResult SubmissionService::submit(const User& student, int assignmentId, const std::string& filePath) {
    requirePermission(student, Permission::SubmitWork, "submit work");
    requireText(filePath, "File path", 260);
    if (filePath.find('\0') != std::string::npos) throw ValidationError("File path contains invalid characters.");

    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFound("Assignment not found.");
    if (!courses_.isEnrolled(student.id(), assignment->courseId)) {
        throw AccessDenied("You are not enrolled in this course.");
    }

    const Timestamp now = clock_.now();
    const Timestamp due = deadlines_.effectiveDueAt(*assignment, student.id());
    const LateResult lateness = penalty_.evaluate(now, due);
    auto attempts = submissions_.findByAssignmentAndStudent(assignmentId, student.id());

    if (static_cast<int>(attempts.size()) >= assignment->maxAttempts) {
        throw RuleViolation("You have used all " + std::to_string(assignment->maxAttempts) + " attempts.");
    }
    if (!attempts.empty()) {
        const Submission& previous = attempts.back();
        if (now > due) throw RuleViolation("Resubmission is only allowed before the deadline.");
        if (previous.status() != SubmissionStatus::Submitted && previous.status() != SubmissionStatus::UnderReview) {
            throw RuleViolation("Your previous attempt has already been marked and cannot be replaced.");
        }
    }

    auto scope = uow_.begin();
    if (!attempts.empty()) {
        auto previous = attempts.back();
        previous.transitionTo(SubmissionStatus::Superseded);
        submissions_.save(previous);
    }
    const int attemptNo = static_cast<int>(attempts.size()) + 1;
    const Submission fresh(0, assignmentId, student.id(), attemptNo, now, filePath, SubmissionStatus::Submitted,
                           lateness.isLate);
    const int id = submissions_.save(fresh);
    audit_.record(student.id(), attemptNo > 1 ? "RESUBMIT" : "SUBMIT", "submission", id, std::nullopt,
                  "attempt " + std::to_string(attemptNo) + (lateness.isLate ? ", late" : ", on time"));
    scope->commit();
    return SubmitResult{Submission(id, assignmentId, student.id(), attemptNo, now, filePath,
                                   SubmissionStatus::Submitted, lateness.isLate),
                        lateness};
}

std::vector<Submission> SubmissionService::mySubmissions(const User& student) const {
    requirePermission(student, Permission::SubmitWork, "view your submissions");
    return submissions_.listByStudent(student.id());
}

}  // namespace lms::services
