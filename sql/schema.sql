-- LMS database schema (SQLite). Normalised to 3NF; see report section 5.4.
-- Timestamps are INTEGER seconds since the Unix epoch (UTC).
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS departments (
    department_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name          TEXT    NOT NULL UNIQUE,
    hod_user_id   INTEGER UNIQUE REFERENCES users(user_id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS users (
    user_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    username       TEXT    NOT NULL UNIQUE COLLATE NOCASE,
    name           TEXT    NOT NULL,
    password_hash  TEXT    NOT NULL,          -- Argon2id string; the salt is embedded
    role           TEXT    NOT NULL CHECK (role IN ('ADMIN','LECTURER','HOD','STUDENT')),
    student_number TEXT    UNIQUE,
    staff_number   TEXT    UNIQUE,
    department_id  INTEGER REFERENCES departments(department_id),
    active         INTEGER NOT NULL DEFAULT 1 CHECK (active IN (0,1)),
    CHECK ((role = 'STUDENT' AND student_number IS NOT NULL AND staff_number IS NULL
                AND department_id IS NULL)
        OR (role IN ('LECTURER','HOD') AND staff_number IS NOT NULL AND student_number IS NULL
                AND department_id IS NOT NULL)
        OR (role = 'ADMIN' AND student_number IS NULL AND staff_number IS NULL
                AND department_id IS NULL))
);

CREATE TABLE IF NOT EXISTS courses (
    course_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    course_code   TEXT    NOT NULL UNIQUE,
    title         TEXT    NOT NULL,
    academic_year INTEGER NOT NULL CHECK (academic_year BETWEEN 2000 AND 2100),
    department_id INTEGER NOT NULL REFERENCES departments(department_id)
);

CREATE TABLE IF NOT EXISTS course_lecturers (
    course_id   INTEGER NOT NULL REFERENCES courses(course_id) ON DELETE CASCADE,
    lecturer_id INTEGER NOT NULL REFERENCES users(user_id),
    PRIMARY KEY (course_id, lecturer_id)
);

CREATE TABLE IF NOT EXISTS enrolments (
    enrolment_id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id   INTEGER NOT NULL REFERENCES users(user_id),
    course_id    INTEGER NOT NULL REFERENCES courses(course_id) ON DELETE CASCADE,
    enrolled_at  INTEGER NOT NULL,
    status       TEXT    NOT NULL DEFAULT 'ACTIVE' CHECK (status IN ('ACTIVE','WITHDRAWN')),
    UNIQUE (student_id, course_id)
);

CREATE TABLE IF NOT EXISTS assignments (
    assignment_id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id     INTEGER NOT NULL REFERENCES courses(course_id) ON DELETE CASCADE,
    created_by    INTEGER NOT NULL REFERENCES users(user_id),
    title         TEXT    NOT NULL,
    description   TEXT    NOT NULL DEFAULT '',
    due_at        INTEGER NOT NULL,
    max_mark      REAL    NOT NULL CHECK (max_mark > 0),
    max_attempts  INTEGER NOT NULL DEFAULT 3 CHECK (max_attempts >= 1)
);

CREATE TABLE IF NOT EXISTS submissions (
    submission_id INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id INTEGER NOT NULL REFERENCES assignments(assignment_id) ON DELETE CASCADE,
    student_id    INTEGER NOT NULL REFERENCES users(user_id),
    attempt_no    INTEGER NOT NULL CHECK (attempt_no >= 1),
    submitted_at  INTEGER NOT NULL,
    file_path     TEXT    NOT NULL,
    status        TEXT    NOT NULL CHECK (status IN ('DRAFT','SUBMITTED','UNDER_REVIEW',
                      'MARKED_PENDING_MODERATION','CHANGES_REQUESTED','APPROVED',
                      'RELEASED','SUPERSEDED')),
    is_late       INTEGER NOT NULL DEFAULT 0 CHECK (is_late IN (0,1)),
    UNIQUE (assignment_id, student_id, attempt_no)
);

CREATE TABLE IF NOT EXISTS extensions (
    extension_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id    INTEGER NOT NULL REFERENCES assignments(assignment_id) ON DELETE CASCADE,
    student_id       INTEGER NOT NULL REFERENCES users(user_id),
    requested_due_at INTEGER NOT NULL,
    reason           TEXT    NOT NULL,
    status           TEXT    NOT NULL DEFAULT 'PENDING'
                         CHECK (status IN ('PENDING','APPROVED','REJECTED')),
    approved_by      INTEGER REFERENCES users(user_id)
);

CREATE TABLE IF NOT EXISTS marks (
    mark_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id INTEGER NOT NULL UNIQUE REFERENCES submissions(submission_id) ON DELETE CASCADE,
    marked_by     INTEGER NOT NULL REFERENCES users(user_id),
    raw_mark      REAL    NOT NULL CHECK (raw_mark >= 0),
    final_mark    REAL    NOT NULL CHECK (final_mark >= 0),
    feedback      TEXT    NOT NULL DEFAULT '',
    status        TEXT    NOT NULL CHECK (status IN ('PENDING_MODERATION','CHANGES_REQUESTED',
                      'APPROVED','RELEASED')),
    marked_at     INTEGER NOT NULL,
    released_at   INTEGER,
    CHECK (final_mark <= raw_mark)
);

CREATE TABLE IF NOT EXISTS moderation_records (
    moderation_id INTEGER PRIMARY KEY AUTOINCREMENT,
    mark_id       INTEGER NOT NULL REFERENCES marks(mark_id) ON DELETE CASCADE,
    reviewed_by   INTEGER NOT NULL REFERENCES users(user_id),
    decision      TEXT    NOT NULL CHECK (decision IN ('APPROVED','CHANGES_REQUESTED')),
    comment       TEXT    NOT NULL DEFAULT '',
    decided_at    INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS audit_log (
    audit_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    actor_user_id INTEGER REFERENCES users(user_id) ON DELETE SET NULL,
    action        TEXT    NOT NULL,
    entity_type   TEXT    NOT NULL,
    entity_id     INTEGER NOT NULL,
    old_value     TEXT,
    new_value     TEXT,
    reason        TEXT,
    occurred_at   INTEGER NOT NULL
);

-- The audit trail is append-only: no edits, no deletions.
CREATE TRIGGER IF NOT EXISTS audit_log_no_update BEFORE UPDATE ON audit_log
BEGIN
    SELECT RAISE(ABORT, 'audit_log is append-only');
END;

CREATE TRIGGER IF NOT EXISTS audit_log_no_delete BEFORE DELETE ON audit_log
BEGIN
    SELECT RAISE(ABORT, 'audit_log is append-only');
END;

CREATE INDEX IF NOT EXISTS idx_enrolments_course     ON enrolments(course_id);
CREATE INDEX IF NOT EXISTS idx_assignments_course    ON assignments(course_id);
CREATE INDEX IF NOT EXISTS idx_submissions_lookup    ON submissions(assignment_id, student_id);
CREATE INDEX IF NOT EXISTS idx_extensions_lookup     ON extensions(assignment_id, student_id);
CREATE INDEX IF NOT EXISTS idx_moderation_mark       ON moderation_records(mark_id);
CREATE INDEX IF NOT EXISTS idx_audit_entity          ON audit_log(entity_type, entity_id);
