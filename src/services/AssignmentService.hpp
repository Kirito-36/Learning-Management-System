#pragma once
#include <string>
#include <vector>

#include "data/IAssignmentRepository.hpp"
#include "data/ICourseRepository.hpp"
#include "data/IUnitOfWork.hpp"
#include "domain/IClock.hpp"
#include "services/AuditService.hpp"
#include "services/DeadlinePolicy.hpp"

namespace lms::services {

// Lecturer use cases around assignments and individual extensions.
class AssignmentService {
public:
    AssignmentService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                      data::IExtensionRepository& extensions, const DeadlinePolicy& deadlines, AuditService& audit,
                      data::IUnitOfWork& uow, const domain::IClock& clock);

    int createAssignment(const domain::User& lecturer, int courseId, const std::string& title,
                         const std::string& description, domain::Timestamp dueAt, double maxMark, int maxAttempts);

    // Grants an individual extension; the new date must be later than the student's current deadline.
    int grantExtension(const domain::User& lecturer, int assignmentId, int studentId,
                       domain::Timestamp newDueAt, const std::string& reason);

    // Grants the same deadline extension to every active student enrolled in the assignment's course.
    int grantClassExtension(const domain::User& lecturer, int assignmentId,
                            domain::Timestamp newDueAt, const std::string& reason);

    // Returns the effective deadline shown to a student in their assignment list.
    domain::Timestamp effectiveDueAtForStudent(const domain::Assignment& assignment, int studentId) const;

    // Assignments visible to the user: a student's enrolled courses or a lecturer's taught courses.
    std::vector<domain::Assignment> listFor(const domain::User& user) const;

private:
    data::ICourseRepository& courses_;
    data::IAssignmentRepository& assignments_;
    data::IExtensionRepository& extensions_;
    const DeadlinePolicy& deadlines_;
    AuditService& audit_;
    data::IUnitOfWork& uow_;
    const domain::IClock& clock_;
};

}  // namespace lms::services
