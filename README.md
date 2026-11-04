<div align="center">

# 🎓 Learning Management System

**A role-based, C++17 learning management system for managing courses, assignments, submissions, and results.**

[Features](#-what-you-can-do) · [Getting started](#-getting-started) · [Documentation](#-documentation)

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/build-CMake-064F8C?logo=cmake&logoColor=white)
![Database](https://img.shields.io/badge/storage-SQLite-003B57?logo=sqlite&logoColor=white)

</div>

---

## ✨ What you can do

Bring everyday academic workflows together in one console application:

- **Organize learning:** Create departments and courses, assign lecturers, and enrol students.
- **Manage coursework:** Create assignments, track submissions and attempts, and grant individual or whole-class extensions.
- **Support assessment:** Mark submissions, send marks for moderation, and release results with feedback.
- **Keep accounts in order:** Manage role-based accounts for students, lecturers, Heads of Department, and administrators.
- **Maintain accountability:** Review audit history for important system activity.

## 👥 Built for the whole learning community

| Role | At a glance |
|---|---|
| **Students** | Find enrolled courses, review assignments, submit work, and check released results and feedback. |
| **Lecturers** | Manage course assignments, review submissions, grant extensions, and mark work. |
| **Heads of Department** | Participate in department-level academic workflows. |
| **Administrators** | Set up users, departments, courses, enrolments, and lecturer assignments. |

## 🛠️ Under the hood

Built in **C++17** with **CMake** and **Ninja**. The project uses **SQLite** for persistent storage and **libsodium** for password hashing and security functionality.

## 🚀 Getting started

Build prerequisites and step-by-step instructions are in the **[Build Guide](docs/Build_guide.md)**. The guide covers configuring, building, launching, and testing the application.

Once the prerequisites are installed, the main build and test commands are:

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
cmake --build build-ninja
.\build-ninja\lms_app.exe
.\build-ninja\lms_tests.exe
```

## 📚 Documentation

- **[Documentation index](docs/README.md)** — build and development guides, testing, deployment, quality, and user manuals.
- **[Student manual](docs/user-manuals/STUDENT_MANUAL.md)**
- **[Lecturer manual](docs/user-manuals/LECTURER_MANUAL.md)**
- **[Administrator manual](docs/user-manuals/ADMINISTRATOR_MANUAL.md)**
- **[HOD_manual](docs/user-manuals/HOD_MANUAL.md)**

---

<div align="center">

*Making course and coursework management a little more organized.*

</div>
