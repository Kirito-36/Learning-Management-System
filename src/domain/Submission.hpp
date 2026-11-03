#pragma once
#include <string>
#include <utility>

#include "domain/SubmissionStateMachine.hpp"
#include "domain/Types.hpp"

namespace lms::domain {

// One attempt at an assignment. Resubmissions create new Submission objects; the
// earlier attempt is moved to Superseded and kept for the record.
class Submission {
public:
    Submission(int id, int assignmentId, int studentId, int attemptNo, Timestamp submittedAt,
               std::string filePath, SubmissionStatus status, bool isLate)
        : id_(id),
          assignmentId_(assignmentId),
          studentId_(studentId),
          attemptNo_(attemptNo),
          submittedAt_(submittedAt),
          filePath_(std::move(filePath)),
          status_(status),
          isLate_(isLate) {}

    int id() const noexcept { return id_; }
    int assignmentId() const noexcept { return assignmentId_; }
    int studentId() const noexcept { return studentId_; }
    int attemptNo() const noexcept { return attemptNo_; }
    Timestamp submittedAt() const noexcept { return submittedAt_; }
    const std::string& filePath() const noexcept { return filePath_; }
    SubmissionStatus status() const noexcept { return status_; }
    bool isLate() const noexcept { return isLate_; }

    // Throws InvalidTransition if the life cycle does not allow the change.
    void transitionTo(SubmissionStatus next) {
        status_ = SubmissionStateMachine::transition(status_, next);
    }

private:
    int id_;
    int assignmentId_;
    int studentId_;
    int attemptNo_;
    Timestamp submittedAt_;
    std::string filePath_;
    SubmissionStatus status_;
    bool isLate_;
};

}  // namespace lms::domain
