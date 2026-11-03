# Learning Management System - Release Notes v1.0

**Release tag:** `v1.0`  
**Release type:** Initial release  
**Application:** C++17 console-based Learning Management System

## Overview

Version 1.0 provides a role-based console LMS for administrators, lecturers, Heads of Department and students. It uses SQLite for persistent storage and libsodium for password hashing.

## Main features

- First-time creation of the initial administrator account.
- User account creation and activation/deactivation.
- Department and course management.
- Lecturer assignment to courses and student enrolment.
- Assignment creation, due dates and attempt limits.
- Student submission and resubmission tracking.
- Deadline extensions and late-submission penalty handling.
- Lecturer marking with feedback.
- Head of Department moderation and mark release.
- Student access to released marks and feedback.
- Audit history for recorded system actions.
- Automated test suite and GitHub Actions build/test workflow.

## Technical requirements

- C++17-compatible GCC compiler.
- CMake and Ninja.
- SQLite3.
- libsodium.
- pkg-config.
- Windows/MSYS2 UCRT64 build instructions are provided in `docs/Build_guide.md` and the installation guide.

## Installation

Follow `docs/deployment/INSTALLATION_GUIDE.md`.

## User documentation

- Administrator: `docs/user-manuals/ADMINISTRATOR_MANUAL.md`
- Lecturer: `docs/user-manuals/LECTURER_MANUAL.md`
- Head of Department: `docs/user-manuals/HOD_MANUAL.md`
- Student: `docs/user-manuals/STUDENT_MANUAL.md`

## Verification

The latest reported successful CI run completed the build and passed 59/59 automated tests. Reported coverage was 91.1% line coverage, 96.4% function coverage and 53.9% branch coverage.

## Known deployment considerations

- The application is a local console program, not a hosted web service.
- The default SQLite database is `lms.db` in the current working directory.
- Student submission files are referenced by filesystem path and must remain accessible.
- Back up the database and submission files regularly.

## Changes after v1.0

None. This is the initial release.
