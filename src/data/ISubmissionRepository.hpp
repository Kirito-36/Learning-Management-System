#pragma once
#include <optional>
#include <vector>

#include "domain/Submission.hpp"

namespace lms::data {

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    // Inserts when submission.id() == 0, otherwise updates. Returns the stored id.
    virtual int save(const domain::Submission& submission) = 0;
    virtual std::optional<domain::Submission> findById(int id) const = 0;
    // Attempts in ascending attempt order.
    virtual std::vector<domain::Submission> findByAssignmentAndStudent(int assignmentId,
                                                                       int studentId) const = 0;
    virtual int countAttempts(int assignmentId, int studentId) const = 0;
    virtual std::vector<domain::Submission> listByAssignment(int assignmentId) const = 0;
    virtual std::vector<domain::Submission> listByStudent(int studentId) const = 0;
};

}  // namespace lms::data
