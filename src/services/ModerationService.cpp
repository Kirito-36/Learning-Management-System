#include "services/ModerationService.hpp"

#include "services/Guards.hpp"

using namespace lms::domain;

namespace lms::services {

ModerationService::ModerationService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                                     data::ISubmissionRepository& submissions, data::IMarkRepository& marks,
                                     AuditService& audit, data::IUnitOfWork& uow, const IClock& clock)
    : courses_(courses), assignments_(assignments), submissions_(submissions), marks_(marks), audit_(audit),
      uow_(uow), clock_(clock) {}

std::vector<Mark> ModerationService::pendingMarks(const User& hod) const {
    requirePermission(hod, Permission::ModerateMarks, "moderate marks");
    return marks_.listByStatusForDepartment(MarkStatus::PendingModeration, departmentOf(hod).value_or(0));
}

std::vector<Mark> ModerationService::approvedMarks(const User& hod) const {
    requirePermission(hod, Permission::ReleaseMarks, "release marks");
    return marks_.listByStatusForDepartment(MarkStatus::Approved, departmentOf(hod).value_or(0));
}

Mark ModerationService::loadForDepartment(const User& hod, int markId, Submission& submissionOut) const {
    auto mark = marks_.findById(markId);
    if (!mark) throw NotFound("Mark not found.");
    auto submission = submissions_.findById(mark->submissionId);
    if (!submission) throw NotFound("Submission not found.");
    const auto assignment = assignments_.findById(submission->assignmentId());
    const auto course = assignment ? courses_.findCourse(assignment->courseId) : std::nullopt;
    if (!course) throw NotFound("Course not found.");
    if (departmentOf(hod) != course->departmentId) {
        throw AccessDenied("This mark belongs to another department.");
    }
    submissionOut = *submission;
    return *mark;
}

void ModerationService::review(const User& hod, int markId, ModerationDecision decision, const std::string& comment) {
    requirePermission(hod, Permission::ModerateMarks, "moderate marks");
    Submission submission(0, 0, 0, 1, 0, "x", SubmissionStatus::Draft, false);
    Mark mark = loadForDepartment(hod, markId, submission);
    if (mark.status != MarkStatus::PendingModeration) {
        throw RuleViolation("This mark is not awaiting moderation.");
    }
    if (decision == ModerationDecision::ChangesRequested) requireText(comment, "Comment", 1000);
    if (comment.size() > 1000) throw ValidationError("Comment must be at most 1000 characters.");

    const bool approve = decision == ModerationDecision::Approved;
    const MarkStatus before = mark.status;
    mark.status = approve ? MarkStatus::Approved : MarkStatus::ChangesRequested;
    submission.transitionTo(approve ? SubmissionStatus::Approved : SubmissionStatus::ChangesRequested);

    auto scope = uow_.begin();
    marks_.update(mark);
    submissions_.save(submission);
    ModerationRecord record;
    record.markId = markId;
    record.reviewedBy = hod.id();
    record.decision = decision;
    record.comment = comment;
    record.decidedAt = clock_.now();
    marks_.addModeration(record);
    audit_.record(hod.id(), "MODERATE_MARK", "mark", markId, toString(before), toString(mark.status),
                  comment.empty() ? std::optional<std::string>("approved") : std::optional<std::string>(comment));
    scope->commit();
}

void ModerationService::release(const User& hod, int markId) {
    requirePermission(hod, Permission::ReleaseMarks, "release marks");
    Submission submission(0, 0, 0, 1, 0, "x", SubmissionStatus::Draft, false);
    Mark mark = loadForDepartment(hod, markId, submission);
    if (mark.status != MarkStatus::Approved) throw RuleViolation("Only approved marks can be released.");

    mark.status = MarkStatus::Released;
    mark.releasedAt = clock_.now();
    submission.transitionTo(SubmissionStatus::Released);

    auto scope = uow_.begin();
    marks_.update(mark);
    submissions_.save(submission);
    audit_.record(hod.id(), "RELEASE_MARK", "mark", markId, toString(MarkStatus::Approved),
                  toString(MarkStatus::Released), "released to student");
    scope->commit();
}

}  // namespace lms::services
