#pragma once
// Core value types and enumerations shared by every layer.
#include <cstdint>
#include <string>

namespace lms::domain {

using Timestamp = std::int64_t;  // seconds since the Unix epoch, UTC
constexpr Timestamp kSecondsPerDay = 86400;

enum class Role { Administrator, Lecturer, HeadOfDepartment, Student };

enum class SubmissionStatus {
    Draft,
    Submitted,
    UnderReview,
    MarkedPendingModeration,
    ChangesRequested,
    Approved,
    Released,
    Superseded
};

enum class MarkStatus { PendingModeration, ChangesRequested, Approved, Released };
enum class ExtensionStatus { Pending, Approved, Rejected };
enum class ModerationDecision { Approved, ChangesRequested };
enum class EnrolmentStatus { Active, Withdrawn };

// String codes match the CHECK constraints in sql/schema.sql.
// The *FromString functions throw std::invalid_argument on unknown input.
std::string toString(Role r);
std::string toString(SubmissionStatus s);
std::string toString(MarkStatus s);
std::string toString(ExtensionStatus s);
std::string toString(ModerationDecision d);
std::string toString(EnrolmentStatus s);

Role roleFromString(const std::string& s);
SubmissionStatus submissionStatusFromString(const std::string& s);
MarkStatus markStatusFromString(const std::string& s);
ExtensionStatus extensionStatusFromString(const std::string& s);
ModerationDecision moderationDecisionFromString(const std::string& s);
EnrolmentStatus enrolmentStatusFromString(const std::string& s);

}  // namespace lms::domain
