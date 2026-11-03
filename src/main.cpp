// Console entry point and role-based LMS UI.
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "data/Database.hpp"
#include "data/SqliteRepositories.hpp"
#include "domain/DailyLatePenaltyStrategy.hpp"
#include "domain/Errors.hpp"
#include "domain/TimeUtil.hpp"
#include "services/AdminService.hpp"
#include "services/AssignmentService.hpp"
#include "services/AuthService.hpp"
#include "services/AuditService.hpp"
#include "services/DeadlinePolicy.hpp"
#include "services/MarkingService.hpp"
#include "services/ModerationService.hpp"
#include "services/ResultsService.hpp"
#include "services/SubmissionService.hpp"
#include "security/SodiumPasswordHasher.hpp"

namespace {
using namespace lms;
using domain::Role;

struct EndOfInput {};

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) throw EndOfInput{};
    return value;
}

int readInt(const std::string& label, int minimum, int maximum) {
    for (;;) {
        const std::string text = prompt(label);
        std::istringstream input(text);
        int value = 0;
        char extra = 0;
        if ((input >> value) && !(input >> extra) && value >= minimum && value <= maximum) return value;
        std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

double readDouble(const std::string& label, double minimum, double maximum) {
    for (;;) {
        const std::string text = prompt(label);
        char* end = nullptr;
        errno = 0;
        const double value = std::strtod(text.c_str(), &end);
        if (!text.empty() && end == text.c_str() + text.size() && errno != ERANGE &&
            value >= minimum && value <= maximum) return value;
        std::cout << "Enter a number from " << minimum << " to " << maximum << ".\n";
    }
}

domain::Timestamp readDateTime(const std::string& label) {
    for (;;) {
        const auto text = prompt(label + " (YYYY-MM-DD HH:MM): ");
        const auto value = domain::parseDateTime(text);
        if (value) return *value;
        std::cout << "That date/time is invalid. Use YYYY-MM-DD HH:MM, for example 2026-09-15 17:00.\n";
    }
}

void line() { std::cout << "------------------------------------------------------------\n"; }
void heading(const std::string& title) {
    std::cout << "\n";
    line();
    std::cout << title << '\n';
    line();
}

const char* roleName(Role role) {
    switch (role) {
        case Role::Administrator: return "Administrator";
        case Role::Lecturer: return "Lecturer";
        case Role::HeadOfDepartment: return "Head of Department";
        case Role::Student: return "Student";
    }
    return "User";
}

void showCourses(const std::vector<domain::Course>& courses) {
    if (courses.empty()) { std::cout << "No courses found.\n"; return; }
    std::cout << std::left << std::setw(6) << "ID" << std::setw(14) << "Code" << std::setw(32)
              << "Title" << "Year\n";
    for (const auto& c : courses)
        std::cout << std::left << std::setw(6) << c.id << std::setw(14) << c.code
                  << std::setw(32) << c.title << c.academicYear << '\n';
}

void showAssignments(const std::vector<domain::Assignment>& assignments) {
    if (assignments.empty()) { std::cout << "No assignments found.\n"; return; }
    for (const auto& a : assignments) {
        std::cout << "#" << a.id << "  " << a.title << " (course " << a.courseId << ")\n"
                  << "    Due: " << domain::formatDateTime(a.dueAt) << " | Max mark: " << a.maxMark
                  << " | Attempts: " << a.maxAttempts << "\n";
        if (!a.description.empty()) std::cout << "    " << a.description << '\n';
    }
}

void showUsers(const std::vector<std::unique_ptr<domain::User>>& users) {
    if (users.empty()) { std::cout << "No users found.\n"; return; }
    std::cout << std::left << std::setw(6) << "ID" << std::setw(24) << "Username" << std::setw(28)
              << "Name" << std::setw(23) << "Role" << "Status\n";
    for (const auto& u : users)
        std::cout << std::left << std::setw(6) << u->id() << std::setw(24) << u->username()
                  << std::setw(28) << u->name() << std::setw(23) << roleName(u->role())
                  << (u->active() ? "Active" : "Inactive") << '\n';
}

struct App {
    data::SqliteUserRepository& users;
    data::SqliteCourseRepository& courses;
    data::SqliteAssignmentRepository& assignments;
    data::SqliteSubmissionRepository& submissions;
    data::SqliteMarkRepository& marks;
    services::AdminService& admin;
    services::AssignmentService& assignmentService;
    services::SubmissionService& submissionService;
    services::MarkingService& marking;
    services::ModerationService& moderation;
    services::ResultsService& results;
    services::AuditService& audit;

    template <typename Action> static void safely(Action action) {
        try { action(); }
        catch (const domain::LmsError& e) { std::cout << "\nUnable to complete that action: " << e.what() << '\n'; }
        catch (const std::exception& e) { std::cout << "\nAn unexpected error occurred: " << e.what() << '\n'; }
    }

    void adminMenu(const domain::User& actor) {
        for (;;) {
            heading("Administrator menu");
            std::cout << "1. List users\n2. Create user\n3. Activate/deactivate user\n"
                         "4. List departments\n5. Create department\n6. List courses\n"
                         "7. Create course\n8. Assign lecturer to course\n9. Enrol student\n"
                         "10. View audit history\n0. Log out\n";
            const int choice = readInt("Choose: ", 0, 10);
            if (choice == 0) return;
            safely([&] {
                if (choice == 1) showUsers(users.listAll());
                else if (choice == 2) createUser(actor);
                else if (choice == 3) setUserActive(actor);
                else if (choice == 4) listDepartments();
                else if (choice == 5) {
                    const int id = admin.createDepartment(actor, prompt("Department name: "));
                    std::cout << "Department created (ID " << id << ").\n";
                } else if (choice == 6) showCourses(courses.listCourses());
                else if (choice == 7) createCourse(actor);
                else if (choice == 8) assignLecturer(actor);
                else if (choice == 9) enrolStudent(actor);
                else viewAudit(actor);
            });
        }
    }

    void createUser(const domain::User& actor) {
        heading("Create user");
        std::cout << "1. Student  2. Lecturer  3. Head of Department  4. Administrator\n";
        const int choice = readInt("Role: ", 1, 4);
        services::NewUser d;
        d.role = choice == 1 ? Role::Student : choice == 2 ? Role::Lecturer :
                 choice == 3 ? Role::HeadOfDepartment : Role::Administrator;
        d.username = prompt("Username: ");
        d.name = prompt("Full name: ");
        d.password = prompt("Initial password (at least 8 characters): ");
        if (d.role == Role::Student) d.studentNumber = prompt("Student number: ");
        if (d.role == Role::Lecturer || d.role == Role::HeadOfDepartment) {
            d.staffNumber = prompt("Staff number: ");
            listDepartments();
            d.departmentId = readInt("Department ID: ", 1, 2147483647);
        }
        std::cout << "User created (ID " << admin.createUser(actor, d) << ").\n";
    }

    void setUserActive(const domain::User& actor) {
        showUsers(users.listAll());
        const int id = readInt("User ID: ", 1, 2147483647);
        std::cout << "1. Activate  2. Deactivate\n";
        admin.setUserActive(actor, id, readInt("Choose: ", 1, 2) == 1);
        std::cout << "User status updated.\n";
    }

    void listDepartments() {
        const auto list = courses.listDepartments();
        if (list.empty()) { std::cout << "No departments found.\n"; return; }
        for (const auto& d : list)
            std::cout << "#" << d.id << "  " << d.name << " | Head: "
                      << (d.hodUserId ? std::to_string(*d.hodUserId) : "not assigned") << '\n';
    }

    void createCourse(const domain::User& actor) {
        listDepartments();
        domain::Course c;
        c.code = prompt("Course code: ");
        c.title = prompt("Course title: ");
        c.academicYear = readInt("Academic year: ", 2000, 2100);
        c.departmentId = readInt("Department ID: ", 1, 2147483647);
        std::cout << "Course created (ID " << admin.createCourse(actor, c) << ").\n";
    }

    void assignLecturer(const domain::User& actor) {
        showCourses(courses.listCourses()); showUsers(users.listAll());
        const int courseId = readInt("Course ID: ", 1, 2147483647);
        const int lecturerId = readInt("Lecturer user ID: ", 1, 2147483647);
        admin.assignLecturer(actor, courseId, lecturerId);
        std::cout << "Lecturer assigned.\n";
    }

    void enrolStudent(const domain::User& actor) {
        showCourses(courses.listCourses()); showUsers(users.listAll());
        const int studentId = readInt("Student user ID: ", 1, 2147483647);
        const int courseId = readInt("Course ID: ", 1, 2147483647);
        admin.enrolStudent(actor, studentId, courseId);
        std::cout << "Student enrolled.\n";
    }

    void lecturerMenu(const domain::User& actor) {
        for (;;) {
            heading(std::string(roleName(actor.role())) + " menu");
            std::cout << "1. My courses\n2. My assignments\n3. Create assignment\n"
                         "4. Grant student extension\n5. View submissions for assignment\n"
                         "6. Mark submission\n";
            if (actor.role() == Role::HeadOfDepartment)
                std::cout << "7. Review pending marks\n8. Release approved marks\n";
            std::cout << "9. View audit history\n0. Log out\n";
            const int choice = readInt("Choose: ", 0, 9);
            if (choice == 0) return;
            safely([&] {
                if (choice == 1) showCourses(courses.coursesForLecturer(actor.id()));
                else if (choice == 2) showAssignments(assignmentService.listFor(actor));
                else if (choice == 3) createAssignment(actor);
                else if (choice == 4) grantExtension(actor);
                else if (choice == 5) viewSubmissions(actor);
                else if (choice == 6) markSubmission(actor);
                else if (choice == 7 && actor.role() == Role::HeadOfDepartment) reviewMarks(actor);
                else if (choice == 8 && actor.role() == Role::HeadOfDepartment) releaseMarks(actor);
                else if (choice == 9) viewAudit(actor);
                else std::cout << "Choose one of the options shown.\n";
            });
        }
    }

    void createAssignment(const domain::User& actor) {
        showCourses(courses.coursesForLecturer(actor.id()));
        const int courseId = readInt("Course ID: ", 1, 2147483647);
        const std::string title = prompt("Title: ");
        const std::string description = prompt("Description: ");
        const auto due = readDateTime("Due date/time");
        const double maximum = readDouble("Maximum mark: ", 0.01, 1000.0);
        const int attempts = readInt("Maximum attempts (1-10): ", 1, 10);
        const int id = assignmentService.createAssignment(actor, courseId, title, description, due, maximum, attempts);
        std::cout << "Assignment created (ID " << id << ").\n";
    }

    void grantExtension(const domain::User& actor) {
        const auto items = assignmentService.listFor(actor);
        showAssignments(items);
        const int assignmentId = readInt("Assignment ID: ", 1, 2147483647);
        showUsers(users.listAll());
        const int studentId = readInt("Student user ID: ", 1, 2147483647);
        const auto due = readDateTime("New due date/time");
        const std::string reason = prompt("Reason: ");
        const int id = assignmentService.grantExtension(actor, assignmentId, studentId, due, reason);
        std::cout << "Extension granted (ID " << id << ").\n";
    }

    void viewSubmissions(const domain::User& actor) {
        showAssignments(assignmentService.listFor(actor));
        const int assignmentId = readInt("Assignment ID: ", 1, 2147483647);
        const auto list = marking.submissionsToMark(actor, assignmentId);
        if (list.empty()) { std::cout << "No submissions found.\n"; return; }
        for (const auto& s : list)
            std::cout << "Submission #" << s.id() << " | student " << s.studentId() << " | attempt "
                      << s.attemptNo() << " | " << domain::toString(s.status()) << " | submitted "
                      << domain::formatDateTime(s.submittedAt()) << (s.isLate() ? " | late" : "")
                      << " | file: " << s.filePath() << '\n';
    }

    void markSubmission(const domain::User& actor) {
        showAssignments(assignmentService.listFor(actor));
        const int assignmentId = readInt("Assignment ID: ", 1, 2147483647);
        const auto list = marking.submissionsToMark(actor, assignmentId);
        if (list.empty()) { std::cout << "No submissions found.\n"; return; }
        for (const auto& s : list)
            std::cout << "Submission #" << s.id() << " | student " << s.studentId() << " | attempt "
                      << s.attemptNo() << " | " << domain::toString(s.status()) << '\n';
        const int id = readInt("Submission ID to mark: ", 1, 2147483647);
        const double score = readDouble("Raw mark: ", 0.0, 1000000.0);
        const std::string feedback = prompt("Feedback (required): ");
        const std::string reason = prompt("Reason for changing an existing mark (otherwise leave blank): ");
        const auto mark = marking.saveMark(actor, id, score, feedback, reason);
        std::cout << "Mark saved for moderation (mark ID " << mark.id << ", final mark " << mark.finalMark << ").\n";
    }

    void reviewMarks(const domain::User& actor) {
        const auto pending = moderation.pendingMarks(actor);
        if (pending.empty()) { std::cout << "There are no marks awaiting moderation.\n"; return; }
        for (const auto& m : pending)
            std::cout << "Mark #" << m.id << " | submission " << m.submissionId << " | raw " << m.rawMark
                      << " | final " << m.finalMark << " | feedback: " << m.feedback << '\n';
        const int id = readInt("Mark ID to review: ", 1, 2147483647);
        std::cout << "1. Approve  2. Request changes\n";
        const int decision = readInt("Decision: ", 1, 2);
        const std::string comment = prompt(decision == 2 ? "Required comment: " : "Comment (optional): ");
        moderation.review(actor, id, decision == 1 ? domain::ModerationDecision::Approved
                                                  : domain::ModerationDecision::ChangesRequested, comment);
        std::cout << (decision == 1 ? "Mark approved.\n" : "Changes requested.\n");
    }

    void releaseMarks(const domain::User& actor) {
        const auto approved = moderation.approvedMarks(actor);
        if (approved.empty()) { std::cout << "There are no approved marks to release.\n"; return; }
        for (const auto& m : approved)
            std::cout << "Mark #" << m.id << " | submission " << m.submissionId << " | final " << m.finalMark << '\n';
        moderation.release(actor, readInt("Mark ID to release: ", 1, 2147483647));
        std::cout << "Mark released to the student.\n";
    }

    void studentMenu(const domain::User& actor) {
        for (;;) {
            heading("Student menu");
            std::cout << "1. My courses\n2. View assignments\n3. Submit/resubmit work\n"
                         "4. My submissions\n5. Released results\n0. Log out\n";
            const int choice = readInt("Choose: ", 0, 5);
            if (choice == 0) return;
            safely([&] {
                if (choice == 1) showCourses(courses.coursesForStudent(actor.id()));
                else if (choice == 2) showAssignments(assignmentService.listFor(actor));
                else if (choice == 3) submitWork(actor);
                else if (choice == 4) mySubmissions(actor);
                else showResults(actor);
            });
        }
    }

    void submitWork(const domain::User& actor) {
        showAssignments(assignmentService.listFor(actor));
        const int assignmentId = readInt("Assignment ID: ", 1, 2147483647);
        const std::string path = prompt("Submission file path: ");
        const auto result = submissionService.submit(actor, assignmentId, path);
        std::cout << "Submitted successfully (submission ID " << result.submission.id() << ", attempt "
                  << result.submission.attemptNo() << ").";
        if (result.lateness.isLate) std::cout << " This submission is late and may receive a penalty.";
        std::cout << '\n';
    }

    void mySubmissions(const domain::User& actor) {
        const auto list = submissionService.mySubmissions(actor);
        if (list.empty()) { std::cout << "You have no submissions yet.\n"; return; }
        for (const auto& s : list)
            std::cout << "Submission #" << s.id() << " | assignment " << s.assignmentId() << " | attempt "
                      << s.attemptNo() << " | " << domain::toString(s.status()) << " | "
                      << domain::formatDateTime(s.submittedAt()) << (s.isLate() ? " | late" : "")
                      << " | " << s.filePath() << '\n';
    }

    void showResults(const domain::User& actor) {
        const auto list = results.releasedFor(actor);
        if (list.empty()) { std::cout << "No results have been released yet.\n"; return; }
        for (const auto& [s, m] : list)
            std::cout << "Assignment " << s.assignmentId() << " | attempt " << s.attemptNo()
                      << " | mark " << m.finalMark << " (raw " << m.rawMark << ")\n"
                      << "    Feedback: " << m.feedback << '\n';
    }

    void viewAudit(const domain::User& actor) {
        const std::string type = prompt("Entity type (user, course, assignment, mark, etc.): ");
        const int id = readInt("Entity ID: ", 1, 2147483647);
        const auto entries = audit.history(actor, type, id);
        if (entries.empty()) { std::cout << "No audit entries for that entity.\n"; return; }
        for (const auto& e : entries)
            std::cout << "#" << e.id << " " << domain::formatDateTime(e.occurredAt) << " | " << e.action
                      << " | actor " << (e.actorUserId ? std::to_string(*e.actorUserId) : "system")
                      << " | " << e.oldValue.value_or("-") << " -> " << e.newValue.value_or("-")
                      << (e.reason ? " | " + *e.reason : "") << '\n';
    }
};

void run(const std::string& dbPath) {
    data::Database db(dbPath);
    db.initialiseSchema();
    data::SqliteUserRepository users(db);
    data::SqliteCourseRepository courses(db);
    data::SqliteAssignmentRepository assignments(db);
    data::SqliteExtensionRepository extensions(db);
    data::SqliteSubmissionRepository submissions(db);
    data::SqliteMarkRepository marks(db);
    data::SqliteAuditRepository auditRepo(db);
    data::SqliteUnitOfWork uow(db);
    domain::SystemClock clock;
    security::SodiumPasswordHasher hasher;
    services::AuditService audit(auditRepo, clock);
    services::DeadlinePolicy deadlines(extensions);
    domain::DailyLatePenaltyStrategy penalty;
    services::AuthService auth(users, hasher);
    services::AdminService admin(users, courses, hasher, audit, uow, clock);
    services::AssignmentService assignmentService(courses, assignments, extensions, deadlines, audit, uow, clock);
    services::SubmissionService submissionService(courses, assignments, submissions, deadlines, penalty, audit, uow, clock);
    services::MarkingService marking(courses, assignments, submissions, marks, deadlines, penalty, audit, uow, clock);
    services::ModerationService moderation(courses, assignments, submissions, marks, audit, uow, clock);
    services::ResultsService results(submissions, marks);
    App app{users, courses, assignments, submissions, marks, admin, assignmentService, submissionService,
            marking, moderation, results, audit};

    std::cout << "\nWelcome to the Learning Management System\n";
    if (users.countAll() == 0) {
        heading("First-time setup");
        std::cout << "No administrator exists. Create the initial administrator account.\n";
        const std::string username = prompt("Username: ");
        const std::string name = prompt("Full name: ");
        const std::string password = prompt("Password (at least 8 characters): ");
        const int id = admin.createInitialAdministrator(username, name, password);
        std::cout << "Administrator created (ID " << id << "). Please sign in.\n";
    }

    for (;;) {
        heading("Sign in");
        const std::string username = prompt("Username (or Ctrl+D to exit): ");
        const std::string password = prompt("Password: ");
        try {
            auto user = auth.login(username, password);
            std::cout << "Welcome, " << user->name() << " (" << roleName(user->role()) << ").\n";
            if (user->role() == Role::Administrator) app.adminMenu(*user);
            else if (user->role() == Role::Student) app.studentMenu(*user);
            else app.lecturerMenu(*user);
        } catch (const domain::AuthenticationError& e) {
            std::cout << e.what() << '\n';
        }
    }
}
}  // namespace

int main(int argc, char** argv) {
    const std::string dbPath = argc > 1 ? argv[1] : "lms.db";
    try {
        run(dbPath);
    } catch (const EndOfInput&) {
        std::cout << "\nGoodbye.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
