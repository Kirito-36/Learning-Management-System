# Learning Management System – Defect Log

## 1. Introduction

This document records defects identified during the verification and validation of the Learning Management System (LMS).

Each defect should be recorded with its severity, cause, corrective action and retest result.

## 2. Defect Summary

During the latest recorded automated CI run on 3 November 2026:

- All 59 automated tests passed.
- The project built successfully.
- Cppcheck completed successfully without reporting diagnostics.

No failing automated tests or Cppcheck diagnostics were recorded in this run.

This does not establish that the entire application is defect-free. Issues discovered during manual system or acceptance testing should be added to this log.

## 3. Defect Register

| Defect ID | Description | Severity | Cause | Corrective action | Retest result |
|---|---|---|---|---|---|
| None recorded | No failing automated tests were recorded in the latest CI run. | N/A | N/A | No correction required for this run. | 59/59 tests passed |

## 4. Defect Severity Classification

| Severity | Description |
|---|---|
| Critical | Prevents the system from operating or compromises essential data or security. |
| High | Prevents an important function or user role from performing its required activities. |
| Medium | A function behaves incorrectly, but a workaround may be available. |
| Low | Minor usability, display or non-essential functional issue. |

## 5. Conclusion

The latest automated test run completed successfully without recorded test failures. Any defects identified during further manual testing will be documented, corrected and retested before the final release.
