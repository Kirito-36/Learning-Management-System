#include "domain/User.hpp"

#include <utility>

namespace lms::domain {

User::User(int id, std::string name, std::string username, std::string passwordHash, bool active)
    : id_(id),
      name_(std::move(name)),
      username_(std::move(username)),
      passwordHash_(std::move(passwordHash)),
      active_(active) {}

bool User::authenticate(const IPasswordHasher& hasher, const std::string& password) const {
    if (!active_) return false;  // deactivated accounts can never log in
    return hasher.verify(passwordHash_, password);
}

const std::set<Permission>& Administrator::permissions() const noexcept {
    static const std::set<Permission> p{Permission::ManageUsers, Permission::ManageCourses,
                                        Permission::EnrolStudents, Permission::ViewAuditLog};
    return p;
}

Lecturer::Lecturer(int id, std::string name, std::string username, std::string passwordHash,
                   bool active, std::string staffNumber, int departmentId)
    : User(id, std::move(name), std::move(username), std::move(passwordHash), active),
      staffNumber_(std::move(staffNumber)),
      departmentId_(departmentId) {}

const std::set<Permission>& Lecturer::permissions() const noexcept {
    static const std::set<Permission> p{Permission::CreateAssignment, Permission::GrantExtension,
                                        Permission::MarkSubmission};
    return p;
}

const std::set<Permission>& HeadOfDepartment::permissions() const noexcept {
    static const std::set<Permission> p{Permission::CreateAssignment, Permission::GrantExtension,
                                        Permission::MarkSubmission,   Permission::ModerateMarks,
                                        Permission::ReleaseMarks,     Permission::ViewAuditLog};
    return p;
}

Student::Student(int id, std::string name, std::string username, std::string passwordHash,
                 bool active, std::string studentNumber)
    : User(id, std::move(name), std::move(username), std::move(passwordHash), active),
      studentNumber_(std::move(studentNumber)) {}

const std::set<Permission>& Student::permissions() const noexcept {
    static const std::set<Permission> p{Permission::SubmitWork, Permission::ViewOwnMarks};
    return p;
}

}  // namespace lms::domain
