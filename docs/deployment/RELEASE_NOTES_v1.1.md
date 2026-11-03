# Learning Management System – Release Notes v1.1

**Release tag:** v1.1  
**Release type:** Maintenance release  
**Based on:** v1.0

## Overview

Version 1.1 implements change request CR1, improving deadline-extension management for lecturers and visibility of extended deadlines for students.

## Changes

- Lecturers and Heads of Department can grant an extension to every actively enrolled student on an assignment's course.
- The class extension is validated against every student's current effective deadline. A request that would shorten an existing individual extension is rejected before any changes are saved.
- The operation stores the extension for each student in the existing SQLite extensions table and records a class-level audit event.
- Students see their effective assignment deadline, including approved individual and whole-class extensions, in the assignment list and when selecting work to submit.
- Individual extensions remain supported.

## Database compatibility

No schema migration is required. Version 1.1 uses the existing enrolments, extensions and audit_log tables.

## Verification

The regression suite includes two additional workflow tests for whole-class extensions. The final test count and CI result must be recorded after the v1.1 build and complete regression run.

## Documentation

- Change request analysis: docs/maintenance/CR1_ANALYSIS.md
- Design update: docs/design/CR1_DESIGN_UPDATE.md
- Lecturer manual: docs/user-manuals/LECTURER_MANUAL.md
- Head of Department manual: docs/user-manuals/HOD_MANUAL.md
- Student manual: docs/user-manuals/STUDENT_MANUAL.md
