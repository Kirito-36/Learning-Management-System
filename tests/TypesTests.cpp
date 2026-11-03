#include <gtest/gtest.h>

#include "domain/Types.hpp"

using namespace lms::domain;

TEST(TypesCodes, AllEnumerationsRoundTripThroughTheirDatabaseCodes) {
    for (Role v : {Role::Administrator, Role::Lecturer, Role::HeadOfDepartment, Role::Student})
        EXPECT_EQ(roleFromString(toString(v)), v);
    for (MarkStatus v : {MarkStatus::PendingModeration, MarkStatus::ChangesRequested,
                         MarkStatus::Approved, MarkStatus::Released})
        EXPECT_EQ(markStatusFromString(toString(v)), v);
    for (ExtensionStatus v : {ExtensionStatus::Pending, ExtensionStatus::Approved, ExtensionStatus::Rejected})
        EXPECT_EQ(extensionStatusFromString(toString(v)), v);
    for (ModerationDecision v : {ModerationDecision::Approved, ModerationDecision::ChangesRequested})
        EXPECT_EQ(moderationDecisionFromString(toString(v)), v);
    for (EnrolmentStatus v : {EnrolmentStatus::Active, EnrolmentStatus::Withdrawn})
        EXPECT_EQ(enrolmentStatusFromString(toString(v)), v);
}

TEST(TypesCodes, UnknownCodesAreRejected) {
    EXPECT_THROW(markStatusFromString("DONE"), std::invalid_argument);
    EXPECT_THROW(extensionStatusFromString(""), std::invalid_argument);
    EXPECT_THROW(moderationDecisionFromString("maybe"), std::invalid_argument);
    EXPECT_THROW(enrolmentStatusFromString("gone"), std::invalid_argument);
    EXPECT_THROW(submissionStatusFromString("released"), std::invalid_argument);  // case-sensitive
}
