// Integration tests: services + real SQLite repositories. Derived from the use case flows.
#include <gtest/gtest.h>

#include "TestWorld.hpp"
#include "domain/Errors.hpp"

using namespace lms;
using namespace lms::domain;
using lmstest::World;

TEST(Workflow, MarkIsHiddenFromStudentUntilHodReleasesIt) {
    World w;
    w.clock.t = w.due - 3600;
    const auto sub = w.submissionSvc.submit(*w.student, w.assignmentId, "work.pdf");
    EXPECT_FALSE(sub.lateness.isLate);
    EXPECT_TRUE(sub.submission.status() == SubmissionStatus::Submitted);

    const Mark mark = w.markingSvc.saveMark(*w.lecturer, sub.submission.id(), 82.5, "Solid report", "");
    EXPECT_TRUE(w.results.releasedFor(*w.student).empty());  // marked, not yet moderated

    w.moderationSvc.review(*w.hod, mark.id, ModerationDecision::Approved, "");
    EXPECT_TRUE(w.results.releasedFor(*w.student).empty());  // approved, not yet released

    w.moderationSvc.release(*w.hod, mark.id);
    const auto released = w.results.releasedFor(*w.student);
    EXPECT_EQ(static_cast<int>(released.size()), 1);
    EXPECT_NEAR(released[0].second.finalMark, 82.5, 1e-9);
    EXPECT_EQ(released[0].second.feedback, std::string("Solid report"));
    EXPECT_TRUE(released[0].first.status() == SubmissionStatus::Released);
}

TEST(Workflow, ChangesRequestedLoopThenApproval) {
    World w;
    const Mark first = w.submitAndMark(*w.student, w.due - 10, 60.0);
    w.moderationSvc.review(*w.hod, first.id, ModerationDecision::ChangesRequested, "Check question 3");
    EXPECT_EQ(static_cast<int>(w.moderationSvc.pendingMarks(*w.hod).size()), 0);

    const auto sub = w.submissionSvc.mySubmissions(*w.student).back();
    EXPECT_TRUE(sub.status() == SubmissionStatus::ChangesRequested);
    const Mark revised = w.markingSvc.saveMark(*w.lecturer, sub.id(), 68.0, "Reviewed", "Q3 marked too harshly");
    EXPECT_EQ(revised.id, first.id);  // same mark row, revised
    w.moderationSvc.review(*w.hod, revised.id, ModerationDecision::Approved, "");
    w.moderationSvc.release(*w.hod, revised.id);
    EXPECT_NEAR(w.results.releasedFor(*w.student)[0].second.finalMark, 68.0, 1e-9);
    EXPECT_EQ(static_cast<int>(w.marks.moderationsFor(first.id).size()), 2);
}

TEST(Workflow, ChangesRequestedNeedsAComment) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due - 10, 60.0);
    EXPECT_THROW(w.moderationSvc.review(*w.hod, m.id, ModerationDecision::ChangesRequested, "  "), ValidationError);
}

TEST(Workflow, LateWorkIsAcceptedFlaggedAndPenalisedAtMarking) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due + World::kDay + 1, 80.0);  // 2 days late
    EXPECT_NEAR(m.rawMark, 80.0, 1e-9);
    EXPECT_NEAR(m.finalMark, 60.0, 1e-9);
    EXPECT_TRUE(w.submissions.findById(m.submissionId)->isLate());
}

TEST(Workflow, WorkMoreThanThreeDaysLateIsAcceptedButScoresZero) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due + 3 * World::kDay + 1, 95.0);
    EXPECT_NEAR(m.finalMark, 0.0, 1e-9);
    EXPECT_NEAR(m.rawMark, 95.0, 1e-9);
}

TEST(Workflow, ResubmissionBeforeDeadlineSupersedesEarlierAttempt) {
    World w;
    w.clock.t = w.due - 5000;
    const auto a1 = w.submissionSvc.submit(*w.student, w.assignmentId, "v1.pdf");
    w.clock.t = w.due - 1000;
    const auto a2 = w.submissionSvc.submit(*w.student, w.assignmentId, "v2.pdf");
    EXPECT_EQ(a2.submission.attemptNo(), 2);
    EXPECT_TRUE(w.submissions.findById(a1.submission.id())->status() == SubmissionStatus::Superseded);
    EXPECT_EQ(static_cast<int>(w.markingSvc.submissionsToMark(*w.lecturer, w.assignmentId).size()), 1);
}

TEST(Workflow, ResubmissionAfterDeadlineIsRefused) {
    World w;
    w.clock.t = w.due - 1000;
    w.submissionSvc.submit(*w.student, w.assignmentId, "v1.pdf");
    w.clock.t = w.due + 1000;
    EXPECT_THROW(w.submissionSvc.submit(*w.student, w.assignmentId, "v2.pdf"), RuleViolation);
}

TEST(Workflow, AttemptLimitIsEnforced) {
    World w;
    const int a = w.assignmentSvc.createAssignment(*w.lecturer, w.courseId, "Two tries", "", w.due, 50.0, 2);
    w.clock.t = w.due - 100;
    w.submissionSvc.submit(*w.student, a, "1.pdf");
    w.submissionSvc.submit(*w.student, a, "2.pdf");
    EXPECT_THROW(w.submissionSvc.submit(*w.student, a, "3.pdf"), RuleViolation);
}

TEST(Workflow, MarkedWorkCannotBeReplacedByResubmission) {
    World w;
    w.submitAndMark(*w.student, w.due - 100, 70.0);
    EXPECT_THROW(w.submissionSvc.submit(*w.student, w.assignmentId, "again.pdf"), RuleViolation);
}

TEST(Workflow, ExtensionShiftsTheStudentsDeadline) {
    World w;
    w.assignmentSvc.grantExtension(*w.lecturer, w.assignmentId, w.student->id(), w.due + 2 * World::kDay,
                                   "Family emergency");
    w.clock.t = w.due + World::kDay;  // one day after the original deadline
    EXPECT_FALSE(w.submissionSvc.submit(*w.student, w.assignmentId, "a.pdf").lateness.isLate);
    EXPECT_TRUE(w.submissionSvc.submit(*w.student2, w.assignmentId, "b.pdf").lateness.isLate);  // no extension
}

TEST(Workflow, ClassExtensionUpdatesEveryActiveStudentAndTheirDisplayedDeadline) {
    World w;
    const Timestamp newDue = w.due + 2 * World::kDay;

    EXPECT_EQ(w.assignmentSvc.grantClassExtension(*w.lecturer, w.assignmentId, newDue, "School closure"), 2);

    const auto assignment = w.assignments.findById(w.assignmentId);
    ASSERT_TRUE(assignment.has_value());
    EXPECT_EQ(w.assignmentSvc.effectiveDueAtForStudent(*assignment, w.student->id()), newDue);
    EXPECT_EQ(w.assignmentSvc.effectiveDueAtForStudent(*assignment, w.student2->id()), newDue);

    w.clock.t = w.due + World::kDay;
    EXPECT_FALSE(w.submissionSvc.submit(*w.student, w.assignmentId, "student1.pdf").lateness.isLate);
    EXPECT_FALSE(w.submissionSvc.submit(*w.student2, w.assignmentId, "student2.pdf").lateness.isLate);
}

TEST(Workflow, ClassExtensionRejectsDeadlineThatWouldShortenAnExistingIndividualExtension) {
    World w;
    w.assignmentSvc.grantExtension(*w.lecturer, w.assignmentId, w.student->id(),
                                   w.due + 3 * World::kDay, "Individual arrangement");
    const auto assignment = w.assignments.findById(w.assignmentId);
    ASSERT_TRUE(assignment.has_value());

    EXPECT_THROW(w.assignmentSvc.grantClassExtension(*w.lecturer, w.assignmentId,
                                                     w.due + 2 * World::kDay, "Class extension"),
                 RuleViolation);
    EXPECT_EQ(w.assignmentSvc.effectiveDueAtForStudent(*assignment, w.student->id()),
              w.due + 3 * World::kDay);
    EXPECT_EQ(w.assignmentSvc.effectiveDueAtForStudent(*assignment, w.student2->id()), w.due);
}

TEST(Workflow, ExtensionMustMoveTheDeadlineLaterAndNeedsAReason) {
    World w;
    EXPECT_THROW(w.assignmentSvc.grantExtension(*w.lecturer, w.assignmentId, w.student->id(), w.due, "same"),
                 RuleViolation);
    EXPECT_THROW(w.assignmentSvc.grantExtension(*w.lecturer, w.assignmentId, w.student->id(), w.due + 100, ""),
                 ValidationError);
}

TEST(Workflow, MarkChangesAreAuditedWithOldNewReasonAndActor) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due - 10, 70.0);
    w.clock.t += 500;
    const auto sub = w.submissions.findById(m.submissionId);
    // Changing a mark that is already awaiting moderation is not allowed; HoD sends it back first.
    EXPECT_THROW(w.markingSvc.saveMark(*w.lecturer, sub->id(), 75.0, "x", "typo"), RuleViolation);
    w.moderationSvc.review(*w.hod, m.id, ModerationDecision::ChangesRequested, "Please recheck");
    EXPECT_THROW(w.markingSvc.saveMark(*w.lecturer, sub->id(), 75.0, "Rechecked", ""), ValidationError);  // no reason
    w.markingSvc.saveMark(*w.lecturer, sub->id(), 75.0, "Rechecked", "Missed a correct answer");

    const auto log = w.auditRepo.findByEntity("mark", m.id);
    ASSERT_TRUE(log.size() >= 3);
    EXPECT_EQ(log[0].action, std::string("MARK_CREATED"));
    const AuditEntry& change = log.back().action == "MARK_CHANGED" ? log.back() : log[log.size() - 2];
    EXPECT_EQ(change.action, std::string("MARK_CHANGED"));
    EXPECT_EQ(change.actorUserId.value_or(-1), w.lecturer->id());
    EXPECT_EQ(change.oldValue.value_or(""), std::string("raw=70, final=70"));
    EXPECT_EQ(change.newValue.value_or(""), std::string("raw=75, final=75"));
    EXPECT_EQ(change.reason.value_or(""), std::string("Missed a correct answer"));
    EXPECT_EQ(change.occurredAt, w.clock.t);
}

TEST(Workflow, InvalidMarksAreRejected) {
    World w;
    w.clock.t = w.due - 10;
    const auto id = w.submissionSvc.submit(*w.student, w.assignmentId, "w.pdf").submission.id();
    EXPECT_THROW(w.markingSvc.saveMark(*w.lecturer, id, -1.0, "f", ""), ValidationError);
    EXPECT_THROW(w.markingSvc.saveMark(*w.lecturer, id, 100.5, "f", ""), ValidationError);
    EXPECT_THROW(w.markingSvc.saveMark(*w.lecturer, id, 50.0, "", ""), ValidationError);  // feedback required
    EXPECT_NO_THROW(w.markingSvc.saveMark(*w.lecturer, id, 100.0, "Full marks", ""));
}
