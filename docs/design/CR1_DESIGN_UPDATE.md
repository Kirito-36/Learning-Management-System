# Design Update – CR1 Whole-Class Extensions (v1.1)

This addendum records the design changes for CR1. The original class and database diagrams were not present in the repository, so the affected design views are documented here.

## 1. Updated use cases

**Lecturer / Head of Department**
- Grant an individual extension.
- Grant a whole-class extension for an assignment in a course they teach.

**Student**
- View assignments with the effective due date, including approved individual or whole-class extensions.

## 2. Updated class diagram

@startuml
class AssignmentService {
  +grantExtension(lecturer, assignmentId, studentId, newDueAt, reason)
  +grantClassExtension(lecturer, assignmentId, newDueAt, reason): int
  +effectiveDueAtForStudent(assignment, studentId): Timestamp
  +listFor(user): vector<Assignment>
}

interface ICourseRepository {
  +isLecturerOfCourse(courseId, lecturerId): bool
  +studentIdsForCourse(courseId): vector<int>
}

class SqliteCourseRepository {
  +studentIdsForCourse(courseId): vector<int>
}

class DeadlinePolicy {
  +effectiveDueAt(assignment, studentId): Timestamp
}

interface IExtensionRepository {
  +add(extension): int
  +latestApprovedDueAt(assignmentId, studentId): optional<Timestamp>
}

class SqliteExtensionRepository
class Extension
class App

AssignmentService --> ICourseRepository
AssignmentService --> IExtensionRepository
AssignmentService --> DeadlinePolicy
SqliteCourseRepository ..|> ICourseRepository
SqliteExtensionRepository ..|> IExtensionRepository
AssignmentService ..> Extension : creates one per active student
App --> AssignmentService
@enduml

## 3. Whole-class extension interaction

1. Lecturer chooses the whole-class extension menu option and selects an assignment.
2. AssignmentService verifies the lecturer's permission and ownership of the assignment's course.
3. The service obtains all active student IDs through ICourseRepository::studentIdsForCourse.
4. The service checks that the new deadline is later than every student's current effective deadline.
5. If any check fails, the operation stops before writing any extension rows.
6. Otherwise, one transaction inserts an approved Extension row for each active student.
7. One GRANT_CLASS_EXTENSION audit event records the assignment, new date, student count and reason.
8. The service commits and returns the number of students extended.

## 4. Data model impact

No schema change is required. Existing relationships remain:

- courses 1 — many assignments
- courses 1 — many active enrolments
- assignments 1 — many extensions
- users 1 — many extensions (as the student)
- users 1 — many extensions (as the approving lecturer)

A whole-class extension is stored as multiple rows in the existing extensions table, one for each active student. The enrolments table is queried to determine the class membership. The audit_log table stores one summary event for the class-level action.

## 5. Deadline display

For each student assignment row, the UI calls AssignmentService::effectiveDueAtForStudent. That delegates to the existing DeadlinePolicy, which returns the later of the assignment's original due date and the student's latest approved extension. The deadline used for display is therefore the same deadline used by submission and late-penalty processing.
