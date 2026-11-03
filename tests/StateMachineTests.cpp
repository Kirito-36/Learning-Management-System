#include <gtest/gtest.h>

#include "domain/Submission.hpp"

using namespace lms::domain;
using S = SubmissionStatus;

TEST(StateMachine, HappyPathReachesReleased) {
    Submission s(1, 1, 1, 1, 0, "work.pdf", S::Draft, false);
    for (S next : {S::Submitted, S::UnderReview, S::MarkedPendingModeration, S::Approved, S::Released}) {
        EXPECT_NO_THROW(s.transitionTo(next));
        EXPECT_EQ(s.status(), next);
    }
}

TEST(StateMachine, ChangesRequestedLoopsBackToReview) {
    EXPECT_TRUE(SubmissionStateMachine::canTransition(S::MarkedPendingModeration, S::ChangesRequested));
    EXPECT_TRUE(SubmissionStateMachine::canTransition(S::ChangesRequested, S::UnderReview));
}

TEST(StateMachine, ResubmissionSupersedesEarlierAttempt) {
    EXPECT_TRUE(SubmissionStateMachine::canTransition(S::Submitted, S::Superseded));
    EXPECT_TRUE(SubmissionStateMachine::canTransition(S::UnderReview, S::Superseded));
}

TEST(StateMachine, MarksCannotBeReleasedWithoutApproval) {
    EXPECT_FALSE(SubmissionStateMachine::canTransition(S::UnderReview, S::Released));
    EXPECT_FALSE(SubmissionStateMachine::canTransition(S::MarkedPendingModeration, S::Released));
    EXPECT_FALSE(SubmissionStateMachine::canTransition(S::Submitted, S::Approved));
}

TEST(StateMachine, TerminalStatesAllowNothing) {
    for (S to : {S::Draft, S::Submitted, S::UnderReview, S::MarkedPendingModeration,
                 S::ChangesRequested, S::Approved, S::Released, S::Superseded}) {
        EXPECT_FALSE(SubmissionStateMachine::canTransition(S::Released, to));
        EXPECT_FALSE(SubmissionStateMachine::canTransition(S::Superseded, to));
    }
}

TEST(StateMachine, IllegalTransitionThrowsAndLeavesStatusUnchanged) {
    Submission s(1, 1, 1, 1, 0, "work.pdf", S::Submitted, false);
    EXPECT_THROW(s.transitionTo(S::Released), InvalidTransition);
    EXPECT_EQ(s.status(), S::Submitted);
}

TEST(Types, StringCodesRoundTrip) {
    for (S s : {S::Draft, S::Submitted, S::UnderReview, S::MarkedPendingModeration,
                S::ChangesRequested, S::Approved, S::Released, S::Superseded}) {
        EXPECT_EQ(submissionStatusFromString(toString(s)), s);
    }
    EXPECT_EQ(roleFromString("HOD"), Role::HeadOfDepartment);
    EXPECT_THROW(roleFromString("principal"), std::invalid_argument);
}
