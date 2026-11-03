#pragma once
#include <set>
#include <string>

#include "domain/IPasswordHasher.hpp"
#include "domain/Types.hpp"

namespace lms::domain {

enum class Permission {
    ManageUsers,
    ManageCourses,
    EnrolStudents,
    CreateAssignment,
    GrantExtension,
    MarkSubmission,
    ModerateMarks,
    ReleaseMarks,
    SubmitWork,
    ViewOwnMarks,
    ViewAuditLog
};

// Abstract base for all account types. Role-based access control is expressed
// through polymorphism: each subclass declares exactly what its role may do.
class User {
public:
    User(int id, std::string name, std::string username, std::string passwordHash, bool active);
    virtual ~User() = default;

    virtual Role role() const noexcept = 0;

    bool can(Permission p) const { return permissions().count(p) > 0; }
    bool authenticate(const IPasswordHasher& hasher, const std::string& password) const;

    int id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    const std::string& username() const noexcept { return username_; }
    const std::string& passwordHash() const noexcept { return passwordHash_; }
    bool active() const noexcept { return active_; }

protected:
    virtual const std::set<Permission>& permissions() const noexcept = 0;

private:
    int id_;
    std::string name_;
    std::string username_;
    std::string passwordHash_;
    bool active_;
};

class Administrator final : public User {
public:
    using User::User;
    Role role() const noexcept override { return Role::Administrator; }

protected:
    const std::set<Permission>& permissions() const noexcept override;
};

class Lecturer : public User {
public:
    Lecturer(int id, std::string name, std::string username, std::string passwordHash, bool active,
             std::string staffNumber, int departmentId);
    Role role() const noexcept override { return Role::Lecturer; }
    const std::string& staffNumber() const noexcept { return staffNumber_; }
    int departmentId() const noexcept { return departmentId_; }

protected:
    const std::set<Permission>& permissions() const noexcept override;

private:
    std::string staffNumber_;
    int departmentId_;
};

// A Head of Department is a member of staff who can also moderate and release marks.
class HeadOfDepartment final : public Lecturer {
public:
    using Lecturer::Lecturer;
    Role role() const noexcept override { return Role::HeadOfDepartment; }

protected:
    const std::set<Permission>& permissions() const noexcept override;
};

class Student final : public User {
public:
    Student(int id, std::string name, std::string username, std::string passwordHash, bool active,
            std::string studentNumber);
    Role role() const noexcept override { return Role::Student; }
    const std::string& studentNumber() const noexcept { return studentNumber_; }

protected:
    const std::set<Permission>& permissions() const noexcept override;

private:
    std::string studentNumber_;
};

}  // namespace lms::domain
