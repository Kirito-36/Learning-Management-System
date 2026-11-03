# Learning Management System – Test Plan

## 1. Introduction

This document describes the testing strategy for the Learning Management System (LMS), developed using C++17 and SQLite.

The purpose of testing is to verify that the system satisfies its specified functional and non-functional requirements, behaves correctly under normal and exceptional conditions, and meets the expectations of its intended users.

Testing follows the Waterfall V-Model, where each testing level corresponds to a development and design activity.

## 2. Testing Objectives

The main objectives are to:

- Verify that individual classes and functions operate correctly.
- Ensure that integrated components communicate and exchange data correctly.
- Confirm that the complete LMS satisfies its Software Requirements Specification (SRS).
- Validate that the system meets the needs of its intended users.
- Identify, record and correct defects before deployment.
- Verify that security, access control and assessment rules are enforced.

## 3. Testing Strategy

The project follows the V-Model, consisting of four main testing levels.

| Testing Level | Verification Artefact | Purpose |
|---|---|---|
| Unit Testing | Detailed class and method design | Test individual classes, functions and business rules. |
| Integration Testing | System architecture and component design | Verify interactions between services, repositories, database and other components. |
| System Testing | Software Requirements Specification | Test the complete application against its functional and non-functional requirements. |
| Acceptance Testing | User requirements and use cases | Confirm that the system supports the intended activities of its four user roles. |

### 3.1 Unit Testing

Unit testing focuses on individual components and business logic.

Examples include:

- Authentication and password verification.
- User-role validation.
- Deadline calculations and late-penalty strategies.
- Assignment and submission operations.
- Individual and whole-class extension rules, including active-enrolment scope and effective deadline display.
- Extension and resubmission rules.
- Marking and moderation operations.
- Audit record creation.

Automated unit tests are implemented using the project's existing testing framework.

### 3.2 Integration Testing

Integration testing verifies that different components work together correctly.

Examples include:

- Authentication service interacting with user repositories.
- Assignment service interacting with the database.
- Submission service storing and retrieving submissions.
- Marking service interacting with moderation and audit services.
- Database operations correctly preserving persistent records.

### 3.3 System Testing

System testing evaluates the complete LMS against the requirements in the SRS.

Testing includes:

- Successful and unsuccessful login attempts.
- Access restrictions for different user roles.
- Assignment creation and management.
- Submission and resubmission.
- Deadline and late-penalty enforcement.
- Extension handling.
- Marking, moderation and mark release.
- Audit logging.
- Database persistence and error handling.

### 3.4 Acceptance Testing

Acceptance testing evaluates whether the LMS supports the expected activities of its intended users.

The four user roles are:

- Student.
- Lecturer.
- Head of Department.
- Administrator.

Representative user scenarios are used to verify that each role can perform its permitted activities and cannot access unauthorised functionality.

## 4. Test Design Techniques

Two test design techniques are used to improve test coverage.

### 4.1 Equivalence Partitioning

Equivalence partitioning divides input data into groups expected to produce similar system behaviour.

Examples include:

| Input | Valid Equivalence Class | Invalid Equivalence Class |
|---|---|---|
| Login credentials | Correct username and password | Incorrect or missing credentials |
| User role | One of the four recognised roles | Unrecognised or unauthorised role |
| Assignment deadline | Valid date and time | Invalid or missing deadline |
| Submission | Valid submission | Empty or invalid submission |
| Mark | Within the permitted mark range | Below minimum or above maximum |

### 4.2 Boundary Value Analysis

Boundary value analysis tests values at and around important limits.

Examples include:

| Feature | Boundary Values to Test |
|---|---|
| Submission deadline | Just before, exactly at and just after the deadline |
| Late penalty | At the start and end of each applicable penalty boundary |
| Marks | Minimum, maximum and values immediately outside the permitted range |
| Resubmissions | Before, at and beyond the permitted attempt limit |
| Extensions | Original deadline, individual extension, whole-class extension, a student with an existing later individual extension, and immediately after the effective deadline |

The actual deadline, penalty and resubmission limits are taken from the approved SRS.

## 5. Entry and Exit Criteria

| Testing Level | Entry Criteria | Exit Criteria |
|---|---|---|
| Unit Testing | Individual components implemented and compilable. | All planned unit tests executed; failures investigated and recorded. |
| Integration Testing | Required components and interfaces implemented. | Integrated operations execute correctly; critical integration defects resolved. |
| System Testing | Integrated application builds and starts successfully. | Planned system tests completed; critical and high-severity defects addressed or formally documented. |
| Acceptance Testing | System testing completed and user scenarios prepared. | Acceptance scenarios executed and results documented. |

## 6. Test Environment

The testing environment consists of:

- Programming language: C++17.
- Build system: CMake.
- Build tool: Ninja.
- Compiler: GCC.
- Database: SQLite.
- Unit testing framework: Existing project test framework.
- Static analysis: Cppcheck and Clang-Tidy.
- Continuous integration: GitHub Actions.

## 7. Test Execution and Defect Management

Tests are executed locally and through the GitHub Actions CI workflow.

Each test result is recorded as:

- PASS – Actual result matches expected result.
- FAIL – Actual result differs from expected result.
- BLOCKED – Test cannot be executed because of an unresolved dependency or issue.

Discovered defects are recorded with their description, severity, cause, corrective action and retest result.

## 8. Code Coverage Target

The project aims to achieve at least 70% line coverage of the business-logic layer.

Coverage is measured using an appropriate coverage tool. The actual coverage percentage and supporting evidence will be included in the coverage report.

## 9. Conclusion

This test strategy provides a structured approach to verifying and validating the LMS. By following the V-Model and applying unit, integration, system and acceptance testing, the project aims to ensure that the implemented system meets its requirements and operates reliably.
