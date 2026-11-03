# Code Coverage Report

**Project:** Learning Management System (LMS)  
**Date:** 3 November 2026  
**Coverage Tool:** GCC and GCOVR 7.2  
**Build System:** CMake and Ninja  
**Testing Framework:** GoogleTest  

## 1. Introduction

This report presents the code coverage results obtained from the automated testing of the Learning Management System. Code coverage was measured using GCC coverage instrumentation and GCOVR.

The purpose of the measurement was to determine how much of the business-logic source code was executed during automated testing.

## 2. Coverage Results

The latest successful GitHub Actions workflow (Run #18) produced the following results:

| Metric | Executed | Total | Coverage |
|---|---:|---:|---:|
| Lines | 571 | 627 | 91.1% |
| Functions | 106 | 110 | 96.4% |
| Branches | 557 | 1,033 | 53.9% |

## 3. Evaluation Against the Target

The project established a minimum line coverage target of 70% for the business-logic layer.

The measured line coverage was 91.1%, exceeding the target by 21.1 percentage points.

This indicates that most measured business-logic code was exercised during automated testing.

Function coverage was also high at 96.4%. However, branch coverage was 53.9%, indicating that some alternative decision paths were not exercised.

Additional tests could improve coverage of these paths, particularly in administrative operations, assignment management, authentication and submission-related conditions.

## 4. Continuous Integration

Coverage measurement is integrated into the GitHub Actions CI workflow.

The workflow:
- Configures the project with coverage instrumentation.
- Builds the application and test executable.
- Executes the automated tests.
- Generates the coverage report using GCOVR.
- Uploads the generated quality reports as a workflow artifact.

The latest workflow completed successfully, including the build, test, coverage and static-analysis steps.

## 5. Conclusion

The LMS achieved 91.1% line coverage, exceeding the minimum required target of 70%.

The results demonstrate substantial automated test coverage of the measured business-logic code. Further testing is recommended to improve branch coverage and exercise additional alternative and exception-handling paths.
