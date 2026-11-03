// Equivalence partitioning and boundary value analysis for the late-penalty rule.
#include <gtest/gtest.h>

#include "domain/DailyLatePenaltyStrategy.hpp"

using namespace lms::domain;

namespace {
constexpr Timestamp kDue = 1'800'000'000;
constexpr Timestamp kDay = kSecondsPerDay;
const DailyLatePenaltyStrategy kStrategy;  // 10% per day, 3 days
}  // namespace

TEST(Penalty, EarlySubmissionIsOnTime) {
    const auto r = kStrategy.evaluate(kDue - 3600, kDue);
    EXPECT_FALSE(r.isLate);
    EXPECT_EQ(r.daysLate, 0);
    EXPECT_NEAR(r.penaltyFraction, 0.0, 1e-9);
}

TEST(Penalty, SubmissionAtDeadlineInstantIsOnTime) {
    EXPECT_FALSE(kStrategy.evaluate(kDue, kDue).isLate);
}

TEST(Penalty, OneSecondLateCountsAsOneDay) {
    const auto r = kStrategy.evaluate(kDue + 1, kDue);
    EXPECT_TRUE(r.isLate);
    EXPECT_EQ(r.daysLate, 1);
    EXPECT_NEAR(r.penaltyFraction, 0.10, 1e-9);
}

TEST(Penalty, ExactlyOneDayLateIsStillOneDay) {
    EXPECT_EQ(kStrategy.evaluate(kDue + kDay, kDue).daysLate, 1);
}

TEST(Penalty, OneDayAndOneSecondIsTwoDays) {
    const auto r = kStrategy.evaluate(kDue + kDay + 1, kDue);
    EXPECT_EQ(r.daysLate, 2);
    EXPECT_NEAR(r.penaltyFraction, 0.20, 1e-9);
}

TEST(Penalty, ExactlyThreeDaysLateIsLastPenalisedDay) {
    const auto r = kStrategy.evaluate(kDue + 3 * kDay, kDue);
    EXPECT_EQ(r.daysLate, 3);
    EXPECT_FALSE(r.zeroMark);
    EXPECT_NEAR(r.penaltyFraction, 0.30, 1e-9);
}

TEST(Penalty, MoreThanThreeDaysLateScoresZero) {
    const auto r = kStrategy.evaluate(kDue + 3 * kDay + 1, kDue);
    EXPECT_TRUE(r.zeroMark);
    EXPECT_EQ(r.daysLate, 4);
    EXPECT_NEAR(r.applyTo(95.0, 100.0), 0.0, 1e-9);
}

TEST(Penalty, ApplyDeductsPercentageOfMaximumMark) {
    const auto twoDays = kStrategy.evaluate(kDue + kDay + 1, kDue);
    EXPECT_NEAR(twoDays.applyTo(80.0, 100.0), 60.0, 1e-9);
    EXPECT_NEAR(twoDays.applyTo(40.0, 50.0), 30.0, 1e-9);  // 20% of 50 = 10 deducted
}

TEST(Penalty, ApplyNeverGoesBelowZero) {
    const auto threeDays = kStrategy.evaluate(kDue + 3 * kDay, kDue);
    EXPECT_NEAR(threeDays.applyTo(15.0, 100.0), 0.0, 1e-9);
}

TEST(Penalty, OnTimeWorkKeepsRawMark) {
    EXPECT_NEAR(kStrategy.evaluate(kDue - 1, kDue).applyTo(72.5, 100.0), 72.5, 1e-9);
}

TEST(Penalty, ExtendedDeadlineRemovesPenalty) {
    // The caller passes the *effective* deadline, so an extension shifts the whole window.
    const Timestamp submitted = kDue + 2 * kDay;
    EXPECT_TRUE(kStrategy.evaluate(submitted, kDue).isLate);
    EXPECT_FALSE(kStrategy.evaluate(submitted, kDue + 3 * kDay).isLate);
}

TEST(Penalty, ConstructorRejectsInvalidParameters) {
    EXPECT_THROW(DailyLatePenaltyStrategy(0.0, 3), std::invalid_argument);
    EXPECT_THROW(DailyLatePenaltyStrategy(1.5, 3), std::invalid_argument);
    EXPECT_THROW(DailyLatePenaltyStrategy(0.1, -1), std::invalid_argument);
    EXPECT_NO_THROW(DailyLatePenaltyStrategy(1.0, 0));
}

TEST(Penalty, StrategyIsReplaceableThroughInterface) {
    const DailyLatePenaltyStrategy strict(0.5, 1);
    const ILatePenaltyStrategy& policy = strict;
    EXPECT_NEAR(policy.evaluate(kDue + 1, kDue).penaltyFraction, 0.5, 1e-9);
    EXPECT_TRUE(policy.evaluate(kDue + kDay + 1, kDue).zeroMark);
}
