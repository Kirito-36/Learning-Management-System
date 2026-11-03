# Change Request Analysis – CR1

**Request:** A lecturer should be able to extend the deadline for a whole class, not just an individual student, and extensions should be shown on the student's assignment list.  
**Target release:** v1.1  
**Maintenance classification:** Perfective maintenance

## 1. Classification and justification

CR1 is **perfective maintenance** because it enhances existing LMS functionality in response to a new user need. The system already supports individual deadline extensions and calculates a student's effective deadline. The change extends this capability to all actively enrolled students on an assignment's course and makes the effective deadline visible in the student assignment list.

It is not corrective maintenance because the original individual-extension behaviour was not defective. It is not adaptive maintenance because no external platform or operating-environment change requires the modification.

## 2. Impact analysis

### 2.1 Requirements affected

The original requirement statements did not have stable IDs in the repository, so the affected requirements are identified by their behaviour:

| Requirement | Change |
|---|---|
| Lecturer extension management | Extend existing individual-extension capability with a whole-class extension option. |
| Class membership | A class extension applies to every actively enrolled student in the assignment's course. Withdrawn students are excluded. |
| Deadline validation | The requested class deadline must be later than every enrolled student's current effective deadline. The operation is all-or-nothing. |
| Student assignment list | Display the effective deadline, including individual and class extensions, rather than only the original assignment due date. |
| Access control | Only a lecturer assigned to the assignment's course may grant either type of extension. |
| Auditability | Record a class-extension audit event with the assignment, lecturer, new deadline, student count and reason. |

### 2.2 Diagrams and design artefacts affected

The earlier design diagrams were not stored in the repository. A version 1.1 design addendum is therefore supplied in docs/design/CR1_DESIGN_UPDATE.md. It updates the relevant class, interaction and data-model views:

- Use-case view: add “Grant whole-class extension” for Lecturer and Head of Department.
- Class view: add studentIdsForCourse, grantClassExtension and effectiveDueAtForStudent.
- Sequence view: show lecturer request, active-enrolment lookup, deadline validation, per-student extension records and audit event.
- Data model: show that extensions continues to store one approved row per student; no new table is required.

### 2.3 Classes and components affected

| Component | Impact |
|---|---|
| AssignmentService | Adds whole-class extension operation, validates the request for every active student, writes all extension rows in one unit of work, and exposes the effective student deadline for display. |
| ICourseRepository | Adds an interface operation to return active student IDs for a course. |
| SqliteCourseRepository | Implements the active-enrolment query. |
| App in src/main.cpp | Adds a lecturer/HOD menu option and displays the effective deadline in the student assignment list and submission selection list. |
| DeadlinePolicy | Reused without modification; continues to calculate the latest approved deadline for a student. |
| Extension / IExtensionRepository / SqliteExtensionRepository | Reused without structural changes; a class extension is represented by one extension record per enrolled student. |

### 2.4 Database tables affected

| Table | Impact |
|---|---|
| enrolments | Read to identify students whose enrolment status is ACTIVE. |
| extensions | Receives one approved extension row per active student, using the existing assignment_id, student_id, requested_due_at, reason, status and approved_by columns. |
| assignments | Read to identify the course and original due date. No column changes. |
| audit_log | Receives one GRANT_CLASS_EXTENSION event for the assignment. |

**Database migration:** None. The existing schema already supports individual extension records for multiple students. The class-level action is implemented by creating those existing records for all active enrolments in one transaction.

### 2.5 Tests affected and added

Existing regression tests that must continue to pass include:

- Workflow.ExtensionShiftsTheStudentsDeadline
- Workflow.ExtensionMustMoveTheDeadlineLaterAndNeedsAReason
- Workflow.LateWorkIsAcceptedFlaggedAndPenalisedAtMarking
- Workflow.ResubmissionAfterDeadlineIsRefused

New tests added to tests/WorkflowTests.cpp:

- Workflow.ClassExtensionUpdatesEveryActiveStudentAndTheirDisplayedDeadline — verifies that all active students receive the deadline, the displayed effective deadline is updated, and submissions after the original deadline are not treated as late.
- Workflow.ClassExtensionRejectsDeadlineThatWouldShortenAnExistingIndividualExtension — verifies that a class extension cannot shorten a student's existing individual extension and that no partial class update occurs.

## 3. Implementation summary

The lecturer/HOD menu now offers both individual and whole-class extensions. The class operation checks course ownership, validates the reason, retrieves active enrolments, checks the requested date against every student's current effective deadline, and writes all per-student records within one transaction. The audit log records one class-level event.

The student assignment list now displays the effective deadline calculated by the existing DeadlinePolicy. This same display is used when the student selects an assignment to submit.

## 4. Release

The change is intended for release v1.1, based on v1.0. The release must only be tagged after the build and complete regression test suite pass.
