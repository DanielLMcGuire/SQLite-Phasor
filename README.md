# Phasor SQLite Bindings

Native SQLite database bindings for the Phasor virtual machine.

## Overview

This plugin provides the C style SQLite3 interface for Phasor.

## Installation

### Prerequisites

- Phasor 2.1.0 or later *(not out yet, compile from source)*
- C++17 compatible compiler

### Building

```bash
# Clone the repository
git clone https://github.com/DanielLMcGuire/SQLite-Phasor.git
cd SQLite-Phasor

# Build the plugin
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Quick Start

```javascript
// Open database
var db = sqlite_open("mydb.db");

// Create table
sqlite_exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");

// Insert data
sqlite_exec(db, "INSERT INTO users (name, age) VALUES ('Alice', 30)");
sqlite_exec(db, "INSERT INTO users (name, age) VALUES ('Bob', 25)");

// Query data
var stmt = sqlite_prepare(db, "SELECT * FROM users");
while (sqlite_step(stmt)) {
    var id = sqlite_column(stmt, 0);
    var name = sqlite_column(stmt, 1);
    var age = sqlite_column(stmt, 2);
    putf("User: %s (ID: %d, Age: %d)", name, id, age);
}

// Clean up
sqlite_finalize(stmt);
sqlite_close(db);
```
See the man page for info 
```bash
man man3/PHASOR_SQLITE.3
```

## API Reference

### Database Functions

#### `sqlite_open(path)`
Opens or creates a SQLite database file.

- **Parameters**: `path` - Path to database file
- **Returns**: Database handle (integer) on success, `null` on failure

#### `sqlite_close(db_handle)`
Closes an open database connection.

- **Parameters**: `db_handle` - Database handle from `sqlite_open()`
- **Returns**: `true` on success, `false` if handle invalid

#### `sqlite_exec(db_handle, sql)`
Executes a SQL statement that doesn't return data.

- **Parameters**: 
  - `db_handle` - Database handle
  - `sql` - SQL statement to execute
- **Returns**: `true` on success, `false` on failure
- **Use for**: CREATE, INSERT, UPDATE, DELETE, etc.

### Prepared Statement Functions

#### `sqlite_prepare(db_handle, sql)`
Prepares a SQL statement for execution.

- **Parameters**:
  - `db_handle` - Database handle
  - `sql` - SQL query to prepare
- **Returns**: Statement handle (integer) on success, `null` on failure

#### `sqlite_step(stmt_handle)`
Executes one step of a prepared statement.

- **Parameters**: `stmt_handle` - Statement handle from `sqlite_prepare()`
- **Returns**: 
  - `true` - Row available (SQLITE_ROW)
  - `false` - Execution complete (SQLITE_DONE)
  - `null` - Error occurred

#### `sqlite_column(stmt_handle, column_index)`
Retrieves a column value from the current row.

- **Parameters**:
  - `stmt_handle` - Statement handle
  - `column_index` - Zero-based column index
- **Returns**: Column value (integer, float, string, or null)

#### `sqlite_finalize(stmt_handle)`
Finalizes a prepared statement and releases resources.

- **Parameters**: `stmt_handle` - Statement handle to finalize
- **Returns**: `true` on success, `false` if handle invalid

### Utility Functions

#### `sqlite_free_string(string_handle)`
Frees a string from the internal string table.

- **Parameters**: `string_handle` - String handle to free
- **Returns**: `null`
- **Note**: Typically not needed in user code

## Examples

### Creating a Database Schema

```javascript
var db = sqlite_open("blog.db");

sqlite_exec(db, `CREATE TABLE IF NOT EXISTS posts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    content TEXT,
    created_at INTEGER
)`);

sqlite_exec(db, `CREATE TABLE IF NOT EXISTS comments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER,
    author TEXT,
    message TEXT,
    FOREIGN KEY (post_id) REFERENCES posts(id)
)`);

sqlite_close(db);
```

### Complex Queries

```javascript
var db = sqlite_open("sales.db");

var stmt = sqlite_prepare(db, `
    SELECT p.name, SUM(s.quantity) as total_sold, SUM(s.quantity * p.price) as revenue
    FROM products p
    JOIN sales s ON p.id = s.product_id
    GROUP BY p.id
    ORDER BY revenue DESC
`);

puts("Sales Report:");
puts("--------------------");

while (sqlite_step(stmt)) {
    var name = sqlite_column(stmt, 0);
    var quantity = sqlite_column(stmt, 1);
    var revenue = sqlite_column(stmt, 2);
    putf("%s: %d units, $%.2f revenue", name, quantity, revenue);
}

sqlite_finalize(stmt);
sqlite_close(db);
```

### Conditional Updates

```javascript
var db = sqlite_open("users.db");

// Find and deactivate inactive users
var threshold = time() - 30*24*60*60;
var stmt = sqlite_prepare(db, "SELECT id, name FROM users WHERE last_login < " + threshold);

while (sqlite_step(stmt)) {
    var id = sqlite_column(stmt, 0);
    var name = sqlite_column(stmt, 1);
    puts("Deactivating user: " + name);
    
    // Update status directly in loop
    // Note: This requires a separate statement since we're iterating
    var update_sql = "UPDATE users SET status = 'inactive' WHERE id = " + id;
    sqlite_exec(db, update_sql);
}

sqlite_finalize(stmt);
sqlite_close(db);
```

### Processing Results with Counters

```javascript
var db = sqlite_open("inventory.db");

var stmt = sqlite_prepare(db, "SELECT name, quantity FROM products WHERE quantity < 10");

var count = 0;
puts("Low Stock Items:");
puts("--------------------");

while (sqlite_step(stmt)) {
    var name = sqlite_column(stmt, 0);
    var quantity = sqlite_column(stmt, 1);
    count = count + 1;
    putf("%d. %s: %d remaining", count, name, quantity);
}

putf("Total items low on stock: %d", count);

sqlite_finalize(stmt);
sqlite_close(db);
```

## Type Mapping

| SQLite Type | Phasor Type | Notes |
|-------------|-------------|-------|
| INTEGER | int | 64-bit signed integer |
| REAL | float | Double-precision float |
| TEXT | string | UTF-8 string |
| NULL | null | Null value |
| BLOB | ❌ | Not currently supported |

## Best Practices

### Always Clean Up Resources

```javascript
var db = sqlite_open("data.db");
var stmt = sqlite_prepare(db, "SELECT * FROM table");

// ... use statement ...

sqlite_finalize(stmt);  // Always finalize statements
sqlite_close(db);       // Always close databases
```

## Limitations

- **No parameter binding**: Prepared statements don't support `?` placeholders yet
- **No BLOB support**: Binary data types are not currently handled
- **No transactions API**: Use `BEGIN`, `COMMIT`, `ROLLBACK` with `sqlite_exec()`
- **No arrays**: Phasor doesn't support arrays; use individual variables or database tables for collections

## Plugin Not Loading?

- Verify the plugin file is in the Phasor plugin directory
- Check file permissions (must be readable and executable)
- Ensure SQLite3 library is installed on the system
