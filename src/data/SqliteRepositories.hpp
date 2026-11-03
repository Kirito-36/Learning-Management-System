#pragma once
// SQLite implementations of the repository interfaces. All SQL in the system lives in
// SqliteRepositories.cpp and uses bound parameters only.
#include "data/Database.hpp"
#include "data/IAssignmentRepository.hpp"
#include "data/IAuditRepository.hpp"
#include "data/ICourseRepository.hpp"
#include "data/IMarkRepository.hpp"
#include "data/ISubmissionRepository.hpp"
#include "data/IUnitOfWork.hpp"
#include "data/IUserRepository.hpp"

namespace lms::data {

class SqliteUserRepository final : public IUserRepository {
public:
    explicit SqliteUserRepository(Database& db) : db_(db) {}
    std::unique_ptr<domain::User> findById(int id) const override;
    std::unique_ptr<domain::User> findByUsername(const std::string& username) const override;
    int add(const domain::User& user) override;
    void update(const domain::User& user) override;
    std::vector<std::unique_ptr<domain::User>> listAll() const override;
    int countAll() const override;

private:
    Database& db_;
};

class SqliteCourseRepository final : public ICourseRepository {
public:
    explicit SqliteCourseRepository(Database& db) : db_(db) {}
    int addDepartment(const domain::Department& department) override;
    std::optional<domain::Department> findDepartment(int id) const override;
    std::vector<domain::Department> listDepartments() const override;
    void setHeadOfDepartment(int departmentId, int userId) override;
    int addCourse(const domain::Course& course) override;
    std::optional<domain::Course> findCourse(int id) const override;
    std::vector<domain::Course> listCourses() const override;
    std::vector<domain::Course> coursesForLecturer(int lecturerId) const override;
    std::vector<domain::Course> coursesForStudent(int studentId) const override;
    void assignLecturer(int courseId, int lecturerId) override;
    bool isLecturerOfCourse(int courseId, int lecturerId) const override;
    int enrol(int studentId, int courseId, domain::Timestamp at) override;
    bool isEnrolled(int studentId, int courseId) const override;

private:
    Database& db_;
};

class SqliteAssignmentRepository final : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db) : db_(db) {}
    int add(const domain::Assignment& assignment) override;
    std::optional<domain::Assignment> findById(int id) const override;
    std::vector<domain::Assignment> listByCourse(int courseId) const override;

private:
    Database& db_;
};

class SqliteExtensionRepository final : public IExtensionRepository {
public:
    explicit SqliteExtensionRepository(Database& db) : db_(db) {}
    int add(const domain::Extension& extension) override;
    std::optional<domain::Timestamp> latestApprovedDueAt(int assignmentId, int studentId) const override;
    std::vector<domain::Extension> listForAssignment(int assignmentId) const override;

private:
    Database& db_;
};

class SqliteSubmissionRepository final : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db) : db_(db) {}
    int save(const domain::Submission& submission) override;
    std::optional<domain::Submission> findById(int id) const override;
    std::vector<domain::Submission> findByAssignmentAndStudent(int assignmentId, int studentId) const override;
    int countAttempts(int assignmentId, int studentId) const override;
    std::vector<domain::Submission> listByAssignment(int assignmentId) const override;
    std::vector<domain::Submission> listByStudent(int studentId) const override;

private:
    Database& db_;
};

class SqliteMarkRepository final : public IMarkRepository {
public:
    explicit SqliteMarkRepository(Database& db) : db_(db) {}
    int add(const domain::Mark& mark) override;
    void update(const domain::Mark& mark) override;
    std::optional<domain::Mark> findById(int id) const override;
    std::optional<domain::Mark> findBySubmission(int submissionId) const override;
    std::vector<domain::Mark> listByStatusForDepartment(domain::MarkStatus status, int departmentId) const override;
    int addModeration(const domain::ModerationRecord& record) override;
    std::vector<domain::ModerationRecord> moderationsFor(int markId) const override;

private:
    Database& db_;
};

class SqliteAuditRepository final : public IAuditRepository {
public:
    explicit SqliteAuditRepository(Database& db) : db_(db) {}
    void append(const domain::AuditEntry& entry) override;
    std::vector<domain::AuditEntry> findByEntity(const std::string& entityType, int entityId) const override;

private:
    Database& db_;
};

class SqliteUnitOfWork final : public IUnitOfWork {
public:
    explicit SqliteUnitOfWork(Database& db) : db_(db) {}
    std::unique_ptr<Scope> begin() override;

private:
    Database& db_;
};

}  // namespace lms::data
