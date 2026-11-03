#pragma once
// Plain entity records used by the repositories and services.
#include <optional>
#include <string>

#include "domain/Types.hpp"

namespace lms::domain {

struct Department {
    int id = 0;
    std::string name;
    std::optional<int> hodUserId;
};

struct Course {
    int id = 0;
    std::string code;
    std::string title;
    int academicYear = 0;
    int departmentId = 0;
};

struct Assignment {
    int id = 0;
    int courseId = 0;
    int createdBy = 0;
    std::string title;
    std::string description;
    Timestamp dueAt = 0;
    double maxMark = 0.0;
    int maxAttempts = 1;
};

struct Extension {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    Timestamp requestedDueAt = 0;
    std::string reason;
    ExtensionStatus status = ExtensionStatus::Pending;
    std::optional<int> approvedBy;
};

struct Mark {
    int id = 0;
    int submissionId = 0;
    int markedBy = 0;
    double rawMark = 0.0;
    double finalMark = 0.0;  // raw mark after the late penalty
    std::string feedback;
    MarkStatus status = MarkStatus::PendingModeration;
    Timestamp markedAt = 0;
    std::optional<Timestamp> releasedAt;
};

struct ModerationRecord {
    int id = 0;
    int markId = 0;
    int reviewedBy = 0;
    ModerationDecision decision = ModerationDecision::Approved;
    std::string comment;
    Timestamp decidedAt = 0;
};

}  // namespace lms::domain
