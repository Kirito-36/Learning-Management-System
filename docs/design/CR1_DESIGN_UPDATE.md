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

## 3. Updated use-case diagram

@startuml
left to right direction
actor Lecturer
actor "Head of Department" as HOD
actor Student
rectangle LMS {
  usecase "Grant individual extension" as U1
  usecase "Grant whole-class extension" as U2
  usecase "View assignments and effective deadlines" as U3
}
Lecturer --> U1
Lecturer --> U2
HOD --> U1
HOD --> U2
Student --> U3
@enduml

## 4. Whole-class extension sequence diagram

@startuml
actor Lecturer
participant App
participant AssignmentService
participant ICourseRepository
participant DeadlinePolicy
participant IExtensionRepository
participant IUnitOfWork
participant AuditService

Lecturer -> App: Choose assignment, new deadline and reason
App -> AssignmentService: grantClassExtension(...)
AssignmentService -> AssignmentService: Check permission and assignment
AssignmentService -> ICourseRepository: isLecturerOfCourse(...)
ICourseRepository --> AssignmentService: authorised
AssignmentService -> ICourseRepository: studentIdsForCourse(courseId)
ICourseRepository --> AssignmentService: active student IDs
loop for each active student
  AssignmentService -> DeadlinePolicy: effectiveDueAt(assignment, studentId)
  DeadlinePolicy -> IExtensionRepository: latestApprovedDueAt(...)
  IExtensionRepository --> DeadlinePolicy: current approved deadline
  DeadlinePolicy --> AssignmentService: effective deadline
end
alt invalid deadline for any student
  AssignmentService --> App: RuleViolation (no records written)
else all deadlines valid
  AssignmentService -> IUnitOfWork: begin transaction
  loop for each active student
    AssignmentService -> IExtensionRepository: add approved Extension
  end
  AssignmentService -> AuditService: record GRANT_CLASS_EXTENSION
  AssignmentService -> IUnitOfWork: commit
  AssignmentService --> App: number of students extended
end
App --> Lecturer: Display confirmation
@enduml

## 5. Data model impact

No schema change is required. Existing relationships remain:

- courses 1 — many assignments
- courses 1 — many active enrolments
- assignments 1 — many extensions
- users 1 — many extensions (as the student)
- users 1 — many extensions (as the approving lecturer)

A whole-class extension is stored as multiple rows in the existing extensions table, one for each active student. The enrolments table is queried to determine the class membership. The audit_log table stores one summary event for the class-level action.

### Relevant entity relationship diagram

@startuml
entity courses {
  * course_id : INTEGER <<PK>>
}
entity assignments {
  * assignment_id : INTEGER <<PK>>
  --
  course_id : INTEGER <<FK>>
  due_at : INTEGER
}
entity enrolments {
  * enrolment_id : INTEGER <<PK>>
  --
  student_id : INTEGER <<FK>>
  course_id : INTEGER <<FK>>
  status : TEXT
}
entity extensions {
  * extension_id : INTEGER <<PK>>
  --
  assignment_id : INTEGER <<FK>>
  student_id : INTEGER <<FK>>
  requested_due_at : INTEGER
  reason : TEXT
  status : TEXT
  approved_by : INTEGER <<FK>>
}
courses ||--o{ assignments
courses ||--o{ enrolments
assignments ||--o{ extensions
@enduml

## 5. Deadline display

For each student assignment row, the UI calls AssignmentService::effectiveDueAtForStudent. That delegates to the existing DeadlinePolicy, which returns the later of the assignment's original due date and the student's latest approved extension. The deadline used for display is therefore the same deadline used by submission and late-penalty processing.
