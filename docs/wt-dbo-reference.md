# Wt::Dbo (Database Objects) Reference Guide

Wt::Dbo is a C++ ORM (Object Relational Mapping) library that provides a class-based view on database tables. It keeps an object hierarchy of database objects automatically synchronized with a database by inserting, updating and deleting database records.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Basic Mapping](#basic-mapping)
3. [Session Management](#session-management)
4. [CRUD Operations](#crud-operations)
5. [Relationships](#relationships)
6. [Querying](#querying)
7. [Transactions](#transactions)
8. [Customization](#customization)
9. [Advanced Topics](#advanced-topics)

## Core Concepts

### Namespace Alias
```cpp
#include <Wt/Dbo/Dbo.h>
namespace dbo = Wt::Dbo;
```

### Key Classes
- **`dbo::Session`** - Main interface for database operations
- **`dbo::ptr<T>`** - Smart pointer for database objects
- **`dbo::weak_ptr<T>`** - Weak reference for database objects
- **`dbo::collection<T>`** - Container for query results
- **`dbo::Transaction`** - RAII transaction management
- **`dbo::Query<T>`** - Query builder and executor

### Database Object Requirements
- Must have a `persist()` template method
- Each field mapped using `dbo::field()`
- Relationships mapped using `dbo::belongsTo()`, `dbo::hasMany()`, `dbo::hasOne()`

## Basic Mapping

### Simple Class Mapping
```cpp
enum class Role {
    Visitor = 0,
    Admin = 1,
    Alien = 42
};

class User {
public:
    std::string name;
    std::string password;
    Role        role;
    int         karma;

    template<class Action>
    void persist(Action& a)
    {
        dbo::field(a, name,     "name");
        dbo::field(a, password, "password");
        dbo::field(a, role,     "role");
        dbo::field(a, karma,    "karma");
    }
};
```

### Supported Types
- **Primitives**: `int`, `long`, `float`, `double`, `bool`
- **Strings**: `std::string`
- **Enums**: Any enum type
- **Wt Types**: `WDate`, `WDateTime`, `WTime`, `WString` (requires `<Wt/Dbo/WtSqlTraits.h>`)
- **Custom Types**: Via specializing `dbo::sql_value_traits<T>`

### Field Mapping Options
```cpp
// Basic field mapping
dbo::field(a, fieldName, "column_name");

// Field with size constraint
dbo::field(a, fieldName, "column_name", 255);

// Optional field (nullable)
dbo::field(a, optionalField, "optional_column");
```

## Session Management

### Creating a Session
```cpp
void setupSession() {
    // SQLite backend
    auto sqlite3 = std::make_unique<dbo::backend::Sqlite3>("blog.db");
    dbo::Session session;
    session.setConnection(std::move(sqlite3));

    // Map classes to tables
    session.mapClass<User>("user");
    session.mapClass<Post>("post");

    // Create schema (development/deployment)
    session.createTables();
}

// Other supported backends:
// PostgreSQL: dbo::backend::Postgres
// MySQL: dbo::backend::MySQL
// SQL Server: dbo::backend::MSSQLServer
```

### Connection Pooling
```cpp
// For multiple concurrent sessions
auto connectionPool = std::make_shared<dbo::FixedSqlConnectionPool>(
    std::make_unique<dbo::backend::Sqlite3>("blog.db"), 10);

dbo::Session session;
session.setConnectionPool(connectionPool);
```

### Session Properties
```cpp
// Enable query logging
session.setProperty("show-queries", "true");

// Configure connection properties
sqlite3->setProperty("show-queries", "true");
sqlite3->setProperty("foreign-keys", "true");
```

## CRUD Operations

### Creating Objects
```cpp
dbo::Transaction transaction(session);

// Method 1: Create then add
auto user = std::make_unique<User>();
user->name = "Joe";
user->password = "Secret";
user->role = Role::Visitor;
user->karma = 13;
dbo::ptr<User> userPtr = session.add(std::move(user));

// Method 2: AddNew shorthand
dbo::ptr<User> newUser = session.addNew<User>();
newUser.modify()->name = "Jane";
newUser.modify()->email = "jane@example.com";

// Method 3: Using dbo::make_ptr
auto userPtr2 = session.add(dbo::make_ptr<User>());
```

### Reading Objects
```cpp
dbo::Transaction transaction(session);

// Find by condition
dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

// Find by ID
dbo::ptr<User> user = session.load<User>(userId);

// Check if object exists
if (joe) {
    std::cout << "Joe's karma: " << joe->karma << std::endl;
}
```

### Updating Objects
```cpp
dbo::Transaction transaction(session);

dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

// Modify object (marks as dirty)
joe.modify()->karma++;
joe.modify()->password = "newpassword";

// Explicit flush (optional - auto-flushed on commit)
joe.flush();
```

### Deleting Objects
```cpp
dbo::Transaction transaction(session);

dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");
joe.remove();

// Object can still be used and re-added
joe.modify()->name = "Joe Restored";
session.add(joe);
```

## Relationships

### Many-to-One (belongs_to / has_many)
```cpp
class Post {
public:
    std::string title;
    std::string content;
    dbo::ptr<User> user;  // Many posts belong to one user

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, title, "title");
        dbo::field(a, content, "content");
        dbo::belongsTo(a, user, "user");
    }
};

class User {
public:
    std::string name;
    dbo::collection<dbo::ptr<Post>> posts;  // One user has many posts

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::hasMany(a, posts, dbo::ManyToOne, "user");
    }
};

// Usage
dbo::ptr<Post> post = session.addNew<Post>();
post.modify()->user = joe;
// Automatically reflected: joe->posts now contains post
```

### Many-to-Many
```cpp
class Post {
public:
    std::string title;
    dbo::collection<dbo::ptr<Tag>> tags;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, title, "title");
        dbo::hasMany(a, tags, dbo::ManyToMany, "post_tags");
    }
};

class Tag {
public:
    std::string name;
    dbo::collection<dbo::ptr<Post>> posts;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::hasMany(a, posts, dbo::ManyToMany, "post_tags");
    }
};

// Usage
post.modify()->tags.insert(cookingTag);
// Automatically creates join table entry
```

### One-to-One
```cpp
class Settings {
public:
    std::string theme;
    dbo::ptr<User> user;  // Owned side

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, theme, "theme");
        dbo::belongsTo(a, user);
    }
};

class User {
public:
    std::string name;
    dbo::weak_ptr<Settings> settings;  // Owning side

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::hasOne(a, settings);
    }
};
```

### Foreign Key Constraints
```cpp
// In belongsTo() calls
dbo::belongsTo(a, user, "user", dbo::OnDeleteCascade);
dbo::belongsTo(a, user, "user", dbo::NotNull);
dbo::belongsTo(a, user, "user", dbo::OnUpdateCascade | dbo::OnDeleteSetNull);

// Available constraints:
// - dbo::NotNull
// - dbo::OnUpdateCascade
// - dbo::OnUpdateSetNull  
// - dbo::OnDeleteCascade
// - dbo::OnDeleteSetNull
```

## Querying

### Basic Queries
```cpp
dbo::Transaction transaction(session);

// Find single object
dbo::ptr<User> user = session.find<User>().where("name = ?").bind("Joe");

// Find with multiple conditions
auto users = session.find<User>()
    .where("karma > ? AND role = ?")
    .bind(10)
    .bind(Role::Admin);

// Order and limit
auto topUsers = session.find<User>()
    .orderBy("karma DESC")
    .limit(10);
```

### Collection Queries
```cpp
using Users = dbo::collection<dbo::ptr<User>>;

Users users = session.find<User>().where("karma > ?").bind(50);

// Get count
std::cout << "Found " << users.size() << " users" << std::endl;

// Iterate results
for (const dbo::ptr<User>& user : users) {
    std::cout << user->name << " has karma: " << user->karma << std::endl;
}
```

### Raw SQL Queries
```cpp
// Query for objects
dbo::ptr<User> user = session.query<dbo::ptr<User>>("select u from user u")
    .where("name = ?").bind("Joe");

// Query for primitives
int count = session.query<int>("select count(1) from user")
    .where("karma > ?").bind(100);

// Query for tuples
using UserInfo = std::tuple<std::string, int>;
dbo::collection<UserInfo> info = session.query<UserInfo>(
    "select name, karma from user").where("role = ?").bind(Role::Admin);
```

### Query Building
```cpp
auto query = session.find<User>();

// Dynamic conditions
if (!nameFilter.empty()) {
    query = query.where("name LIKE ?").bind("%" + nameFilter + "%");
}

if (minKarma > 0) {
    query = query.where("karma >= ?").bind(minKarma);
}

// Execute
auto results = query.resultList();
```

### Binding Strategies
```cpp
// Dynamic binding (default) - reusable query
auto query = session.find<User>().where("name = ?");
auto joe = query.bind("Joe").resultValue();
auto jane = query.bind("Jane").resultValue();

// Direct binding - single use, lower overhead
auto user = session.find<User, dbo::DirectBinding>()
    .where("name = ?").bind("Joe");
```

## Transactions

### Basic Transaction Usage
```cpp
// RAII transaction
{
    dbo::Transaction transaction(session);
    
    auto user = session.addNew<User>();
    user.modify()->name = "NewUser";
    
    // Auto-commit on scope exit
}

// Manual commit/rollback
dbo::Transaction transaction(session);
try {
    // Database operations
    transaction.commit();
} catch (...) {
    transaction.rollback();
    throw;
}
```

### Nested Transactions
```cpp
void outerOperation() {
    dbo::Transaction transaction(session);
    
    // Some operations
    innerOperation(); // Creates nested transaction
    
    // More operations
    // Both transactions must commit for changes to persist
}

void innerOperation() {
    dbo::Transaction transaction(session); // Participates in outer transaction
    // Operations here
}
```

### Optimistic Locking
```cpp
try {
    dbo::Transaction transaction(session);
    
    auto user = session.find<User>().where("id = ?").bind(userId);
    user.modify()->karma++;
    
    transaction.commit();
    
} catch (dbo::StaleObjectException& e) {
    // Handle concurrent modification
    // Must reread() the object before retrying
    user.reread();
    // Retry logic here
}
```

### Transaction Isolation
```cpp
// Read Committed (minimum required level)
// Modifications only visible after commit
// Prevents dirty reads

// Handle transaction failures
void performOperation() {
    int retries = 3;
    while (retries-- > 0) {
        try {
            dbo::Transaction transaction(session);
            // Database operations
            transaction.commit();
            break;
        } catch (dbo::StaleObjectException&) {
            if (retries == 0) throw;
            // Reread stale objects and retry
        }
    }
}
```

## Customization

### Custom Primary Key Field Name
```cpp
namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<Post> : public dbo_default_traits {
            static const char *surrogateIdField() {
                return "post_id";  // Default is "id"
            }
        };
    }
}
```

### Disable Version Field
```cpp
namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<Post> : public dbo_default_traits {
            static const char *versionField() {
                return nullptr;  // Disables optimistic locking
            }
        };
    }
}
```

### Natural Primary Key
```cpp
class User {
public:
    std::string userId;  // Natural key
    std::string name;

    template<class Action>
    void persist(Action& a) {
        dbo::id(a, userId, "user_id", 20);  // field, column, size
        dbo::field(a, name, "name");
    }
};

namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<User> : public dbo_default_traits {
            using IdType = std::string;
            
            static IdType invalidId() {
                return std::string();
            }
            
            static const char *surrogateIdField() { 
                return nullptr; 
            }
        };
    }
}
```

### Composite Primary Key
```cpp
struct Coordinate {
    int x, y;

    Coordinate() : x(-1), y(-1) { }
    Coordinate(int an_x, int an_y) : x(an_x), y(an_y) { }

    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const Coordinate& other) const {
        if (x < other.x) return true;
        else if (x == other.x) return y < other.y;
        else return false;
    }
};

std::ostream& operator<<(std::ostream& o, const Coordinate& c) {
    return o << "(" << c.x << ", " << c.y << ")";
}

// Persistence mapping for composite type
namespace Wt {
    namespace Dbo {
        template <class Action>
        void field(Action& action, Coordinate& coordinate,
                   const std::string& name, int size = -1) {
            field(action, coordinate.x, name + "_x");
            field(action, coordinate.y, name + "_y");
        }
    }
}

// Using composite key
class GeoTag {
public:
    Coordinate position;
    std::string name;

    template <class Action>
    void persist(Action& a) {
        dbo::id(a, position, "position");
        dbo::field(a, name, "name");
    }
};

namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<GeoTag> : public dbo_default_traits {
            using IdType = Coordinate;
            static IdType invalidId() { return Coordinate{}; }
            static const char *surrogateIdField() { return nullptr; }
        };
    }
}
```

### Foreign Key as Primary Key
```cpp
class UserInfo {
public:
    dbo::ptr<User> user;  // FK and PK
    std::string info;

    template<class Action>
    void persist(Action& a) {
        dbo::id(a, user, "user", dbo::OnDeleteCascade);
        dbo::field(a, info, "info");
    }
};

namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<UserInfo> : public dbo_default_traits {
            using IdType = ptr<User>;
            static IdType invalidId() { return ptr<User>{}; }
            static const char *surrogateIdField() { return nullptr; }
        };
    }
}
```

## Advanced Topics

### Custom SQL Value Types
```cpp
// For custom types, specialize sql_value_traits
namespace Wt {
    namespace Dbo {
        template<>
        struct sql_value_traits<MyCustomType> {
            static const bool specialized = true;
            static const char *type(SqlConnection *conn, int size);
            static void bind(const MyCustomType& v, SqlStatement *statement, int column, int size);
            static bool read(MyCustomType& v, SqlStatement *statement, int column, int size);
        };
    }
}
```

### Session Configuration
```cpp
void configureSession(dbo::Session& session) {
    // Set connection timeout
    session.setProperty("connection-timeout", "30");
    
    // Configure SQLite specific options
    auto sqlite = dynamic_cast<dbo::backend::Sqlite3*>(session.connection());
    if (sqlite) {
        sqlite->setProperty("synchronous", "normal");
        sqlite->setProperty("journal_mode", "wal");
        sqlite->setProperty("foreign_keys", "on");
    }
}
```

### Performance Optimization
```cpp
// Eager loading to avoid N+1 queries
auto users = session.find<User>().join("posts");

// Batch operations
{
    dbo::Transaction transaction(session);
    
    for (const auto& userData : largeDataSet) {
        auto user = session.addNew<User>();
        user.modify()->name = userData.name;
        // Don't commit inside loop
    }
    // Single commit for all operations
}

// Use direct binding for single-use queries
auto user = session.find<User, dbo::DirectBinding>()
    .where("id = ?").bind(userId);
```

### Error Handling
```cpp
void robustDatabaseOperation() {
    const int maxRetries = 3;
    int attempt = 0;
    
    while (attempt < maxRetries) {
        try {
            dbo::Transaction transaction(session);
            
            // Your database operations here
            
            transaction.commit();
            return; // Success
            
        } catch (dbo::StaleObjectException& e) {
            attempt++;
            if (attempt >= maxRetries) {
                throw; // Give up after max retries
            }
            
            // Reread stale objects
            for (auto& obj : modifiedObjects) {
                obj.reread();
            }
            
        } catch (dbo::Exception& e) {
            // Other Dbo exceptions
            std::cerr << "Database error: " << e.what() << std::endl;
            throw;
            
        } catch (std::exception& e) {
            // Generic exceptions
            std::cerr << "Unexpected error: " << e.what() << std::endl;
            throw;
        }
    }
}
```

### Database Schema Management
```cpp
// Create tables
session.createTables();

// Drop tables (careful!)
session.dropTables();

// Create individual table
session.createTable<User>();

// Execute raw SQL for migrations
{
    dbo::Transaction transaction(session);
    session.execute("ALTER TABLE user ADD COLUMN email VARCHAR(255)");
}
```

---

*This reference guide covers the essential aspects of Wt::Dbo for building database-driven applications. For complete API documentation, refer to the official Wt documentation.*
