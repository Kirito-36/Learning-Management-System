# LMS Code Metrics Report

## 1. Introduction

This document presents code metrics for the Learning Management System (LMS). These metrics provide an overview of the size and quality of the source code.

## 2. Source Code Metrics

The following measurements were obtained from the project source files.

| Metric | Result |
|---|---:|
| Programming language | C++17 |
| Non-empty source lines | 2,667 |
| Source directory | `src/` |
| Build system | CMake and Ninja |
| Unit testing framework | GoogleTest |

## 3. Test Coverage Metrics

The latest successful CI coverage report recorded:

| Metric | Result |
|---|---:|
| Line coverage | 91.1% |
| Function coverage | 96.4% |
| Branch coverage | 53.9% |

The measured line coverage exceeds the project's minimum target of 70%.

## 4. Code Quality

Cppcheck is used to identify potential programming errors, style issues, performance concerns and portability problems.

The latest successful CI run completed static analysis without reporting diagnostics.

The project also includes a `.clang-tidy` configuration file for additional static-analysis checks.

## 5. Conclusion

The LMS contains 2,667 non-empty source lines and has achieved 91.1% line coverage.

The project uses automated testing and static-analysis tools to support maintainability, reliability and software quality.
