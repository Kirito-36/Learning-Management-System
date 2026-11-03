# Moderated LMS (CS4433)

C++17 console application with SQLite persistence and role-based access
(Administrator, Lecturer, Head of Department, Student).

## Build

Requirements: CMake >= 3.16, a C++17 compiler, SQLite3 dev files, libsodium dev files.
On Debian/Ubuntu: `sudo apt install cmake g++ pkg-config libsqlite3-dev libsodium-dev`.

    cmake -S . -B build
    cmake --build build -j
    ctest --test-dir build --output-on-failure
    ./build/lms_app lms.db

The first configure downloads GoogleTest unless it is already installed (`libgtest-dev`).
Add `-DLMS_ENABLE_COVERAGE=ON` for coverage; see `.github/workflows/ci.yml` for the report command.

## Layout

    src/domain     entities, enums, late-penalty strategy, submission state machine (no dependencies)
    src/data       SQLite wrapper, repository interfaces, SQLite implementations, unit of work
    src/services   use-case services (auth, admin, assignments, submission, marking, moderation, results, audit)
    src/security   libsodium password hasher
    sql/schema.sql database schema (embedded into the binary at build time)
    tests/         GoogleTest unit tests
