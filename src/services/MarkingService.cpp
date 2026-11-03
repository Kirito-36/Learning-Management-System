#include "services/MarkingService.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "services/Guards.hpp"

using namespace lms::domain;

namespace lms::services {
namespace {
std::string describe(double raw, double final) {
    std::ostringstream os;
    os << "raw=" << raw << ", final=" << final;
    return os.str();
}
}  // namespace

MarkingService::MarkingService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                               data::ISubmissionRepository& submissions, data::IMarkRepository& marks,
                               const DeadlinePolicy& deadlines, const ILatePenaltyStrategy& penalty,
                               AuditService& audit, data::IUnitOfWork& uow, const IClock& clock)
    : courses_(courses), assignments_(assignments), submissions_(submissions), marks_(marks),
      deadlines_(deadlines), penalty_(penalty), audit_(audit), uow_(uow), clock_(clock) {}

Mark MarkingService::saveMark(const User& lecturer, int submissionId, double rawMark, const std::string& feedback,
                              const std::string& reason) {
    requirePermission(lecturer, Permission::MarkSubmission, "mark submissions");
    auto submission = submissions_.findById(submissionId);
    if (!submission) throw NotFound("Submission not found.");
    const auto assignment = assignments_.findById(submission->assignmentId());
    if (!assignment) throw NotFound("Assignment not found.");
    if (!courses_.isLecturerOfCourse(assignment->courseId, lecturer.id())) {
        throw AccessDenied("You do not teach this course.");
    }
    if (!std::isfinite(rawMark) || rawMark < 0.0 || rawMark > assignment->maxMark) {
        throw ValidationError("Mark must be between 0 and " + std::to_string(static_cast<int>(assignment->maxMark)) +
                              ".");
    }
    requireText(feedback, "Feedback", 4000);

    const SubmissionStatus status = submission->status();
    if (status != SubmissionStatus::Submitted && status != SubmissionStatus::UnderReview &&
        status != SubmissionStatus::ChangesRequested) {
        throw RuleViolation("This submission cannot be marked in its current state.");
    }

    const Timestamp due = deadlines_.effectiveDueAt(*assignment, submission->studentId());
    const LateResult lateness = penalty_.evaluate(submission->submittedAt(), due);
    const double finalMark = lateness.applyTo(rawMark, assignment->maxMark);

    auto existing = marks_.findBySubmission(submissionId);
    if (existing) requireText(reason, "Reason for changing the mark", 500);

    auto scope = uow_.begin();
    if (status != SubmissionStatus::UnderReview) submission->transitionTo(SubmissionStatus::UnderReview);
    submission->transitionTo(SubmissionStatus::MarkedPendingModeration);
    submissions_.save(*submission);

    Mark mark;
    if (existing) {
        const std::string before = describe(existing->rawMark, existing->finalMark);
        mark = *existing;
        mark.markedBy = lecturer.id();
        mark.rawMark = rawMark;
        mark.finalMark = finalMark;
        mark.feedback = feedback;
        mark.status = MarkStatus::PendingModeration;
        mark.markedAt = clock_.now();
        marks_.update(mark);
        audit_.record(lecturer.id(), "MARK_CHANGED", "mark", mark.id, before, describe(rawMark, finalMark), reason);
    } else {
        mark.submissionId = submissionId;
        mark.markedBy = lecturer.id();
        mark.rawMark = rawMark;
        mark.finalMark = finalMark;
        mark.feedback = feedback;
        mark.status = MarkStatus::PendingModeration;
        mark.markedAt = clock_.now();
        mark.id = marks_.add(mark);
        audit_.record(lecturer.id(), "MARK_CREATED", "mark", mark.id, std::nullopt, describe(rawMark, finalMark),
                      "initial marking");
    }
    scope->commit();
    return mark;
}

std::vector<Submission> MarkingService::submissionsToMark(const User& lecturer, int assignmentId) const {
    requirePermission(lecturer, Permission::MarkSubmission, "mark submissions");
    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFound("Assignment not found.");
    if (!courses_.isLecturerOfCourse(assignment->courseId, lecturer.id())) {
        throw AccessDenied("You do not teach this course.");
    }

    const auto submissions = submissions_.listByAssignment(assignmentId);
    std::vector<Submission> out;
    std::copy_if(submissions.begin(), submissions.end(), std::back_inserter(out),
                 [](const Submission& s) { return s.status() != SubmissionStatus::Superseded; });
    return out;
}

}  // namespace lms::services
