#pragma once
#include <optional>
#include <vector>

#include "domain/Records.hpp"

namespace lms::data {

class IMarkRepository {
public:
    virtual ~IMarkRepository() = default;
    virtual int add(const domain::Mark& mark) = 0;
    virtual void update(const domain::Mark& mark) = 0;
    virtual std::optional<domain::Mark> findById(int id) const = 0;
    virtual std::optional<domain::Mark> findBySubmission(int submissionId) const = 0;
    // Marks in the given status for courses of one department, oldest first.
    virtual std::vector<domain::Mark> listByStatusForDepartment(domain::MarkStatus status,
                                                                int departmentId) const = 0;
    virtual int addModeration(const domain::ModerationRecord& record) = 0;
    virtual std::vector<domain::ModerationRecord> moderationsFor(int markId) const = 0;
};

}  // namespace lms::data
