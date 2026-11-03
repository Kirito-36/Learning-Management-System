#include "data/SqliteRepositories.hpp"

#include <utility>

using namespace lms::domain;

namespace lms::data {
namespace {

// ---- binding and mapping helpers ------------------------------------------------------------

void bindOpt(Statement& st, int i, const std::optional<int>& v) {
    if (v) st.bind(i, *v); else st.bindNull(i);
}
void bindOpt(Statement& st, int i, const std::optional<std::string>& v) {
    if (v) st.bind(i, *v); else st.bindNull(i);
}
void bindOpt(Statement& st, int i, const std::optional<Timestamp>& v) {
    if (v) st.bind(i, static_cast<std::int64_t>(*v)); else st.bindNull(i);
}
std::optional<int> optInt(const Statement& st, int i) {
    if (st.columnIsNull(i)) return std::nullopt;
    return static_cast<int>(st.columnInt(i));
}
std::optional<std::string> optText(const Statement& st, int i) {
    if (st.columnIsNull(i)) return std::nullopt;
    return st.columnText(i);
}
int intAt(const Statement& st, int i) { return static_cast<int>(st.columnInt(i)); }

template <typename T, typename Mapper>
std::vector<T> collect(Statement& st, Mapper map) {
    std::vector<T> out;
    while (st.step()) out.push_back(map(st));
    return out;
}

// ---- row mappers ----------------------------------------------------------------------------

constexpr const char* kUserCols =
    "user_id, username, name, password_hash, role, student_number, staff_number, department_id, active";

std::unique_ptr<User> readUser(const Statement& st) {
    const int id = intAt(st, 0);
    std::string username = st.columnText(1), name = st.columnText(2), hash = st.columnText(3);
    const Role role = roleFromString(st.columnText(4));
    const bool active = st.columnInt(8) != 0;
    switch (role) {
        case Role::Administrator:
            return std::make_unique<Administrator>(id, name, username, hash, active);
        case Role::Lecturer:
            return std::make_unique<Lecturer>(id, name, username, hash, active, st.columnText(6), intAt(st, 7));
        case Role::HeadOfDepartment:
            return std::make_unique<HeadOfDepartment>(id, name, username, hash, active, st.columnText(6),
                                                      intAt(st, 7));
        case Role::Student:
            return std::make_unique<Student>(id, name, username, hash, active, st.columnText(5));
    }
    throw DatabaseError("corrupt user row");
}

constexpr const char* kCourseCols = "c.course_id, c.course_code, c.title, c.academic_year, c.department_id";

Course readCourse(const Statement& st) {
    return Course{intAt(st, 0), st.columnText(1), st.columnText(2), intAt(st, 3), intAt(st, 4)};
}

constexpr const char* kAssignmentCols =
    "assignment_id, course_id, created_by, title, description, due_at, max_mark, max_attempts";

Assignment readAssignment(const Statement& st) {
    return Assignment{intAt(st, 0), intAt(st, 1), intAt(st, 2), st.columnText(3), st.columnText(4),
                      st.columnInt(5), st.columnDouble(6), intAt(st, 7)};
}

constexpr const char* kSubmissionCols =
    "submission_id, assignment_id, student_id, attempt_no, submitted_at, file_path, status, is_late";

Submission readSubmission(const Statement& st) {
    return Submission(intAt(st, 0), intAt(st, 1), intAt(st, 2), intAt(st, 3), st.columnInt(4),
                      st.columnText(5), submissionStatusFromString(st.columnText(6)), st.columnInt(7) != 0);
}

constexpr const char* kMarkCols =
    "m.mark_id, m.submission_id, m.marked_by, m.raw_mark, m.final_mark, m.feedback, m.status, "
    "m.marked_at, m.released_at";

Mark readMark(const Statement& st) {
    Mark m;
    m.id = intAt(st, 0);
    m.submissionId = intAt(st, 1);
    m.markedBy = intAt(st, 2);
    m.rawMark = st.columnDouble(3);
    m.finalMark = st.columnDouble(4);
    m.feedback = st.columnText(5);
    m.status = markStatusFromString(st.columnText(6));
    m.markedAt = st.columnInt(7);
    if (!st.columnIsNull(8)) m.releasedAt = st.columnInt(8);
    return m;
}

int insertedId(Database& db) { return static_cast<int>(db.lastInsertRowId()); }

}  // namespace

// ---- users ----------------------------------------------------------------------------------

std::unique_ptr<User> SqliteUserRepository::findById(int id) const {
    auto st = db_.prepare(std::string("SELECT ") + kUserCols + " FROM users WHERE user_id = ?");
    st.bind(1, id);
    return st.step() ? readUser(st) : nullptr;
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) const {
    auto st = db_.prepare(std::string("SELECT ") + kUserCols + " FROM users WHERE username = ?");
    st.bind(1, username);
    return st.step() ? readUser(st) : nullptr;
}

int SqliteUserRepository::add(const User& user) {
    const auto* lecturer = dynamic_cast<const Lecturer*>(&user);  // also matches HeadOfDepartment
    const auto* student = dynamic_cast<const Student*>(&user);
    auto st = db_.prepare(
        "INSERT INTO users (username, name, password_hash, role, student_number, staff_number, "
        "department_id, active) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    st.bind(1, user.username()).bind(2, user.name()).bind(3, user.passwordHash()).bind(4, toString(user.role()));
    bindOpt(st, 5, student ? std::optional<std::string>(student->studentNumber()) : std::nullopt);
    bindOpt(st, 6, lecturer ? std::optional<std::string>(lecturer->staffNumber()) : std::nullopt);
    bindOpt(st, 7, lecturer ? std::optional<int>(lecturer->departmentId()) : std::nullopt);
    st.bind(8, user.active() ? 1 : 0);
    st.step();
    return insertedId(db_);
}

void SqliteUserRepository::update(const User& user) {
    auto st = db_.prepare("UPDATE users SET name = ?, password_hash = ?, active = ? WHERE user_id = ?");
    st.bind(1, user.name()).bind(2, user.passwordHash()).bind(3, user.active() ? 1 : 0).bind(4, user.id());
    st.step();
}

std::vector<std::unique_ptr<User>> SqliteUserRepository::listAll() const {
    auto st = db_.prepare(std::string("SELECT ") + kUserCols + " FROM users ORDER BY role, username");
    return collect<std::unique_ptr<User>>(st, [](const Statement& s) { return readUser(s); });
}

int SqliteUserRepository::countAll() const {
    auto st = db_.prepare("SELECT COUNT(*) FROM users");
    st.step();
    return intAt(st, 0);
}

// ---- departments, courses, lecturers, enrolments ----------------------------------------------

int SqliteCourseRepository::addDepartment(const Department& d) {
    auto st = db_.prepare("INSERT INTO departments (name) VALUES (?)");
    st.bind(1, d.name);
    st.step();
    return insertedId(db_);
}

static Department readDepartment(const Statement& st) {
    return Department{intAt(st, 0), st.columnText(1), optInt(st, 2)};
}

std::optional<Department> SqliteCourseRepository::findDepartment(int id) const {
    auto st = db_.prepare("SELECT department_id, name, hod_user_id FROM departments WHERE department_id = ?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readDepartment(st);
}

std::vector<Department> SqliteCourseRepository::listDepartments() const {
    auto st = db_.prepare("SELECT department_id, name, hod_user_id FROM departments ORDER BY name");
    return collect<Department>(st, readDepartment);
}

void SqliteCourseRepository::setHeadOfDepartment(int departmentId, int userId) {
    auto st = db_.prepare("UPDATE departments SET hod_user_id = ? WHERE department_id = ?");
    st.bind(1, userId).bind(2, departmentId);
    st.step();
}

int SqliteCourseRepository::addCourse(const Course& c) {
    auto st = db_.prepare(
        "INSERT INTO courses (course_code, title, academic_year, department_id) VALUES (?, ?, ?, ?)");
    st.bind(1, c.code).bind(2, c.title).bind(3, c.academicYear).bind(4, c.departmentId);
    st.step();
    return insertedId(db_);
}

std::optional<Course> SqliteCourseRepository::findCourse(int id) const {
    auto st = db_.prepare(std::string("SELECT ") + kCourseCols + " FROM courses c WHERE c.course_id = ?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readCourse(st);
}

std::vector<Course> SqliteCourseRepository::listCourses() const {
    auto st = db_.prepare(std::string("SELECT ") + kCourseCols + " FROM courses c ORDER BY c.course_code");
    return collect<Course>(st, readCourse);
}

std::vector<Course> SqliteCourseRepository::coursesForLecturer(int lecturerId) const {
    auto st = db_.prepare(std::string("SELECT ") + kCourseCols +
                          " FROM courses c JOIN course_lecturers cl ON cl.course_id = c.course_id "
                          "WHERE cl.lecturer_id = ? ORDER BY c.course_code");
    st.bind(1, lecturerId);
    return collect<Course>(st, readCourse);
}

std::vector<Course> SqliteCourseRepository::coursesForStudent(int studentId) const {
    auto st = db_.prepare(std::string("SELECT ") + kCourseCols +
                          " FROM courses c JOIN enrolments e ON e.course_id = c.course_id "
                          "WHERE e.student_id = ? AND e.status = 'ACTIVE' ORDER BY c.course_code");
    st.bind(1, studentId);
    return collect<Course>(st, readCourse);
}

void SqliteCourseRepository::assignLecturer(int courseId, int lecturerId) {
    auto st = db_.prepare("INSERT INTO course_lecturers (course_id, lecturer_id) VALUES (?, ?)");
    st.bind(1, courseId).bind(2, lecturerId);
    st.step();
}

bool SqliteCourseRepository::isLecturerOfCourse(int courseId, int lecturerId) const {
    auto st = db_.prepare("SELECT 1 FROM course_lecturers WHERE course_id = ? AND lecturer_id = ?");
    st.bind(1, courseId).bind(2, lecturerId);
    return st.step();
}

int SqliteCourseRepository::enrol(int studentId, int courseId, Timestamp at) {
    auto st = db_.prepare("INSERT INTO enrolments (student_id, course_id, enrolled_at) VALUES (?, ?, ?)");
    st.bind(1, studentId).bind(2, courseId).bind(3, static_cast<std::int64_t>(at));
    st.step();
    return insertedId(db_);
}

bool SqliteCourseRepository::isEnrolled(int studentId, int courseId) const {
    auto st = db_.prepare("SELECT 1 FROM enrolments WHERE student_id = ? AND course_id = ? AND status = 'ACTIVE'");
    st.bind(1, studentId).bind(2, courseId);
    return st.step();
}

// ---- assignments and extensions ---------------------------------------------------------------

int SqliteAssignmentRepository::add(const Assignment& a) {
    auto st = db_.prepare(
        "INSERT INTO assignments (course_id, created_by, title, description, due_at, max_mark, max_attempts) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)");
    st.bind(1, a.courseId).bind(2, a.createdBy).bind(3, a.title).bind(4, a.description)
        .bind(5, static_cast<std::int64_t>(a.dueAt)).bind(6, a.maxMark).bind(7, a.maxAttempts);
    st.step();
    return insertedId(db_);
}

std::optional<Assignment> SqliteAssignmentRepository::findById(int id) const {
    auto st = db_.prepare(std::string("SELECT ") + kAssignmentCols + " FROM assignments WHERE assignment_id = ?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readAssignment(st);
}

std::vector<Assignment> SqliteAssignmentRepository::listByCourse(int courseId) const {
    auto st = db_.prepare(std::string("SELECT ") + kAssignmentCols +
                          " FROM assignments WHERE course_id = ? ORDER BY due_at, assignment_id");
    st.bind(1, courseId);
    return collect<Assignment>(st, readAssignment);
}

int SqliteExtensionRepository::add(const Extension& e) {
    auto st = db_.prepare(
        "INSERT INTO extensions (assignment_id, student_id, requested_due_at, reason, status, approved_by) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    st.bind(1, e.assignmentId).bind(2, e.studentId).bind(3, static_cast<std::int64_t>(e.requestedDueAt))
        .bind(4, e.reason).bind(5, toString(e.status));
    bindOpt(st, 6, e.approvedBy);
    st.step();
    return insertedId(db_);
}

std::optional<Timestamp> SqliteExtensionRepository::latestApprovedDueAt(int assignmentId, int studentId) const {
    auto st = db_.prepare("SELECT MAX(requested_due_at) FROM extensions "
                          "WHERE assignment_id = ? AND student_id = ? AND status = 'APPROVED'");
    st.bind(1, assignmentId).bind(2, studentId);
    st.step();
    if (st.columnIsNull(0)) return std::nullopt;
    return st.columnInt(0);
}

std::vector<Extension> SqliteExtensionRepository::listForAssignment(int assignmentId) const {
    auto st = db_.prepare("SELECT extension_id, assignment_id, student_id, requested_due_at, reason, status, "
                          "approved_by FROM extensions WHERE assignment_id = ? ORDER BY extension_id");
    st.bind(1, assignmentId);
    return collect<Extension>(st, [](const Statement& s) {
        return Extension{intAt(s, 0), intAt(s, 1), intAt(s, 2), s.columnInt(3), s.columnText(4),
                         extensionStatusFromString(s.columnText(5)), optInt(s, 6)};
    });
}

// ---- submissions ------------------------------------------------------------------------------

int SqliteSubmissionRepository::save(const Submission& s) {
    if (s.id() == 0) {
        auto st = db_.prepare(
            "INSERT INTO submissions (assignment_id, student_id, attempt_no, submitted_at, file_path, "
            "status, is_late) VALUES (?, ?, ?, ?, ?, ?, ?)");
        st.bind(1, s.assignmentId()).bind(2, s.studentId()).bind(3, s.attemptNo())
            .bind(4, static_cast<std::int64_t>(s.submittedAt())).bind(5, s.filePath())
            .bind(6, toString(s.status())).bind(7, s.isLate() ? 1 : 0);
        st.step();
        return insertedId(db_);
    }
    auto st = db_.prepare("UPDATE submissions SET status = ?, is_late = ? WHERE submission_id = ?");
    st.bind(1, toString(s.status())).bind(2, s.isLate() ? 1 : 0).bind(3, s.id());
    st.step();
    return s.id();
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) const {
    auto st = db_.prepare(std::string("SELECT ") + kSubmissionCols + " FROM submissions WHERE submission_id = ?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readSubmission(st);
}

std::vector<Submission> SqliteSubmissionRepository::findByAssignmentAndStudent(int assignmentId, int studentId) const {
    auto st = db_.prepare(std::string("SELECT ") + kSubmissionCols +
                          " FROM submissions WHERE assignment_id = ? AND student_id = ? ORDER BY attempt_no");
    st.bind(1, assignmentId).bind(2, studentId);
    return collect<Submission>(st, readSubmission);
}

int SqliteSubmissionRepository::countAttempts(int assignmentId, int studentId) const {
    auto st = db_.prepare("SELECT COUNT(*) FROM submissions WHERE assignment_id = ? AND student_id = ?");
    st.bind(1, assignmentId).bind(2, studentId);
    st.step();
    return intAt(st, 0);
}

std::vector<Submission> SqliteSubmissionRepository::listByAssignment(int assignmentId) const {
    auto st = db_.prepare(std::string("SELECT ") + kSubmissionCols +
                          " FROM submissions WHERE assignment_id = ? ORDER BY student_id, attempt_no");
    st.bind(1, assignmentId);
    return collect<Submission>(st, readSubmission);
}

std::vector<Submission> SqliteSubmissionRepository::listByStudent(int studentId) const {
    auto st = db_.prepare(std::string("SELECT ") + kSubmissionCols +
                          " FROM submissions WHERE student_id = ? ORDER BY assignment_id, attempt_no");
    st.bind(1, studentId);
    return collect<Submission>(st, readSubmission);
}

// ---- marks and moderation ---------------------------------------------------------------------

int SqliteMarkRepository::add(const Mark& m) {
    auto st = db_.prepare(
        "INSERT INTO marks (submission_id, marked_by, raw_mark, final_mark, feedback, status, marked_at, "
        "released_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    st.bind(1, m.submissionId).bind(2, m.markedBy).bind(3, m.rawMark).bind(4, m.finalMark).bind(5, m.feedback)
        .bind(6, toString(m.status)).bind(7, static_cast<std::int64_t>(m.markedAt));
    bindOpt(st, 8, m.releasedAt);
    st.step();
    return insertedId(db_);
}

void SqliteMarkRepository::update(const Mark& m) {
    auto st = db_.prepare("UPDATE marks SET marked_by = ?, raw_mark = ?, final_mark = ?, feedback = ?, "
                          "status = ?, marked_at = ?, released_at = ? WHERE mark_id = ?");
    st.bind(1, m.markedBy).bind(2, m.rawMark).bind(3, m.finalMark).bind(4, m.feedback)
        .bind(5, toString(m.status)).bind(6, static_cast<std::int64_t>(m.markedAt));
    bindOpt(st, 7, m.releasedAt);
    st.bind(8, m.id);
    st.step();
}

std::optional<Mark> SqliteMarkRepository::findById(int id) const {
    auto st = db_.prepare(std::string("SELECT ") + kMarkCols + " FROM marks m WHERE m.mark_id = ?");
    st.bind(1, id);
    if (!st.step()) return std::nullopt;
    return readMark(st);
}

std::optional<Mark> SqliteMarkRepository::findBySubmission(int submissionId) const {
    auto st = db_.prepare(std::string("SELECT ") + kMarkCols + " FROM marks m WHERE m.submission_id = ?");
    st.bind(1, submissionId);
    if (!st.step()) return std::nullopt;
    return readMark(st);
}

std::vector<Mark> SqliteMarkRepository::listByStatusForDepartment(MarkStatus status, int departmentId) const {
    auto st = db_.prepare(std::string("SELECT ") + kMarkCols +
                          " FROM marks m JOIN submissions s ON s.submission_id = m.submission_id "
                          "JOIN assignments a ON a.assignment_id = s.assignment_id "
                          "JOIN courses c ON c.course_id = a.course_id "
                          "WHERE m.status = ? AND c.department_id = ? ORDER BY m.marked_at, m.mark_id");
    st.bind(1, toString(status)).bind(2, departmentId);
    return collect<Mark>(st, readMark);
}

int SqliteMarkRepository::addModeration(const ModerationRecord& r) {
    auto st = db_.prepare("INSERT INTO moderation_records (mark_id, reviewed_by, decision, comment, decided_at) "
                          "VALUES (?, ?, ?, ?, ?)");
    st.bind(1, r.markId).bind(2, r.reviewedBy).bind(3, toString(r.decision)).bind(4, r.comment)
        .bind(5, static_cast<std::int64_t>(r.decidedAt));
    st.step();
    return insertedId(db_);
}

std::vector<ModerationRecord> SqliteMarkRepository::moderationsFor(int markId) const {
    auto st = db_.prepare("SELECT moderation_id, mark_id, reviewed_by, decision, comment, decided_at "
                          "FROM moderation_records WHERE mark_id = ? ORDER BY moderation_id");
    st.bind(1, markId);
    return collect<ModerationRecord>(st, [](const Statement& s) {
        return ModerationRecord{intAt(s, 0), intAt(s, 1), intAt(s, 2),
                                moderationDecisionFromString(s.columnText(3)), s.columnText(4), s.columnInt(5)};
    });
}

// ---- audit log --------------------------------------------------------------------------------

void SqliteAuditRepository::append(const AuditEntry& e) {
    auto st = db_.prepare("INSERT INTO audit_log (actor_user_id, action, entity_type, entity_id, old_value, "
                          "new_value, reason, occurred_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    bindOpt(st, 1, e.actorUserId);
    st.bind(2, e.action).bind(3, e.entityType).bind(4, e.entityId);
    bindOpt(st, 5, e.oldValue);
    bindOpt(st, 6, e.newValue);
    bindOpt(st, 7, e.reason);
    st.bind(8, static_cast<std::int64_t>(e.occurredAt));
    st.step();
}

std::vector<AuditEntry> SqliteAuditRepository::findByEntity(const std::string& entityType, int entityId) const {
    auto st = db_.prepare("SELECT audit_id, actor_user_id, action, entity_type, entity_id, old_value, new_value, "
                          "reason, occurred_at FROM audit_log WHERE entity_type = ? AND entity_id = ? "
                          "ORDER BY audit_id");
    st.bind(1, entityType).bind(2, entityId);
    return collect<AuditEntry>(st, [](const Statement& s) {
        AuditEntry e;
        e.id = intAt(s, 0);
        e.actorUserId = optInt(s, 1);
        e.action = s.columnText(2);
        e.entityType = s.columnText(3);
        e.entityId = intAt(s, 4);
        e.oldValue = optText(s, 5);
        e.newValue = optText(s, 6);
        e.reason = optText(s, 7);
        e.occurredAt = s.columnInt(8);
        return e;
    });
}

// ---- unit of work -----------------------------------------------------------------------------

namespace {
class SqliteScope final : public IUnitOfWork::Scope {
public:
    explicit SqliteScope(Database& db) : tx_(db) {}
    void commit() override { tx_.commit(); }

private:
    Transaction tx_;
};
}  // namespace

std::unique_ptr<IUnitOfWork::Scope> SqliteUnitOfWork::begin() { return std::make_unique<SqliteScope>(db_); }

}  // namespace lms::data
