#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.hpp>
#include "sqlite/sqlite3.h"
#include <string>
#include <unordered_map>
#include <mutex>

static std::unordered_map<int, sqlite3*> db_table;
static std::unordered_map<int, sqlite3_stmt*> stmt_table;
static std::unordered_map<int, std::string> string_table;
static int next_db_handle = 1;
static int next_stmt_handle = 1;
static int next_string_handle = 1;
static std::mutex db_mutex;
static std::mutex stmt_mutex;
static std::mutex string_mutex;

sqlite3* get_db(int handle) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = db_table.find(handle);
    return it != db_table.end() ? it->second : nullptr;
}

sqlite3_stmt* get_stmt(int handle) {
    std::lock_guard<std::mutex> lock(stmt_mutex);
    auto it = stmt_table.find(handle);
    return it != stmt_table.end() ? it->second : nullptr;
}

int store_string(const std::string& s) {
    std::lock_guard<std::mutex> lock(string_mutex);
    int handle = next_string_handle++;
    string_table[handle] = s;
    return handle;
}

const char* get_string(int handle) {
    std::lock_guard<std::mutex> lock(string_mutex);
    auto it = string_table.find(handle);
    return it != string_table.end() ? it->second.c_str() : nullptr;
}

void free_string(int handle) {
    std::lock_guard<std::mutex> lock(string_mutex);
    string_table.erase(handle);
}

PhasorValue sqlite_open(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 1 || !phasor_is_string(argv[0])) return phasor_make_null();
    const char* filename = phasor_to_string(argv[0]);
    sqlite3* db = nullptr;
    if (sqlite3_open(filename, &db) != SQLITE_OK) { sqlite3_close(db); return phasor_make_null(); }

    std::lock_guard<std::mutex> lock(db_mutex);
    int handle = next_db_handle++;
    db_table[handle] = db;
    return phasor_make_int(handle);
}

PhasorValue sqlite_close(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 1 || !phasor_is_int(argv[0])) return phasor_make_bool(false);

    int handle = (int)phasor_to_int(argv[0]);
    sqlite3* db = nullptr;
    {
        std::lock_guard<std::mutex> lock(db_mutex);
        auto it = db_table.find(handle);
        if (it != db_table.end()) { db = it->second; db_table.erase(it); }
    }

    if (db) {
        sqlite3_close(db);
        return phasor_make_bool(true);
    }
    return phasor_make_bool(false);
}

PhasorValue sqlite_exec(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 2 || !phasor_is_int(argv[0]) || !phasor_is_string(argv[1])) 
        return phasor_make_bool(false);

    int handle = (int)phasor_to_int(argv[0]);
    const char* sql = phasor_to_string(argv[1]);
    sqlite3* db = get_db(handle);
    if (!db) return phasor_make_bool(false);

    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return phasor_make_bool(false);
    }
    return phasor_make_bool(true);
}

PhasorValue sqlite_prepare(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 2 || !phasor_is_int(argv[0]) || !phasor_is_string(argv[1])) return phasor_make_null();
    int db_handle = (int)phasor_to_int(argv[0]);
    const char* sql = phasor_to_string(argv[1]);
    sqlite3* db = get_db(db_handle);
    if (!db) return phasor_make_null();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return phasor_make_null();

    std::lock_guard<std::mutex> lock(stmt_mutex);
    int handle = next_stmt_handle++;
    stmt_table[handle] = stmt;
    return phasor_make_int(handle);
}

PhasorValue sqlite_step(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 1 || !phasor_is_int(argv[0])) return phasor_make_null();
    int handle = (int)phasor_to_int(argv[0]);
    sqlite3_stmt* stmt = get_stmt(handle);
    if (!stmt) return phasor_make_null();

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) return phasor_make_bool(true);
    if (rc == SQLITE_DONE) return phasor_make_bool(false);
    return phasor_make_null();
}

PhasorValue sqlite_column(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 2 || !phasor_is_int(argv[0]) || !phasor_is_int(argv[1])) return phasor_make_null();
    int stmt_handle = (int)phasor_to_int(argv[0]);
    int col_index = (int)phasor_to_int(argv[1]);
    sqlite3_stmt* stmt = get_stmt(stmt_handle);
    if (!stmt) return phasor_make_null();

    int count = sqlite3_column_count(stmt);
    if (col_index < 0 || col_index >= count) return phasor_make_null();

    int type = sqlite3_column_type(stmt, col_index);
    switch (type) {
    case SQLITE_INTEGER: return phasor_make_int(sqlite3_column_int(stmt, col_index));
    case SQLITE_FLOAT:   return phasor_make_float(sqlite3_column_double(stmt, col_index));
    case SQLITE_TEXT: {
        std::string s(reinterpret_cast<const char*>(sqlite3_column_text(stmt, col_index)));
        int str_handle = store_string(s);
        return phasor_make_string(get_string(str_handle));
    }
    case SQLITE_NULL: return phasor_make_null();
    default: return phasor_make_null();
    }
}

PhasorValue sqlite_finalize(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 1 || !phasor_is_int(argv[0])) return phasor_make_bool(false);

    int handle = (int)phasor_to_int(argv[0]);
    sqlite3_stmt* stmt = nullptr;
    {
        std::lock_guard<std::mutex> lock(stmt_mutex);
        auto it = stmt_table.find(handle);
        if (it != stmt_table.end()) { stmt = it->second; stmt_table.erase(it); }
    }

    if (stmt) {
        sqlite3_finalize(stmt);
        return phasor_make_bool(true);
    }
    return phasor_make_bool(false);
}


PhasorValue sqlite_free_string(PhasorVM* vm, int argc, const PhasorValue* argv) {
    if (argc != 1 || !phasor_is_int(argv[0])) return phasor_make_null();
    int handle = (int)phasor_to_int(argv[0]);
    free_string(handle);
    return phasor_make_null();
}

PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI* api, PhasorVM* vm) {
    api->register_function(vm, "sqlite_open", &sqlite_open);
    api->register_function(vm, "sqlite_close", &sqlite_close);
    api->register_function(vm, "sqlite_exec", &sqlite_exec);
    api->register_function(vm, "sqlite_prepare", &sqlite_prepare);
    api->register_function(vm, "sqlite_step", &sqlite_step);
    api->register_function(vm, "sqlite_column", &sqlite_column);
    api->register_function(vm, "sqlite_finalize", &sqlite_finalize);
    api->register_function(vm, "sqlite_free_string", &sqlite_free_string);
}
