# LMS Installation Guide

**Audience:** School technician  
**Release:** v1.0  
**Platform covered:** Windows 10/11, MSYS2 UCRT64

## 1. Requirements

Prepare a Windows computer with:
- Internet access for downloading the project and dependencies.
- A C++17 compiler (GCC), CMake and Ninja.
- SQLite3, libsodium and pkg-config.
- Sufficient disk space for the application, database and student submission files.

## 2. Install the development/runtime dependencies

1. Install MSYS2 from https://www.msys2.org/.
2. Open the **MSYS2 UCRT64** terminal.
3. Update the package database and packages when prompted:
   ```bash
   pacman -Syu
   ```
   If the terminal closes during the update, reopen UCRT64 and repeat the update command if required.
4. Install the required packages:
   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-libsodium
   ```
5. Install CMake for Windows if it is not already installed. Ensure its installation directory is on PATH.
6. Add `C:\msys64\ucrt64\bin` to the Windows PATH.
7. Open a new PowerShell terminal and verify:
   ```powershell
   g++ --version
   cmake --version
   ninja --version
   ```

## 3. Obtain the LMS source

1. Download the v1.0 source ZIP from the project's GitHub Releases page, or clone the repository.
2. Extract the ZIP to a permanent folder, for example `C:\SchoolLMS`.
3. Open PowerShell in that folder. Confirm that `CMakeLists.txt` is present.

## 4. Configure and build

Run these commands from the project root:

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
cmake --build build-ninja
```

A successful build creates:
- `build-ninja\lms_app.exe` — the LMS console application.
- `build-ninja\lms_tests.exe` — automated tests (not needed for ordinary daily use).

If configuration fails, check that GCC, CMake, Ninja, SQLite3, libsodium and pkg-config are installed and that the UCRT64 directory is on PATH.

## 5. First launch and administrator setup

1. Open PowerShell in the project root directory.
2. Start the application:
   ```powershell
   .\build-ninja\lms_app.exe
   ```
3. On the first launch, the application reports that no administrator exists and asks for an initial username, full name and password.
4. Create the initial administrator account. Use a strong password and keep it in the school's approved password-management process.
5. Sign in with the new administrator credentials.
6. Create departments, then create user accounts and courses using the Administrator menu.

The application creates and initialises its SQLite database automatically. By default it is named `lms.db` in the current working directory. To use a different database location, pass the path after the executable, for example:

```powershell
.\build-ninja\lms_app.exe "C:\SchoolLMSData\lms.db"
```

Ensure the target folder exists and the application account has permission to read and write there.

## 6. Verify installation

1. Sign in as the administrator.
2. Confirm that the Administrator menu appears.
3. Create a test department and test accounts if permitted.
4. Sign out and sign in using a test account for each configured role.
5. Confirm that the correct role menu appears.
6. Close the application normally by entering `0` at the sign-in prompt.
7. Back up the database before entering real school data.

To run the automated test suite after building:

```powershell
.\build-ninja\lms_tests.exe
```

## 7. Backup and maintenance

- Close the LMS before copying the database file.
- Back up `lms.db` regularly to an access-controlled location.
- Back up student submission files separately; the database stores their file paths.
- Restrict write access to the database and submission folders to authorised staff.
- Do not delete or overwrite the database when updating the application.
- Before installing an update, make a backup of the database and record the current application version.

## 8. Troubleshooting

| Problem | Action |
|---|---|
| `g++` not recognised | Check that `C:\msys64\ucrt64\bin` is on PATH, then reopen PowerShell. |
| CMake cannot find SQLite3 or libsodium | Install the missing MSYS2 package and rerun the configure command. |
| Ninja is not found | Install the UCRT64 Ninja package and reopen the terminal. |
| Build directory has a different generator | Remove only the generated `build-ninja` directory, then rerun configuration. |
| Application cannot open database | Check that the database folder exists and the user has read/write permission. |
| User cannot access a submission | Check that the submitted file path is correct and the file is still present. |

## 9. Support information

Project source and release history are maintained in the GitHub repository:
https://github.com/Kirito-36/Learning-Management-System
