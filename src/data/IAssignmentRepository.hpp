#pragma once
#include <optional>
#include <vector>

#include "domain/Records.hpp"

namespace lms::data {

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual int add(const domain::Assignment& assignment) = 0;
    virtual std::optional<domain::Assignment> findById(int id) const = 0;
    virtual std::vector<domain::Assignment> listByCourse(int courseId) const = 0;
};

class IExtensionRepository {
public:
    virtual ~IExtensionRepository() = default;
    virtual int add(const domain::Extension& extension) = 0;
    // Latest due date among APPROVED extensions for this student and assignment, if any.
    virtual std::optional<domain::Timestamp> latestApprovedDueAt(int assignmentId, int studentId) const = 0;
    virtual std::vector<domain::Extension> listForAssignment(int assignmentId) const = 0;
};

}  // namespace lms::data
