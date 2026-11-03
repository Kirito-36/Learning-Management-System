#pragma once
// Thin RAII wrapper over SQLite. All SQL in the application goes through
// Statement, which only supports bound parameters (no string concatenation).
#include <cstdint>
#include <stdexcept>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace lms::data {

class DatabaseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);  // throws DatabaseError
    ~Statement();
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    // Parameter indices are 1-based, as in SQLite.
    Statement& bind(int index, int value);
    Statement& bind(int index, std::int64_t value);
    Statement& bind(int index, double value);
    Statement& bind(int index, const std::string& value);
    Statement& bindNull(int index);

    bool step();  // true while a row is available, false when done; throws on error
    void reset();

    std::int64_t columnInt(int index) const;
    double columnDouble(int index) const;
    std::string columnText(int index) const;
    bool columnIsNull(int index) const;

private:
    sqlite3* db_ = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
};

class Database {
public:
    explicit Database(const std::string& path);  // ":memory:" is allowed; throws DatabaseError
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void initialiseSchema();                // idempotent; uses the embedded sql/schema.sql
    void execute(const std::string& sql);   // for statements without parameters only
    Statement prepare(const std::string& sql);
    std::int64_t lastInsertRowId() const;
    sqlite3* handle() const noexcept { return db_; }

private:
    sqlite3* db_ = nullptr;
};

// Rolls back automatically unless commit() is called, so an exception can never
// leave a half-written mark or submission behind.
class Transaction {
public:
    explicit Transaction(Database& db);
    ~Transaction();
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    void commit();

private:
    Database& db_;
    bool done_ = false;
};

}  // namespace lms::data
