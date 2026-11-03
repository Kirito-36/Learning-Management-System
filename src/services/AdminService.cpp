#include "services/AdminService.hpp"

#include <cctype>

#include "services/Guards.hpp"

using namespace lms::domain;

namespace lms::services {
namespace {

void validateUsername(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) throw ValidationError("Username must be 3 to 32 characters.");
    for (unsigned char c : u) {
        if (!(std::isalnum(c) || c == '.' || c == '_' || c == '-')) {
            throw ValidationError("Username may contain only letters, digits, '.', '_' and '-'.");
        }
    }
}

void validatePassword(const std::string& p) {
    if (p.size() < 8) throw ValidationError("Password must be at least 8 characters.");
    if (p.size() > 128) throw ValidationError("Password must be at most 128 characters.");
}

}  // namespace

AdminService::AdminService(data::IUserRepository& users, data::ICourseRepository& courses,
                           const IPasswordHasher& hasher, AuditService& audit, data::IUnitOfWork& uow,
                           const IClock& clock)
    : users_(users), courses_(courses), hasher_(hasher), audit_(audit), uow_(uow), clock_(clock) {}

int AdminService::createInitialAdministrator(const std::string& username, const std::string& name,
                                             const std::string& password) {
    if (users_.countAll() != 0) throw RuleViolation("The initial administrator can only be created on an empty system.");
    validateUsername(username);
    requireText(name, "Name", 100);
    validatePassword(password);
    auto scope = uow_.begin();
    const Administrator admin(0, name, username, hasher_.hash(password), true);
    const int id = users_.add(admin);
    audit_.record(std::nullopt, "CREATE_USER", "user", id, std::nullopt, toString(Role::Administrator),
                  "initial administrator");
    scope->commit();
    return id;
}

int AdminService::createUser(const User& actor, const NewUser& d) {
    requirePermission(actor, Permission::ManageUsers, "create user accounts");
    validateUsername(d.username);
    requireText(d.name, "Name", 100);
    validatePassword(d.password);
    if (users_.findByUsername(d.username)) throw ValidationError("That username is already taken.");

    const std::string hash = hasher_.hash(d.password);
    std::unique_ptr<User> user;
    switch (d.role) {
        case Role::Administrator:
            user = std::make_unique<Administrator>(0, d.name, d.username, hash, true);
            break;
        case Role::Student:
            requireText(d.studentNumber, "Student number", 32);
            user = std::make_unique<Student>(0, d.name, d.username, hash, true, d.studentNumber);
            break;
        case Role::Lecturer:
        case Role::HeadOfDepartment: {
            requireText(d.staffNumber, "Staff number", 32);
            const auto dept = courses_.findDepartment(d.departmentId);
            if (!dept) throw NotFound("Department not found.");
            if (d.role == Role::HeadOfDepartment && dept->hodUserId) {
                throw RuleViolation("That department already has a Head of Department.");
            }
            if (d.role == Role::Lecturer) {
                user = std::make_unique<Lecturer>(0, d.name, d.username, hash, true, d.staffNumber, d.departmentId);
            } else {
                user = std::make_unique<HeadOfDepartment>(0, d.name, d.username, hash, true, d.staffNumber,
                                                          d.departmentId);
            }
            break;
        }
    }
    auto scope = uow_.begin();
    const int id = users_.add(*user);
    if (d.role == Role::HeadOfDepartment) courses_.setHeadOfDepartment(d.departmentId, id);
    audit_.record(actor.id(), "CREATE_USER", "user", id, std::nullopt, toString(d.role));
    scope->commit();
    return id;
}

void AdminService::setUserActive(const User& actor, int userId, bool active) {
    requirePermission(actor, Permission::ManageUsers, "change user accounts");
    if (userId == actor.id()) throw RuleViolation("You cannot change your own active status.");
    auto user = users_.findById(userId);
    if (!user) throw NotFound("User not found.");
    if (user->active() == active) return;
    // The repository persists active/name/hash from the object, so rebuild with the new flag.
    std::unique_ptr<User> updated;
    const std::string hash = user->passwordHash();
    switch (user->role()) {
        case Role::Administrator:
            updated = std::make_unique<Administrator>(userId, user->name(), user->username(), hash, active);
            break;
        case Role::Student:
            updated = std::make_unique<Student>(userId, user->name(), user->username(), hash, active,
                                                static_cast<const Student&>(*user).studentNumber());
            break;
        case Role::Lecturer: {
            const auto& l = static_cast<const Lecturer&>(*user);
            updated = std::make_unique<Lecturer>(userId, l.name(), l.username(), hash, active, l.staffNumber(),
                                                 l.departmentId());
            break;
        }
        case Role::HeadOfDepartment: {
            const auto& l = static_cast<const HeadOfDepartment&>(*user);
            updated = std::make_unique<HeadOfDepartment>(userId, l.name(), l.username(), hash, active,
                                                         l.staffNumber(), l.departmentId());
            break;
        }
    }
    auto scope = uow_.begin();
    users_.update(*updated);
    audit_.record(actor.id(), "SET_ACTIVE", "user", userId, user->active() ? "active" : "inactive",
                  active ? "active" : "inactive");
    scope->commit();
}

int AdminService::createDepartment(const User& actor, const std::string& name) {
    requirePermission(actor, Permission::ManageCourses, "create departments");
    requireText(name, "Department name", 100);
    auto scope = uow_.begin();
    const int id = courses_.addDepartment(Department{0, name, std::nullopt});
    audit_.record(actor.id(), "CREATE_DEPARTMENT", "department", id, std::nullopt, name);
    scope->commit();
    return id;
}

int AdminService::createCourse(const User& actor, const Course& c) {
    requirePermission(actor, Permission::ManageCourses, "create courses");
    requireText(c.code, "Course code", 20);
    requireText(c.title, "Course title", 100);
    if (c.academicYear < 2000 || c.academicYear > 2100) throw ValidationError("Academic year must be 2000-2100.");
    if (!courses_.findDepartment(c.departmentId)) throw NotFound("Department not found.");
    auto scope = uow_.begin();
    const int id = courses_.addCourse(c);
    audit_.record(actor.id(), "CREATE_COURSE", "course", id, std::nullopt, c.code);
    scope->commit();
    return id;
}

void AdminService::assignLecturer(const User& actor, int courseId, int lecturerId) {
    requirePermission(actor, Permission::ManageCourses, "assign lecturers");
    const auto course = courses_.findCourse(courseId);
    if (!course) throw NotFound("Course not found.");
    const auto user = users_.findById(lecturerId);
    if (!user || !user->active() || (user->role() != Role::Lecturer && user->role() != Role::HeadOfDepartment)) {
        throw ValidationError("The selected user is not an active lecturer.");
    }
    if (departmentOf(*user) != course->departmentId) {
        throw RuleViolation("A lecturer can only teach courses in their own department.");
    }
    if (courses_.isLecturerOfCourse(courseId, lecturerId)) throw RuleViolation("Lecturer is already assigned.");
    auto scope = uow_.begin();
    courses_.assignLecturer(courseId, lecturerId);
    audit_.record(actor.id(), "ASSIGN_LECTURER", "course", courseId, std::nullopt, std::to_string(lecturerId));
    scope->commit();
}

void AdminService::enrolStudent(const User& actor, int studentId, int courseId) {
    requirePermission(actor, Permission::EnrolStudents, "enrol students");
    if (!courses_.findCourse(courseId)) throw NotFound("Course not found.");
    const auto user = users_.findById(studentId);
    if (!user || !user->active() || user->role() != Role::Student) {
        throw ValidationError("The selected user is not an active student.");
    }
    if (courses_.isEnrolled(studentId, courseId)) throw RuleViolation("Student is already enrolled.");
    auto scope = uow_.begin();
    courses_.enrol(studentId, courseId, clock_.now());
    audit_.record(actor.id(), "ENROL_STUDENT", "course", courseId, std::nullopt, std::to_string(studentId));
    scope->commit();
}

}  // namespace lms::services
