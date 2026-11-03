# Static Analysis Report

## 1. Purpose

Static analysis is used to identify potential programming errors, maintainability problems and other code-quality issues without executing the application.

## 2. Tool Used

The project uses Cppcheck to analyse the C++17 source code.

The CI workflow executes:

`cppcheck --enable=warning,style,performance,portability --std=c++17 --error-exitcode=1 --inline-suppr -I src src`

## 3. Results

The latest successful GitHub Actions run was completed on 3 November 2026.

| Check | Result |
|---|---|
| Source files analysed | 18 |
| Cppcheck diagnostics | 0 |
| Static analysis | Passed |
| Build warnings treated as errors | Enabled |
| CI workflow | Successful |

## 4. Interpretation

Cppcheck completed successfully without reporting diagnostics in the analysed source files.

The CMake configuration also enables compiler warnings and supports treating warnings as errors through the CI build configuration.

A `.clang-tidy` configuration file is included in the repository to define additional code-quality checks.

## 5. Conclusion

The static-analysis stage completed successfully. No Cppcheck diagnostics were reported during the latest CI execution. Static analysis will continue to be performed as part of the project's quality-assurance process.
