# LMS Build Guide

## 1. Introduction

This document explains how to configure, compile, run and test the Learning Management System (LMS).

The project is developed in C++17 and uses CMake as its build system. Ninja is used to manage the compilation process.

## 2. Prerequisites

Before building the project, ensure that the following tools and dependencies are installed:

| Tool / Dependency | Purpose |
|---|---|
| C++17-compatible compiler (GCC) | Compiles the C++ source files |
| CMake | Configures the project build |
| Ninja | Executes the build process |
| SQLite3 | Provides persistent database storage |
| libsodium | Provides password hashing and security functionality |
| pkg-config | Helps CMake locate installed dependencies |

This guide uses Windows with MSYS2 and the UCRT64 environment.

## 3. Installing the Dependencies

### 3.1 Install MSYS2

Download and install MSYS2 from its official website:

https://www.msys2.org/

Open the **MSYS2 UCRT64** terminal after installation.

### 3.2 Install the compiler and dependencies

Run the following commands in the MSYS2 UCRT64 terminal:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

```bash
pacman -S mingw-w64-ucrt-x86_64-ninja
```

```bash
pacman -S mingw-w64-ucrt-x86_64-sqlite3
```

```bash
pacman -S mingw-w64-ucrt-x86_64-pkgconf
```

```bash
pacman -S mingw-w64-ucrt-x86_64-libsodium
```

CMake must also be installed and accessible from the Windows terminal.

### 3.3 Configure the Windows PATH

Add the following directory to the Windows PATH environment variable:

```text
C:\msys64\ucrt64\bin
```

This allows Windows and VS Code to locate the installed compiler and supporting tools.

Open a new PowerShell terminal after updating PATH.

Verify the installations:

```powershell
g++ --version
cmake --version
ninja --version
```

## 4. Obtaining the Project

Clone the LMS repository or download its source files from GitHub.

Open the project root directory in Visual Studio Code.

The project root is the directory containing the main `CMakeLists.txt` file.

Do not open only the `src` folder because CMake needs access to the complete project structure.

## 5. Configuring the Project

Open a PowerShell terminal in the project root directory.

Run:

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
```

This command:

- Uses the current directory as the source directory.
- Creates a `build-ninja` directory for generated build files.
- Selects Ninja as the build system.
- Specifies the MSYS2 UCRT64 installation directory so CMake can locate the required dependencies.

A successful configuration should display messages similar to:

```text
-- Configuring done
-- Generating done
-- Build files have been written to: ...
```

## 6. Building the LMS

After configuration succeeds, compile the project using:

```powershell
cmake --build build-ninja
```

CMake and Ninja will compile the source files and generate the application and test executables.

A successful build should complete without compilation or linking errors.

The generated executables include:

- `lms_app.exe` – the LMS application.
- `lms_tests.exe` – the automated test executable.

Both are generated inside the `build-ninja` directory.

## 7. Running the Application

To start the LMS application, run:

```powershell
.\build-ninja\lms_app.exe
```

The application will launch its console interface.

Follow the on-screen instructions to navigate the available menus and system functionality.

Ensure that any required database setup has been completed using the appropriate SQL scripts in the `sql` directory.

## 8. Running the Automated Tests

The project includes an automated test suite.

To execute the tests, run:

```powershell
.\build-ninja\lms_tests.exe
```

The test executable runs the available test cases and displays their results in the terminal.

A successful test run should report that all executed tests have passed.

## 9. Static Analysis

Cppcheck can be used to inspect the source code for potential problems and code-quality improvements.

Run the following command from the project root if Cppcheck is installed:

```powershell
cppcheck --enable=warning,style,performance,portability --std=c++17 --error-exitcode=1 --inline-suppr -I src src
```

The analysis checks the source files for warnings, style issues, performance concerns and portability issues.

Findings should be reviewed and documented as part of the project's quality assurance activities.

## 10. Project Structure

The repository is organised into the following main directories:

| Directory / File | Description |
|---|---|
| `src/` | Contains the LMS application source code and header files. |
| `tests/` | Contains automated tests. |
| `sql/` | Contains SQL scripts used for database setup and management. |
| `docs/` | Contains project documentation and reports. |
| `cmake/` | Contains additional CMake configuration files, where applicable. |
| `.github/` | Contains GitHub Actions workflow configuration. |
| `CMakeLists.txt` | Main CMake build configuration. |
| `.clang-tidy` | Contains Clang-Tidy static-analysis configuration. |
| `.gitignore` | Specifies files and directories excluded from Git tracking. |

## 11. Troubleshooting

### 11.1 Compiler not found

If CMake cannot locate GCC, check that the following directory has been added to the Windows PATH:

```text
C:\msys64\ucrt64\bin
```

Restart the terminal and verify that `g++ --version` works.

### 11.2 CMake cannot find a dependency

Ensure that the required MSYS2 packages are installed.

Also verify that the CMake configuration command includes:

```powershell
-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
```

### 11.3 Build directory uses a different generator

If CMake reports that the existing build directory was configured with another generator, remove the generated `build-ninja` directory and configure the project again.

Run:

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
```

Only remove the generated build directory, not the project source files.

### 11.4 Application executable cannot be found

Ensure that the build completed successfully:

```powershell
cmake --build build-ninja
```

Then check that `lms_app.exe` exists inside the `build-ninja` directory.

## 12. Important Notes

- The project requires a C++17-compatible compiler.
- The commands in this guide assume that the project is being built from its root directory.
- The `build-ninja` directory contains generated files and should not be committed to the GitHub repository.
- The application and automated tests should be rebuilt after source-code changes.
- The GitHub Actions workflow in `.github/workflows/ci.yml` provides an automated build and testing process.

---

**End of Build Guide**
