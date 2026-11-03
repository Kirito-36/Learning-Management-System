// Access control, authentication, injection resistance, persistence and non-functional checks.
#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>

#include "TestWorld.hpp"
#include "domain/Errors.hpp"
#include "domain/TimeUtil.hpp"
#include "security/SodiumPasswordHasher.hpp"

using namespace lms;
using namespace lms::domain;
using lmstest::World;

TEST(AccessControl, RolesCannotPerformOtherRolesActions) {
    World w;
    w.clock.t = w.due - 10;
    const auto sub = w.submissionSvc.submit(*w.student, w.assignmentId, "w.pdf").submission;
    EXPECT_THROW(w.markingSvc.saveMark(*w.student, sub.id(), 50.0, "f", ""), AccessDenied);
    EXPECT_THROW(w.markingSvc.saveMark(*w.adminUser, sub.id(), 50.0, "f", ""), AccessDenied);
    EXPECT_THROW(w.submissionSvc.submit(*w.lecturer, w.assignmentId, "x.pdf"), AccessDenied);
    EXPECT_THROW(w.admin.createDepartment(*w.lecturer, "Hacking"), AccessDenied);
    EXPECT_THROW(w.assignmentSvc.createAssignment(*w.student, w.courseId, "t", "", w.due, 10.0, 1), AccessDenied);

    const Mark m = w.markingSvc.saveMark(*w.lecturer, sub.id(), 50.0, "f", "");
    EXPECT_THROW(w.moderationSvc.review(*w.lecturer, m.id, ModerationDecision::Approved, ""), AccessDenied);
    EXPECT_THROW(w.moderationSvc.release(*w.lecturer, m.id), AccessDenied);
    EXPECT_THROW(w.results.releasedFor(*w.lecturer), AccessDenied);
}

TEST(AccessControl, LecturerOnlyActsOnCoursesTheyTeach) {
    World w;
    w.clock.t = w.due - 10;
    const auto sub = w.submissionSvc.submit(*w.student, w.assignmentId, "w.pdf").submission;
    EXPECT_THROW(w.markingSvc.saveMark(*w.otherLecturer, sub.id(), 50.0, "f", ""), AccessDenied);
    EXPECT_THROW(w.assignmentSvc.grantExtension(*w.otherLecturer, w.assignmentId, w.student->id(), w.due + 99, "r"),
                 AccessDenied);
    EXPECT_THROW(w.markingSvc.submissionsToMark(*w.otherLecturer, w.assignmentId), AccessDenied);
}

TEST(AccessControl, HeadOfDepartmentOnlyModeratesOwnDepartment) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due - 10, 50.0);
    const int dept2 = w.admin.createDepartment(*w.adminUser, "Mathematics");
    const auto hod2 = w.users.findById(w.addUser(Role::HeadOfDepartment, "hod2", "H2", dept2));
    EXPECT_THROW(w.moderationSvc.review(*hod2, m.id, ModerationDecision::Approved, ""), AccessDenied);
    EXPECT_EQ(static_cast<int>(w.moderationSvc.pendingMarks(*hod2).size()), 0);
    EXPECT_EQ(static_cast<int>(w.moderationSvc.pendingMarks(*w.hod).size()), 1);
}

TEST(AccessControl, UnenrolledStudentCannotSubmit) {
    World w;
    const auto outsider = w.users.findById(w.addUser(Role::Student, "outsider", "S9", 0));
    EXPECT_THROW(w.submissionSvc.submit(*outsider, w.assignmentId, "w.pdf"), AccessDenied);
}

TEST(AccessControl, ReleaseRequiresApprovalFirst) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due - 10, 50.0);
    EXPECT_THROW(w.moderationSvc.release(*w.hod, m.id), RuleViolation);
}

TEST(AccessControl, StudentsSeeOnlyTheirOwnReleasedMarks) {
    World w;
    const Mark m = w.submitAndMark(*w.student, w.due - 10, 50.0);
    w.moderationSvc.review(*w.hod, m.id, ModerationDecision::Approved, "");
    w.moderationSvc.release(*w.hod, m.id);
    EXPECT_EQ(static_cast<int>(w.results.releasedFor(*w.student).size()), 1);
    EXPECT_EQ(static_cast<int>(w.results.releasedFor(*w.student2).size()), 0);
}

TEST(Authentication, LoginSucceedsAndFailuresShareOneMessage) {
    World w;
    EXPECT_EQ(w.auth.login("student", "password1")->id(), w.student->id());
    std::string wrongPassword, unknownUser;
    try { w.auth.login("student", "nope-nope"); } catch (const AuthenticationError& e) { wrongPassword = e.what(); }
    try { w.auth.login("ghost", "password1"); } catch (const AuthenticationError& e) { unknownUser = e.what(); }
    EXPECT_FALSE(wrongPassword.empty());
    EXPECT_EQ(wrongPassword, unknownUser);
}

TEST(Authentication, DeactivatedAccountsCannotLogIn) {
    World w;
    w.admin.setUserActive(*w.adminUser, w.student->id(), false);
    EXPECT_THROW(w.auth.login("student", "password1"), AuthenticationError);
    w.admin.setUserActive(*w.adminUser, w.student->id(), true);
    EXPECT_NO_THROW(w.auth.login("student", "password1"));
    EXPECT_THROW(w.admin.setUserActive(*w.adminUser, w.adminUser->id(), false), RuleViolation);
}

TEST(Authentication, PasswordsAreNeverStoredInPlainText) {
    World w;
    const auto stored = w.users.findByUsername("student");
    EXPECT_TRUE(stored->passwordHash() != "password1");
}

TEST(Authentication, SodiumHasherProducesSaltedArgon2idHashes) {
    security::SodiumPasswordHasher h;
    const std::string a = h.hash("correct horse battery");
    const std::string b = h.hash("correct horse battery");
    EXPECT_TRUE(a != b);                                   // random salt
    EXPECT_TRUE(a.find("correct") == std::string::npos);   // no plain text
    EXPECT_TRUE(a.rfind("$argon2id$", 0) == 0);
    EXPECT_TRUE(h.verify(a, "correct horse battery"));
    EXPECT_FALSE(h.verify(a, "correct horse batterY"));
    EXPECT_FALSE(h.verify("not-a-hash", "x"));
}

TEST(AdminRules, AccountValidation) {
    World w;
    services::NewUser n;
    n.role = Role::Student;
    n.username = "newbie";
    n.name = "New";
    n.password = "short";
    n.studentNumber = "S50";
    EXPECT_THROW(w.admin.createUser(*w.adminUser, n), ValidationError);   // weak password
    n.password = "longenough1";
    n.username = "student";
    EXPECT_THROW(w.admin.createUser(*w.adminUser, n), ValidationError);   // duplicate username
    n.username = "bad name!";
    EXPECT_THROW(w.admin.createUser(*w.adminUser, n), ValidationError);   // illegal characters
    n.username = "newbie";
    n.studentNumber = "";
    EXPECT_THROW(w.admin.createUser(*w.adminUser, n), ValidationError);   // student number required
    EXPECT_THROW(w.admin.createInitialAdministrator("root", "Root", "password1"), RuleViolation);
    n.role = Role::HeadOfDepartment;
    n.staffNumber = "H9";
    n.departmentId = w.departmentId;
    EXPECT_THROW(w.admin.createUser(*w.adminUser, n), RuleViolation);     // department already has a HoD
}

TEST(Injection, SqlMetacharactersAreTreatedAsData) {
    World w;
    EXPECT_TRUE(w.users.findByUsername("x' OR '1'='1") == nullptr);
    EXPECT_TRUE(w.users.findByUsername("student'; DROP TABLE users;--") == nullptr);
    EXPECT_THROW(w.auth.login("admin' --", "whatever1"), AuthenticationError);
    w.clock.t = w.due - 10;
    const auto id = w.submissionSvc.submit(*w.student, w.assignmentId, "w'; DROP TABLE marks;--.pdf").submission.id();
    const Mark m = w.markingSvc.saveMark(*w.lecturer, id, 10.0, "Robert'); DROP TABLE students;--", "");
    EXPECT_EQ(w.marks.findById(m.id)->feedback, std::string("Robert'); DROP TABLE students;--"));
    EXPECT_EQ(w.submissions.findById(id)->filePath(), std::string("w'; DROP TABLE marks;--.pdf"));
    EXPECT_EQ(w.users.countAll(), 6);  // tables intact
}

TEST(Integrity, AuditLogCannotBeEditedOrDeleted) {
    World w;
    EXPECT_THROW(w.db.execute("UPDATE audit_log SET action = 'x'"), data::DatabaseError);
    EXPECT_THROW(w.db.execute("DELETE FROM audit_log"), data::DatabaseError);
}

TEST(Integrity, DatabaseRejectsInvalidRows) {
    World w;
    EXPECT_THROW(w.db.execute("INSERT INTO users (username,name,password_hash,role) VALUES ('z','z','h','STUDENT')"),
                 data::DatabaseError);
    EXPECT_THROW(w.db.execute("INSERT INTO enrolments (student_id,course_id,enrolled_at) VALUES (999,1,0)"),
                 data::DatabaseError);  // foreign key
}

TEST(Integrity, UncommittedScopeRollsBack) {
    World w;
    {
        auto scope = w.uow.begin();
        w.courses.addDepartment(Department{0, "Temporary", std::nullopt});
        // no commit: leaves scope as if an exception had been thrown
    }
    EXPECT_EQ(static_cast<int>(w.courses.listDepartments().size()), 1);
}

TEST(Persistence, DataSurvivesRestart) {
    const std::string path = "lms_persistence_test.db";
    std::remove(path.c_str());
    int id = 0;
    {
        data::Database db(path);
        db.initialiseSchema();
        data::SqliteUserRepository users(db);
        lmstest::FakeHasher hasher;
        users.add(Administrator(0, "Admin", "admin", hasher.hash("password1"), true));
        id = users.findByUsername("admin")->id();
    }
    {
        data::Database db(path);
        db.initialiseSchema();  // idempotent on an existing database
        data::SqliteUserRepository users(db);
        const auto again = users.findByUsername("ADMIN");  // usernames are case-insensitive
        ASSERT_TRUE(again != nullptr);
        EXPECT_EQ(again->id(), id);
    }
    std::remove(path.c_str());
}

TEST(Performance, LookupWithEightHundredStudentsIsFast) {
    World w;
    {
        auto scope = w.uow.begin();
        for (int i = 0; i < 800; ++i) {
            w.users.add(Student(0, "Student", "perf" + std::to_string(i), "fake$pw", true, "P" + std::to_string(i)));
        }
        scope->commit();
    }
    EXPECT_EQ(w.users.countAll(), 806);
    const auto start = std::chrono::steady_clock::now();
    const auto found = w.users.findByUsername("perf799");
    const auto listed = w.users.listAll();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    ASSERT_TRUE(found != nullptr);
    EXPECT_EQ(static_cast<int>(listed.size()), 806);
    EXPECT_TRUE(ms < 200);
}

TEST(TimeUtil, ParsesAndFormatsRoundTrip) {
    const auto t = parseDateTime("2026-11-07 17:30");
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(formatDateTime(*t), std::string("2026-11-07 17:30"));
    EXPECT_TRUE(parseDateTime("2028-02-29 00:00").has_value());   // leap day
    EXPECT_FALSE(parseDateTime("2027-02-29 00:00").has_value());
    EXPECT_FALSE(parseDateTime("2026-02-31 10:00").has_value());
    EXPECT_FALSE(parseDateTime("2026-13-01 10:00").has_value());
    EXPECT_FALSE(parseDateTime("2026-11-07 24:00").has_value());
    EXPECT_FALSE(parseDateTime("2026-11-07").has_value());
    EXPECT_FALSE(parseDateTime("tomorrow").has_value());
    EXPECT_EQ(*parseDateTime("2026-11-08 00:00") - *parseDateTime("2026-11-07 00:00"), kSecondsPerDay);
}
