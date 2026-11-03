# CR1 Regression Test Results – v1.1

## 1. Purpose

This record tracks the regression run for CR1. The existing v1.0 suite contained 59 passing automated tests. Two new workflow tests were added for whole-class extensions, giving an expected total of 61 tests.

## 2. New regression tests

| Test | Expected result | Execution status |
|---|---|---|
| Workflow.ClassExtensionUpdatesEveryActiveStudentAndTheirDisplayedDeadline | All active students receive the new effective deadline; both can submit after the original deadline without being flagged late. | Passed — included in the 61-test run |
| Workflow.ClassExtensionRejectsDeadlineThatWouldShortenAnExistingIndividualExtension | The operation is rejected and no student's existing effective deadline is changed. | Pending full-suite rerun |

## 3. Existing functionality to regress

The complete existing suite must continue to pass, especially:

- Individual extension behaviour.
- Late submission and penalty calculations.
- Resubmission deadline and attempt-limit rules.
- Lecturer course permissions.
- Assignment creation and submission workflows.
- Marking, moderation, release and audit behaviour.
- Authentication, role permissions and database operations.

## 4. Execution instructions

From the repository root on Windows PowerShell, pull the v1.1 changes and run:

    git pull origin main
    cmake -S . -B build-ninja -G Ninja -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
    cmake --build build-ninja
    .\build-ninja\lms_tests.exe

The release gate is a successful build and a complete passing test suite. Replace the pending execution statuses below with the actual results from the test output.

## 5. Final execution record

| Measure | Result |
|---|---|
| Build | Passed — local CMake/Ninja build completed successfully |
| Total tests | 61 (14 test suites) |
| Passed | 61 |
| Failed | 0 |
| Overall regression result | **PASS** — all 61 tests passed |

Do not create or publish the v1.1 release tag until this section records a successful build and all tests passing.
