# Coding standard (CS4433 LMS)

Based on the C++ Core Guidelines, with these project rules:

1. **Language:** C++17, no compiler extensions. Warnings (`-Wall -Wextra -Wpedantic`) must be clean in CI.
2. **Layering:** `domain` depends on nothing; `data` and `security` depend on `domain`; `services` depend on
   `domain` and on repository interfaces only; the UI depends on `services`. No upward includes.
3. **Naming:** types `PascalCase`; functions and variables `camelCase`; private data members end in `_`;
   constants `kPascalCase`; namespaces `lms::<layer>`; files match the type they define (the SQLite repositories are grouped in `SqliteRepositories.*`).
4. **Ownership:** no raw `new`/`delete`; use `std::unique_ptr`/values. Polymorphic types have virtual destructors.
5. **Errors:** throw exceptions derived from `std::exception` for invalid input and database failures;
   catch them at the UI boundary. Destructors never throw.
6. **SQL:** only inside `src/data`; always parameterised via `Statement::bind`; never concatenate user input.
7. **Security:** never store or log plain-text passwords; hashing goes through `IPasswordHasher`.
8. **Const-correctness:** mark non-mutating members `const`; pass large objects by `const&`.
9. **Comments:** explain *why*, not *what*; every public interface has a short contract comment.
10. **Formatting:** 4 spaces, 100-column limit, braces on every `if`/`for`.
11. **Commits:** imperative subject line (`Add late-penalty strategy`), one logical change per commit,
    feature branches merged by pull request, releases tagged `vMAJOR.MINOR`.
