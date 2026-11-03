#pragma once
#include <optional>
#include <string>

#include "data/ICourseRepository.hpp"
#include "data/IUnitOfWork.hpp"
#include "data/IUserRepository.hpp"
#include "domain/IClock.hpp"
#include "domain/IPasswordHasher.hpp"
#include "services/AuditService.hpp"

namespace lms::services {

struct NewUser {
    domain::Role role = domain::Role::Student;
    std::string username;
    std::string name;
    std::string password;
    std::string studentNumber;  // students only
    std::string staffNumber;    // lecturers and heads of department only
    int departmentId = 0;       // lecturers and heads of department only
};

// Administrator use cases: accounts, departments, courses, lecturer allocation, enrolment.
class AdminService {
public:
    AdminService(data::IUserRepository& users, data::ICourseRepository& courses, const domain::IPasswordHasher& hasher,
                 AuditService& audit, data::IUnitOfWork& uow, const domain::IClock& clock);

    // First-run bootstrap: only permitted while no users exist.
    int createInitialAdministrator(const std::string& username, const std::string& name, const std::string& password);

    int createUser(const domain::User& actor, const NewUser& details);
    void setUserActive(const domain::User& actor, int userId, bool active);
    int createDepartment(const domain::User& actor, const std::string& name);
    int createCourse(const domain::User& actor, const domain::Course& course);
    void assignLecturer(const domain::User& actor, int courseId, int lecturerId);
    void enrolStudent(const domain::User& actor, int studentId, int courseId);

private:
    data::IUserRepository& users_;
    data::ICourseRepository& courses_;
    const domain::IPasswordHasher& hasher_;
    AuditService& audit_;
    data::IUnitOfWork& uow_;
    const domain::IClock& clock_;
};

}  // namespace lms::services
