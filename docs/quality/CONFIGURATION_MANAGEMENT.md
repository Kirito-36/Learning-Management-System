# Configuration Management

## 1. Version Control

Git and GitHub are used for source-code version control and team collaboration. The project repository maintains the system's source files, documentation, configuration files and automated tests.

## 2. Branching Strategy

The project uses the following branching strategy:

- **main:** Contains the stable version of the system.
- **feature branches:** Used by developers to implement individual features or make changes.
- **bugfix branches:** Used to correct identified defects.

Changes should be committed to an appropriate branch and reviewed before being merged into the main branch.

## 3. Commit Management

Meaningful commit messages are used to describe changes made to the project. Commits should focus on specific changes, such as adding functionality, fixing defects, updating documentation or improving tests.

## 4. Build and Configuration Management

CMake is used to manage the C++17 build configuration, while Ninja can be used as the build system.

Project dependencies include:

- SQLite
- libsodium
- GoogleTest

Build configuration is maintained in `CMakeLists.txt`. The CI workflow installs the required dependencies and builds the application automatically.

## 5. Continuous Integration

GitHub Actions is configured through:

`.github/workflows/ci.yml`

The workflow runs when changes are pushed to the main branch or when pull requests target main.

The pipeline performs the following operations:

1. Checks out the repository.
2. Installs project dependencies.
3. Configures the CMake project.
4. Builds the application.
5. Executes automated tests.
6. Generates a code coverage report.
7. Runs Cppcheck static analysis.
8. Uploads quality reports as workflow artifacts.

## 6. Release Management

Stable project versions should be identified using Git tags. Release tags provide identifiable snapshots of the project at important milestones.

The repository's milestone releases should be documented alongside their corresponding commit identifiers.
