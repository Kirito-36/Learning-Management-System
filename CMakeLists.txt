cmake_minimum_required(VERSION 3.16)
project(LMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # used by clang-tidy

option(LMS_BUILD_TESTS "Build the unit tests" ON)
option(LMS_ENABLE_COVERAGE "Instrument for gcov/lcov coverage (GCC/Clang)" OFF)
option(LMS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# Embed sql/schema.sql in the binary so the application needs no external files.
file(READ "${CMAKE_SOURCE_DIR}/sql/schema.sql" LMS_SCHEMA_SQL)
configure_file(cmake/SchemaSql.hpp.in "${CMAKE_BINARY_DIR}/generated/SchemaSql.hpp" @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/sql/schema.sql")

add_library(lms_core STATIC
    src/domain/Types.cpp
    src/domain/User.cpp
    src/domain/DailyLatePenaltyStrategy.cpp
    src/domain/SubmissionStateMachine.cpp
    src/domain/TimeUtil.cpp
    src/data/Database.cpp
    src/data/SqliteRepositories.cpp
    src/security/SodiumPasswordHasher.cpp
    src/services/AdminService.cpp
    src/services/AssignmentService.cpp
    src/services/AuditService.cpp
    src/services/AuthService.cpp
    src/services/DeadlinePolicy.cpp
    src/services/MarkingService.cpp
    src/services/ModerationService.cpp
    src/services/ResultsService.cpp
    src/services/SubmissionService.cpp)
target_include_directories(lms_core PUBLIC src "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(lms_core PUBLIC SQLite::SQLite3 PkgConfig::SODIUM)

if(NOT MSVC)
    target_compile_options(lms_core PUBLIC -Wall -Wextra -Wpedantic)
    if(LMS_WARNINGS_AS_ERRORS)
        target_compile_options(lms_core PUBLIC -Werror)
    endif()
    if(LMS_ENABLE_COVERAGE)
        target_compile_options(lms_core PUBLIC --coverage -O0 -g)
        target_link_options(lms_core PUBLIC --coverage)
    endif()
endif()

add_executable(lms_app src/main.cpp)
target_link_libraries(lms_app PRIVATE lms_core)

if(LMS_BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    add_executable(lms_tests
        tests/PenaltyTests.cpp
        tests/StateMachineTests.cpp
        tests/TypesTests.cpp
        tests/UserTests.cpp
        tests/WorkflowTests.cpp
        tests/SecurityTests.cpp)
    target_include_directories(lms_tests PRIVATE tests)
    target_link_libraries(lms_tests PRIVATE lms_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(lms_tests)
endif()
