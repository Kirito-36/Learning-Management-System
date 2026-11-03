#include "data/Database.hpp"

#include <sqlite3.h>

#include <utility>

#include "SchemaSql.hpp"

namespace lms::data {
namespace {
[[noreturn]] void fail(sqlite3* db, const std::string& context) {
    throw DatabaseError(context + ": " + (db ? sqlite3_errmsg(db) : "no database handle"));
}
}  // namespace

// ---- Statement ----------------------------------------------------------------------------

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK) {
        fail(db_, "prepare failed");
    }
}

Statement::~Statement() {
    if (stmt_) sqlite3_finalize(stmt_);
}

Statement::Statement(Statement&& other) noexcept : db_(other.db_), stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_) sqlite3_finalize(stmt_);
        db_ = other.db_;
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
    }
    return *this;
}

Statement& Statement::bind(int index, int value) { return bind(index, static_cast<std::int64_t>(value)); }

Statement& Statement::bind(int index, std::int64_t value) {
    if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK) fail(db_, "bind failed");
    return *this;
}

Statement& Statement::bind(int index, double value) {
    if (sqlite3_bind_double(stmt_, index, value) != SQLITE_OK) fail(db_, "bind failed");
    return *this;
}

Statement& Statement::bind(int index, const std::string& value) {
    if (sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()),
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        fail(db_, "bind failed");
    }
    return *this;
}

Statement& Statement::bindNull(int index) {
    if (sqlite3_bind_null(stmt_, index) != SQLITE_OK) fail(db_, "bind failed");
    return *this;
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    fail(db_, "step failed");
}

void Statement::reset() {
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

std::int64_t Statement::columnInt(int index) const { return sqlite3_column_int64(stmt_, index); }
double Statement::columnDouble(int index) const { return sqlite3_column_double(stmt_, index); }
bool Statement::columnIsNull(int index) const {
    return sqlite3_column_type(stmt_, index) == SQLITE_NULL;
}

std::string Statement::columnText(int index) const {
    const unsigned char* text = sqlite3_column_text(stmt_, index);
    return text ? std::string(reinterpret_cast<const char*>(text)) : std::string();
}

// ---- Database -----------------------------------------------------------------------------

Database::Database(const std::string& path) {
    if (sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) !=
        SQLITE_OK) {
        const std::string message = db_ ? sqlite3_errmsg(db_) : "out of memory";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw DatabaseError("cannot open database '" + path + "': " + message);
    }
    sqlite3_busy_timeout(db_, 5000);
    execute("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::execute(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string message = error ? error : "unknown error";
        sqlite3_free(error);
        throw DatabaseError("execute failed: " + message);
    }
}

void Database::initialiseSchema() { execute(kSchemaSql); }

Statement Database::prepare(const std::string& sql) { return Statement(db_, sql); }

std::int64_t Database::lastInsertRowId() const { return sqlite3_last_insert_rowid(db_); }

// ---- Transaction --------------------------------------------------------------------------

Transaction::Transaction(Database& db) : db_(db) { db_.execute("BEGIN IMMEDIATE;"); }

Transaction::~Transaction() {
    if (!done_) {
        try {
            db_.execute("ROLLBACK;");
        } catch (...) {  // never throw from a destructor
        }
    }
}

void Transaction::commit() {
    db_.execute("COMMIT;");
    done_ = true;
}

}  // namespace lms::data
