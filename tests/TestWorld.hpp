#pragma once
// Shared test harness: an in-memory SQLite database wired to the real repositories and
// services, with a controllable clock and a fast fake password hasher.
#include <memory>
#include <string>

#include "data/SqliteRepositories.hpp"
#include "domain/DailyLatePenaltyStrategy.hpp"
#include "services/AdminService.hpp"
#include "services/AssignmentService.hpp"
#include "services/AuditService.hpp"
#include "services/AuthService.hpp"
#include "services/MarkingService.hpp"
#include "services/ModerationService.hpp"
#include "services/ResultsService.hpp"
#include "services/SubmissionService.hpp"

namespace lmstest {

using namespace lms;

class FakeClock final : public domain::IClock {
public:
    domain::Timestamp t = 1'800'000'000;
    domain::Timestamp now() const override { return t; }
};

class FakeHasher final : public domain::IPasswordHasher {
public:
    std::string hash(const std::string& p) const override { return "fake$" + p; }
    bool verify(const std::string& h, const std::string& p) const override { return h == "fake$" + p; }
};

struct World {
    static constexpr domain::Timestamp kDay = domain::kSecondsPerDay;

    data::Database db{":memory:"};
    FakeClock clock;
    FakeHasher hasher;
    domain::DailyLatePenaltyStrategy penalty;  // 10% per day, 3 days

    data::SqliteUserRepository users{db};
    data::SqliteCourseRepository courses{db};
    data::SqliteAssignmentRepository assignments{db};
    data::SqliteExtensionRepository extensions{db};
    data::SqliteSubmissionRepository submissions{db};
    data::SqliteMarkRepository marks{db};
    data::SqliteAuditRepository auditRepo{db};
    data::SqliteUnitOfWork uow{db};

    services::AuditService audit{auditRepo, clock};
    services::DeadlinePolicy deadlines{extensions};
    services::AuthService auth{users, hasher};
    services::AdminService admin{users, courses, hasher, audit, uow, clock};
    services::AssignmentService assignmentSvc{courses, assignments, extensions, deadlines, audit, uow, clock};
    services::SubmissionService submissionSvc{courses, assignments, submissions, deadlines, penalty, audit, uow, clock};
    services::MarkingService markingSvc{courses, assignments, submissions, marks, deadlines, penalty, audit, uow, clock};
    services::ModerationService moderationSvc{courses, assignments, submissions, marks, audit, uow, clock};
    services::ResultsService results{submissions, marks};

    std::unique_ptr<domain::User> adminUser, hod, lecturer, otherLecturer, student, student2;
    int departmentId = 0, courseId = 0, assignmentId = 0;
    domain::Timestamp due = 0;

    // Department "Computing" with HoD, two lecturers (only `lecturer` teaches the course),
    // two enrolled students, and one assignment (100 marks, 3 attempts) due in 7 days.
    World() {
        db.initialiseSchema();
        adminUser = users.findById(admin.createInitialAdministrator("admin", "Admin", "password1"));
        departmentId = admin.createDepartment(*adminUser, "Computing");
        hod = users.findById(addUser(domain::Role::HeadOfDepartment, "hod", "H1", departmentId));
        lecturer = users.findById(addUser(domain::Role::Lecturer, "lecturer", "L1", departmentId));
        otherLecturer = users.findById(addUser(domain::Role::Lecturer, "lecturer2", "L2", departmentId));
        student = users.findById(addUser(domain::Role::Student, "student", "S1", 0));
        student2 = users.findById(addUser(domain::Role::Student, "student2", "S2", 0));
        domain::Course c;
        c.code = "CS4433";
        c.title = "Software Engineering";
        c.academicYear = 2026;
        c.departmentId = departmentId;
        courseId = admin.createCourse(*adminUser, c);
        admin.assignLecturer(*adminUser, courseId, lecturer->id());
        admin.enrolStudent(*adminUser, student->id(), courseId);
        admin.enrolStudent(*adminUser, student2->id(), courseId);
        due = clock.t + 7 * kDay;
        assignmentId = assignmentSvc.createAssignment(*lecturer, courseId, "Assignment 1", "Build it", due, 100.0, 3);
    }

    int addUser(domain::Role role, const std::string& username, const std::string& number, int dept) {
        services::NewUser n;
        n.role = role;
        n.username = username;
        n.name = username + " name";
        n.password = "password1";
        n.studentNumber = role == domain::Role::Student ? number : "";
        n.staffNumber = role == domain::Role::Student ? "" : number;
        n.departmentId = dept;
        return admin.createUser(*adminUser, n);
    }

    // Student submits at `at`, lecturer marks `raw`; returns the mark.
    domain::Mark submitAndMark(const domain::User& s, domain::Timestamp at, double raw) {
        clock.t = at;
        const auto result = submissionSvc.submit(s, assignmentId, "work.pdf");
        return markingSvc.saveMark(*lecturer, result.submission.id(), raw, "Good work", "");
    }
};

}  // namespace lmstest
