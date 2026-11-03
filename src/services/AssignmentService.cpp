#include "services/AssignmentService.hpp"

#include <cmath>

#include "domain/TimeUtil.hpp"
#include "services/Guards.hpp"

using namespace lms::domain;

namespace lms::services {

AssignmentService::AssignmentService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                                     data::IExtensionRepository& extensions, const DeadlinePolicy& deadlines,
                                     AuditService& audit, data::IUnitOfWork& uow, const IClock& clock)
    : courses_(courses), assignments_(assignments), extensions_(extensions), deadlines_(deadlines),
      audit_(audit), uow_(uow), clock_(clock) {}

int AssignmentService::createAssignment(const User& lecturer, int courseId, const std::string& title,
                                        const std::string& description, Timestamp dueAt, double maxMark,
                                        int maxAttempts) {
    requirePermission(lecturer, Permission::CreateAssignment, "create assignments");
    if (!courses_.findCourse(courseId)) throw NotFound("Course not found.");
    if (!courses_.isLecturerOfCourse(courseId, lecturer.id())) throw AccessDenied("You do not teach this course.");
    requireText(title, "Title", 100);
    if (description.size() > 2000) throw ValidationError("Description must be at most 2000 characters.");
    if (!std::isfinite(maxMark) || maxMark <= 0.0 || maxMark > 1000.0) {
        throw ValidationError("Maximum mark must be greater than 0 and at most 1000.");
    }
    if (maxAttempts < 1 || maxAttempts > 10) throw ValidationError("Attempts must be between 1 and 10.");
    if (dueAt <= clock_.now()) throw RuleViolation("The deadline must be in the future.");

    Assignment a;
    a.courseId = courseId;
    a.createdBy = lecturer.id();
    a.title = title;
    a.description = description;
    a.dueAt = dueAt;
    a.maxMark = maxMark;
    a.maxAttempts = maxAttempts;
    auto scope = uow_.begin();
    const int id = assignments_.add(a);
    audit_.record(lecturer.id(), "CREATE_ASSIGNMENT", "assignment", id, std::nullopt, title);
    scope->commit();
    return id;
}

int AssignmentService::grantExtension(const User& lecturer, int assignmentId, int studentId, Timestamp newDueAt,
                                      const std::string& reason) {
    requirePermission(lecturer, Permission::GrantExtension, "grant extensions");
    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFound("Assignment not found.");
    if (!courses_.isLecturerOfCourse(assignment->courseId, lecturer.id())) {
        throw AccessDenied("You do not teach this course.");
    }
    if (!courses_.isEnrolled(studentId, assignment->courseId)) throw ValidationError("That student is not enrolled.");
    requireText(reason, "Reason", 500);
    const Timestamp current = deadlines_.effectiveDueAt(*assignment, studentId);
    if (newDueAt <= current) throw RuleViolation("The extension must be later than the current deadline.");

    Extension e;
    e.assignmentId = assignmentId;
    e.studentId = studentId;
    e.requestedDueAt = newDueAt;
    e.reason = reason;
    e.status = ExtensionStatus::Approved;
    e.approvedBy = lecturer.id();
    auto scope = uow_.begin();
    const int id = extensions_.add(e);
    audit_.record(lecturer.id(), "GRANT_EXTENSION", "assignment", assignmentId, formatDateTime(current),
                  formatDateTime(newDueAt), "student " + std::to_string(studentId) + ": " + reason);
    scope->commit();
    return id;
}

std::vector<Assignment> AssignmentService::listFor(const User& user) const {
    std::vector<Course> courseList;
    if (user.role() == Role::Student) {
        courseList = courses_.coursesForStudent(user.id());
    } else if (user.role() == Role::Lecturer || user.role() == Role::HeadOfDepartment) {
        courseList = courses_.coursesForLecturer(user.id());
    } else {
        return {};
    }
    std::vector<Assignment> out;
    for (const auto& c : courseList) {
        const auto items = assignments_.listByCourse(c.id);
        out.insert(out.end(), items.begin(), items.end());
    }
    return out;
}

}  // namespace lms::services
