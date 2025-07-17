# Database Integration with Wt::Dbo

Wt::Dbo is a powerful C++ Object-Relational Mapping (ORM) library that provides seamless database integration for Wt applications. This document covers comprehensive usage patterns, advanced features, and best practices.

## Table of Contents

1. [Introduction and Setup](#introduction-and-setup)
2. [Basic Mapping and Operations](#basic-mapping-and-operations)
3. [Relationships and Associations](#relationships-and-associations)
4. [Advanced Querying](#advanced-querying)
5. [Transaction Management](#transaction-management)
6. [Schema Customization](#schema-customization)
7. [Performance Optimization](#performance-optimization)
8. [Best Practices](#best-practices)

## Introduction and Setup

### Core Concepts

Wt::Dbo implements object-relational mapping using:

- **Database Objects (DBOs)**: C++ classes that map to database tables
- **Sessions**: Manage database connections and object lifecycle
- **Transactions**: Ensure ACID properties for database operations
- **Smart Pointers**: `dbo::ptr<T>` and `dbo::weak_ptr<T>` for object references
- **Collections**: `dbo::collection<T>` for query results

### Database Backend Support

Wt::Dbo supports multiple database backends:

```cpp
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/Dbo/backend/MSSQLServer.h>

namespace dbo = Wt::Dbo;

// SQLite - ideal for development and small applications
auto sqlite = std::make_unique<dbo::backend::Sqlite3>("app.db");

// PostgreSQL - production-ready with advanced features
auto postgres = std::make_unique<dbo::backend::Postgres>(
    "host=localhost user=app password=secret dbname=appdb");

// MySQL/MariaDB - widely supported
auto mysql = std::make_unique<dbo::backend::MySQL>(
    "appdb", "app", "secret", "localhost", 3306);

// Microsoft SQL Server - enterprise environments
auto mssql = std::make_unique<dbo::backend::MSSQLServer>(
    "Driver={ODBC Driver 17 for SQL Server};"
    "Server=localhost;Database=appdb;UID=app;PWD=secret;");
```

### Session Configuration

```cpp
class DatabaseManager {
public:
    DatabaseManager() {
        // Create connection
        auto connection = std::make_unique<dbo::backend::Sqlite3>("app.db");
        
        // Optional: Enable query logging
        connection->setProperty("show-queries", "true");
        
        // Set connection
        session_.setConnection(std::move(connection));
        
        // Map classes to tables
        session_.mapClass<User>("users");
        session_.mapClass<Post>("posts");
        session_.mapClass<Comment>("comments");
        
        // Create schema if needed
        try {
            session_.createTables();
            log("info") << "Database schema created";
        } catch (const dbo::Exception& e) {
            log("info") << "Using existing schema: " << e.what();
        }
    }
    
    dbo::Session& session() { return session_; }
    
private:
    dbo::Session session_;
};
```

## Basic Mapping and Operations

### Simple Class Mapping

```cpp
#include <Wt/Dbo/Types.h>
#include <Wt/WDateTime.h>

class User {
public:
    std::string username;
    std::string email;
    std::string hashedPassword;
    Wt::WDateTime createdAt;
    Wt::WDateTime lastLogin;
    bool isActive;
    int loginAttempts;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, username, "username");
        dbo::field(a, email, "email");
        dbo::field(a, hashedPassword, "password_hash");
        dbo::field(a, createdAt, "created_at");
        dbo::field(a, lastLogin, "last_login");
        dbo::field(a, isActive, "is_active");
        dbo::field(a, loginAttempts, "login_attempts");
    }
};

// Required for Wt::Dbo
DBO_EXTERN_TEMPLATES(User)
```

### Advanced Field Mapping

```cpp
class Product {
public:
    std::string name;
    std::string description;
    double price;
    int stockQuantity;
    std::string category;
    std::vector<std::string> tags;  // Will be serialized
    
    template<class Action>
    void persist(Action& a) {
        // Basic fields
        dbo::field(a, name, "name");
        dbo::field(a, description, "description");
        dbo::field(a, price, "price");
        dbo::field(a, stockQuantity, "stock_quantity");
        
        // Field with size constraint
        dbo::field(a, category, "category", 50);
        
        // Custom serialization for complex types
        std::string tagsStr;
        if (a.getsValue()) {
            // Deserializing from database
            dbo::field(a, tagsStr, "tags");
            deserializeTags(tagsStr);
        } else {
            // Serializing to database
            tagsStr = serializeTags();
            dbo::field(a, tagsStr, "tags");
        }
    }
    
private:
    void deserializeTags(const std::string& str) {
        tags.clear();
        std::istringstream iss(str);
        std::string tag;
        while (std::getline(iss, tag, ',')) {
            if (!tag.empty()) {
                tags.push_back(tag);
            }
        }
    }
    
    std::string serializeTags() const {
        std::ostringstream oss;
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) oss << ",";
            oss << tags[i];
        }
        return oss.str();
    }
};
```

### Basic CRUD Operations

```cpp
void demonstrateCRUD(dbo::Session& session) {
    // CREATE
    {
        dbo::Transaction transaction(session);
        
        // Method 1: Create and add
        auto user = std::make_unique<User>();
        user->username = "john_doe";
        user->email = "john@example.com";
        user->hashedPassword = hashPassword("secret123");
        user->createdAt = Wt::WDateTime::currentDateTime();
        user->isActive = true;
        user->loginAttempts = 0;
        
        dbo::ptr<User> userPtr = session.add(std::move(user));
        
        // Method 2: Direct creation
        auto newUser = session.addNew<User>();
        newUser.modify()->username = "jane_doe";
        newUser.modify()->email = "jane@example.com";
        newUser.modify()->hashedPassword = hashPassword("password456");
        newUser.modify()->createdAt = Wt::WDateTime::currentDateTime();
        newUser.modify()->isActive = true;
        newUser.modify()->loginAttempts = 0;
        
        transaction.commit();
    }
    
    // READ
    {
        dbo::Transaction transaction(session);
        
        // Find by ID
        dbo::ptr<User> user = session.find<User>().where("id = ?").bind(1);
        if (user) {
            std::cout << "Found user: " << user->username << std::endl;
        }
        
        // Find by unique field
        dbo::ptr<User> userByEmail = session.find<User>()
            .where("email = ?").bind("john@example.com");
        
        // Find all users
        dbo::collection<dbo::ptr<User>> allUsers = session.find<User>();
        std::cout << "Total users: " << allUsers.size() << std::endl;
        
        for (const auto& u : allUsers) {
            std::cout << "User: " << u->username 
                     << " (" << u->email << ")" << std::endl;
        }
    }
    
    // UPDATE
    {
        dbo::Transaction transaction(session);
        
        dbo::ptr<User> user = session.find<User>()
            .where("username = ?").bind("john_doe");
        
        if (user) {
            // Modify returns non-const reference
            user.modify()->lastLogin = Wt::WDateTime::currentDateTime();
            user.modify()->loginAttempts = 0;
            
            // Changes are automatically flushed
        }
        
        transaction.commit();
    }
    
    // DELETE
    {
        dbo::Transaction transaction(session);
        
        dbo::ptr<User> user = session.find<User>()
            .where("username = ?").bind("john_doe");
        
        if (user) {
            user.remove();
        }
        
        transaction.commit();
    }
}
```

## Relationships and Associations

### One-to-Many Relationships

```cpp
class Author {
public:
    std::string name;
    std::string bio;
    dbo::collection<dbo::ptr<Book>> books;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::field(a, bio, "bio");
        dbo::hasMany(a, books, dbo::ManyToOne, "author");
    }
};

class Book {
public:
    std::string title;
    std::string isbn;
    Wt::WDate publishDate;
    double price;
    dbo::ptr<Author> author;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, title, "title");
        dbo::field(a, isbn, "isbn");
        dbo::field(a, publishDate, "publish_date");
        dbo::field(a, price, "price");
        dbo::belongsTo(a, author, "author");
    }
};

// Working with one-to-many relationships
void demonstrateOneToMany(dbo::Session& session) {
    dbo::Transaction transaction(session);
    
    // Create author
    auto author = session.addNew<Author>();
    author.modify()->name = "J.K. Rowling";
    author.modify()->bio = "British author, best known for Harry Potter series";
    
    // Create books
    auto book1 = session.addNew<Book>();
    book1.modify()->title = "Harry Potter and the Philosopher's Stone";
    book1.modify()->isbn = "978-0439708180";
    book1.modify()->publishDate = Wt::WDate(1997, 6, 26);
    book1.modify()->price = 19.99;
    book1.modify()->author = author;  // Set relationship
    
    auto book2 = session.addNew<Book>();
    book2.modify()->title = "Harry Potter and the Chamber of Secrets";
    book2.modify()->isbn = "978-0439064873";
    book2.modify()->publishDate = Wt::WDate(1998, 7, 2);
    book2.modify()->price = 19.99;
    book2.modify()->author = author;
    
    // Access books through author
    std::cout << "Books by " << author->name << ":" << std::endl;
    for (const auto& book : author->books) {
        std::cout << "- " << book->title << " (" << book->publishDate.year() << ")" << std::endl;
    }
    
    transaction.commit();
}
```

### Many-to-Many Relationships

```cpp
class Student {
public:
    std::string firstName;
    std::string lastName;
    std::string email;
    dbo::collection<dbo::ptr<Course>> courses;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, firstName, "first_name");
        dbo::field(a, lastName, "last_name");
        dbo::field(a, email, "email");
        dbo::hasMany(a, courses, dbo::ManyToMany, "student_courses");
    }
};

class Course {
public:
    std::string code;
    std::string name;
    int credits;
    std::string description;
    dbo::collection<dbo::ptr<Student>> students;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, code, "code");
        dbo::field(a, name, "name");
        dbo::field(a, credits, "credits");
        dbo::field(a, description, "description");
        dbo::hasMany(a, students, dbo::ManyToMany, "student_courses");
    }
};

// Working with many-to-many relationships
void demonstrateManyToMany(dbo::Session& session) {
    dbo::Transaction transaction(session);
    
    // Create students
    auto student1 = session.addNew<Student>();
    student1.modify()->firstName = "Alice";
    student1.modify()->lastName = "Johnson";
    student1.modify()->email = "alice.johnson@university.edu";
    
    auto student2 = session.addNew<Student>();
    student2.modify()->firstName = "Bob";
    student2.modify()->lastName = "Smith";
    student2.modify()->email = "bob.smith@university.edu";
    
    // Create courses
    auto course1 = session.addNew<Course>();
    course1.modify()->code = "CS101";
    course1.modify()->name = "Introduction to Computer Science";
    course1.modify()->credits = 3;
    course1.modify()->description = "Basic programming concepts";
    
    auto course2 = session.addNew<Course>();
    course2.modify()->code = "MATH201";
    course2.modify()->name = "Calculus II";
    course2.modify()->credits = 4;
    course2.modify()->description = "Advanced calculus concepts";
    
    // Establish relationships
    student1.modify()->courses.insert(course1);
    student1.modify()->courses.insert(course2);
    student2.modify()->courses.insert(course1);
    
    // Query relationships
    std::cout << "Courses for " << student1->firstName << ":" << std::endl;
    for (const auto& course : student1->courses) {
        std::cout << "- " << course->code << ": " << course->name << std::endl;
    }
    
    std::cout << "Students in " << course1->code << ":" << std::endl;
    for (const auto& student : course1->students) {
        std::cout << "- " << student->firstName << " " << student->lastName << std::endl;
    }
    
    transaction.commit();
}
```

### One-to-One Relationships

```cpp
class UserProfile {
public:
    dbo::ptr<User> user;
    std::string biography;
    std::string website;
    std::string location;
    Wt::WDate birthDate;
    
    template<class Action>
    void persist(Action& a) {
        dbo::belongsTo(a, user, "user");
        dbo::field(a, biography, "biography");
        dbo::field(a, website, "website");
        dbo::field(a, location, "location");
        dbo::field(a, birthDate, "birth_date");
    }
};

// Updated User class to include profile
class User {
public:
    std::string username;
    std::string email;
    std::string hashedPassword;
    Wt::WDateTime createdAt;
    dbo::weak_ptr<UserProfile> profile;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, username, "username");
        dbo::field(a, email, "email");
        dbo::field(a, hashedPassword, "password_hash");
        dbo::field(a, createdAt, "created_at");
        dbo::hasOne(a, profile);
    }
};

void demonstrateOneToOne(dbo::Session& session) {
    dbo::Transaction transaction(session);
    
    // Create user
    auto user = session.addNew<User>();
    user.modify()->username = "alice_dev";
    user.modify()->email = "alice@developer.com";
    user.modify()->hashedPassword = hashPassword("secure_password");
    user.modify()->createdAt = Wt::WDateTime::currentDateTime();
    
    // Create profile
    auto profile = session.addNew<UserProfile>();
    profile.modify()->user = user;
    profile.modify()->biography = "Software developer with 5 years experience";
    profile.modify()->website = "https://alice-dev.com";
    profile.modify()->location = "San Francisco, CA";
    profile.modify()->birthDate = Wt::WDate(1990, 5, 15);
    
    // Access profile through user (note: involves a query)
    if (user->profile) {
        std::cout << user->username << "'s bio: " << user->profile->biography << std::endl;
    }
    
    transaction.commit();
}
```

## Advanced Querying

### Complex Queries

```cpp
void demonstrateAdvancedQueries(dbo::Session& session) {
    dbo::Transaction transaction(session);
    
    // Query with WHERE conditions
    auto activeUsers = session.find<User>()
        .where("is_active = ? AND login_attempts < ?")
        .bind(true)
        .bind(3);
    
    // Query with ORDER BY
    auto sortedUsers = session.find<User>()
        .where("is_active = ?")
        .bind(true)
        .orderBy("created_at DESC")
        .limit(10);
    
    // Query with LIKE
    auto searchResults = session.find<User>()
        .where("username LIKE ? OR email LIKE ?")
        .bind("%john%")
        .bind("%john%");
    
    // Query with date ranges
    auto recentUsers = session.find<User>()
        .where("created_at > ?")
        .bind(Wt::WDateTime::currentDateTime().addDays(-30));
    
    // Aggregate queries
    int userCount = session.query<int>("SELECT COUNT(*) FROM users")
        .where("is_active = ?")
        .bind(true);
    
    double avgLoginAttempts = session.query<double>(
        "SELECT AVG(login_attempts) FROM users")
        .where("is_active = ?")
        .bind(true);
    
    // Join queries
    auto usersWithProfiles = session.query<dbo::ptr<User>>(
        "SELECT u FROM users u JOIN user_profile p ON u.id = p.user_id")
        .where("p.location LIKE ?")
        .bind("%California%");
    
    // Custom result types
    struct UserSummary {
        std::string username;
        std::string email;
        int bookCount;
        
        UserSummary() = default;
        UserSummary(const std::string& u, const std::string& e, int c)
            : username(u), email(e), bookCount(c) {}
    };
    
    auto summaries = session.query<UserSummary>(
        "SELECT u.username, u.email, COUNT(b.id) "
        "FROM users u LEFT JOIN books b ON u.id = b.author_id "
        "GROUP BY u.id, u.username, u.email");
    
    for (const auto& summary : summaries) {
        std::cout << summary.username << " has " << summary.bookCount << " books" << std::endl;
    }
}

// Enable custom result type
namespace Wt {
namespace Dbo {
template<>
struct query_result_traits<UserSummary> {
    typedef boost::tuple<std::string, std::string, int> result;
    
    static UserSummary load(const result& r) {
        return UserSummary(boost::get<0>(r), boost::get<1>(r), boost::get<2>(r));
    }
};
}
}
```

### Dynamic Query Building

```cpp
class UserQueryBuilder {
public:
    UserQueryBuilder(dbo::Session& session) : session_(session) {}
    
    UserQueryBuilder& active(bool isActive = true) {
        conditions_.push_back("is_active = ?");
        parameters_.push_back(isActive);
        return *this;
    }
    
    UserQueryBuilder& usernameContains(const std::string& pattern) {
        conditions_.push_back("username LIKE ?");
        parameters_.push_back("%" + pattern + "%");
        return *this;
    }
    
    UserQueryBuilder& createdAfter(const Wt::WDateTime& date) {
        conditions_.push_back("created_at > ?");
        parameters_.push_back(date);
        return *this;
    }
    
    UserQueryBuilder& orderBy(const std::string& column, bool ascending = true) {
        orderBy_ = column + (ascending ? " ASC" : " DESC");
        return *this;
    }
    
    UserQueryBuilder& limit(int count) {
        limit_ = count;
        return *this;
    }
    
    dbo::collection<dbo::ptr<User>> execute() {
        auto query = session_.find<User>();
        
        if (!conditions_.empty()) {
            std::string whereClause;
            for (size_t i = 0; i < conditions_.size(); ++i) {
                if (i > 0) whereClause += " AND ";
                whereClause += conditions_[i];
            }
            query = query.where(whereClause);
            
            for (const auto& param : parameters_) {
                query = bindParameter(query, param);
            }
        }
        
        if (!orderBy_.empty()) {
            query = query.orderBy(orderBy_);
        }
        
        if (limit_ > 0) {
            query = query.limit(limit_);
        }
        
        return query;
    }
    
private:
    dbo::Session& session_;
    std::vector<std::string> conditions_;
    std::vector<boost::any> parameters_;
    std::string orderBy_;
    int limit_ = 0;
    
    template<typename T>
    dbo::Query<dbo::ptr<User>> bindParameter(dbo::Query<dbo::ptr<User>> query, const T& param) {
        return query.bind(boost::any_cast<T>(param));
    }
};

// Usage
void demonstrateDynamicQueries(dbo::Session& session) {
    dbo::Transaction transaction(session);
    
    auto results = UserQueryBuilder(session)
        .active(true)
        .usernameContains("john")
        .createdAfter(Wt::WDateTime::currentDateTime().addDays(-90))
        .orderBy("created_at", false)
        .limit(20)
        .execute();
    
    for (const auto& user : results) {
        std::cout << "Found: " << user->username << std::endl;
    }
}
```

## Transaction Management

### Basic Transactions

```cpp
void demonstrateTransactions(dbo::Session& session) {
    // Simple transaction
    {
        dbo::Transaction transaction(session);
        
        auto user = session.addNew<User>();
        user.modify()->username = "transactional_user";
        user.modify()->email = "trans@example.com";
        
        // Transaction commits automatically when going out of scope
    }
    
    // Manual commit/rollback
    {
        dbo::Transaction transaction(session);
        
        try {
            auto user = session.addNew<User>();
            user.modify()->username = "manual_user";
            user.modify()->email = "manual@example.com";
            
            // Perform some operation that might fail
            if (someCondition()) {
                throw std::runtime_error("Something went wrong");
            }
            
            transaction.commit();
        } catch (const std::exception& e) {
            // Transaction automatically rolls back
            log("error") << "Transaction failed: " << e.what();
        }
    }
    
    // Nested transactions (using savepoints if supported)
    {
        dbo::Transaction outerTransaction(session);
        
        auto user = session.addNew<User>();
        user.modify()->username = "outer_user";
        
        {
            dbo::Transaction innerTransaction(session);
            
            auto profile = session.addNew<UserProfile>();
            profile.modify()->user = user;
            profile.modify()->biography = "Inner transaction";
            
            // Inner transaction commits
        }
        
        // Outer transaction commits
    }
}
```

### Optimistic Locking

```cpp
void demonstrateOptimisticLocking(dbo::Session& session) {
    try {
        dbo::Transaction transaction(session);
        
        // Load user in one session
        auto user1 = session.find<User>().where("id = ?").bind(1);
        
        // Simulate another session modifying the same user
        // This would increment the version field
        
        // Try to modify user1
        user1.modify()->loginAttempts++;
        
        // If version changed, StaleObjectException will be thrown
        transaction.commit();
        
    } catch (const dbo::StaleObjectException& e) {
        log("warning") << "Concurrent modification detected: " << e.what();
        
        // Strategy 1: Retry with fresh data
        retryWithFreshData(session);
        
        // Strategy 2: Merge changes
        // mergeChanges(session);
        
        // Strategy 3: Abort and notify user
        // notifyUser("Data was modified by another user");
    }
}

void retryWithFreshData(dbo::Session& session) {
    dbo::Transaction transaction(session);
    
    // Reload the object with current data
    auto user = session.find<User>().where("id = ?").bind(1);
    if (user) {
        user.reread();  // Force reload from database
        user.modify()->loginAttempts++;
        transaction.commit();
    }
}
```

### Connection Pool Management

```cpp
class DatabaseConnectionPool {
public:
    DatabaseConnectionPool(const std::string& connectionString, int poolSize = 10) {
        pool_ = std::make_unique<dbo::FixedSqlConnectionPool>(
            [connectionString]() {
                return std::make_unique<dbo::backend::Postgres>(connectionString);
            },
            poolSize
        );
    }
    
    class ScopedSession {
    public:
        ScopedSession(DatabaseConnectionPool& pool) : session_(pool.createSession()) {}
        
        dbo::Session& operator*() { return *session_; }
        dbo::Session* operator->() { return session_.get(); }
        
    private:
        std::unique_ptr<dbo::Session> session_;
    };
    
    ScopedSession getSession() {
        return ScopedSession(*this);
    }
    
private:
    std::unique_ptr<dbo::SqlConnectionPool> pool_;
    
    std::unique_ptr<dbo::Session> createSession() {
        auto session = std::make_unique<dbo::Session>();
        session->setConnectionPool(*pool_);
        
        // Map classes
        session->mapClass<User>("users");
        session->mapClass<UserProfile>("user_profiles");
        
        return session;
    }
};

// Usage with connection pool
void useConnectionPool() {
    DatabaseConnectionPool pool("host=localhost dbname=app user=app", 20);
    
    // Each operation gets a session from the pool
    {
        auto session = pool.getSession();
        dbo::Transaction transaction(*session);
        
        auto users = session->find<User>();
        for (const auto& user : users) {
            std::cout << user->username << std::endl;
        }
    }
    // Session automatically returned to pool
}
```

## Schema Customization

### Custom Primary Keys

```cpp
// Natural primary key
class Product {
public:
    std::string sku;  // Natural key
    std::string name;
    double price;
    
    template<class Action>
    void persist(Action& a) {
        dbo::id(a, sku, "sku", 20);  // SKU as primary key, max 20 chars
        dbo::field(a, name, "name");
        dbo::field(a, price, "price");
    }
};

// Customize dbo_traits for natural key
namespace Wt {
namespace Dbo {
template<>
struct dbo_traits<Product> : public dbo_default_traits {
    using IdType = std::string;
    
    static IdType invalidId() {
        return std::string();
    }
    
    static const char* surrogateIdField() {
        return nullptr;  // No auto-increment ID
    }
};
}
}

// Composite primary key
struct OrderLineKey {
    long orderId;
    long productId;
    
    OrderLineKey() = default;
    OrderLineKey(long oid, long pid) : orderId(oid), productId(pid) {}
    
    bool operator==(const OrderLineKey& other) const {
        return orderId == other.orderId && productId == other.productId;
    }
    
    bool operator<(const OrderLineKey& other) const {
        if (orderId < other.orderId) return true;
        if (orderId == other.orderId) return productId < other.productId;
        return false;
    }
};

std::ostream& operator<<(std::ostream& o, const OrderLineKey& key) {
    return o << "(" << key.orderId << "," << key.productId << ")";
}

// Custom field mapping for composite key
namespace Wt {
namespace Dbo {
template<class Action>
void field(Action& action, OrderLineKey& key, const std::string& name, int size = -1) {
    field(action, key.orderId, name + "_order_id");
    field(action, key.productId, name + "_product_id");
}
}
}

class OrderLine {
public:
    OrderLineKey id;
    int quantity;
    double unitPrice;
    
    template<class Action>
    void persist(Action& a) {
        dbo::id(a, id, "composite_key");
        dbo::field(a, quantity, "quantity");
        dbo::field(a, unitPrice, "unit_price");
    }
};

// Customize traits for composite key
namespace Wt {
namespace Dbo {
template<>
struct dbo_traits<OrderLine> : public dbo_default_traits {
    using IdType = OrderLineKey;
    
    static IdType invalidId() {
        return OrderLineKey();
    }
    
    static const char* surrogateIdField() {
        return nullptr;
    }
};
}
}
```

### Version Field Customization

```cpp
// Disable versioning for read-only tables
class LogEntry {
public:
    Wt::WDateTime timestamp;
    std::string level;
    std::string message;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, timestamp, "timestamp");
        dbo::field(a, level, "level");
        dbo::field(a, message, "message");
    }
};

namespace Wt {
namespace Dbo {
template<>
struct dbo_traits<LogEntry> : public dbo_default_traits {
    static const char* versionField() {
        return nullptr;  // Disable version field
    }
};
}
}

// Custom version field name
class Document {
public:
    std::string title;
    std::string content;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, title, "title");
        dbo::field(a, content, "content");
    }
};

namespace Wt {
namespace Dbo {
template<>
struct dbo_traits<Document> : public dbo_default_traits {
    static const char* versionField() {
        return "revision";  // Custom version field name
    }
};
}
}
```

### Foreign Key Constraints

```cpp
class Order {
public:
    Wt::WDateTime orderDate;
    std::string status;
    dbo::ptr<User> customer;
    dbo::collection<dbo::ptr<OrderItem>> items;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, orderDate, "order_date");
        dbo::field(a, status, "status");
        
        // Foreign key with constraints
        dbo::belongsTo(a, customer, "customer", 
                      dbo::OnDeleteCascade | dbo::OnUpdateCascade);
        
        dbo::hasMany(a, items, dbo::ManyToOne, "order");
    }
};

class OrderItem {
public:
    int quantity;
    double price;
    dbo::ptr<Order> order;
    dbo::ptr<Product> product;
    
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, quantity, "quantity");
        dbo::field(a, price, "price");
        
        // Cascade delete when order is deleted
        dbo::belongsTo(a, order, "order", dbo::OnDeleteCascade);
        
        // Set null when product is deleted
        dbo::belongsTo(a, product, "product", dbo::OnDeleteSetNull);
    }
};
```

## Performance Optimization

### Query Optimization

```cpp
class OptimizedQueries {
public:
    // Use prepared statements for repeated queries
    static void preparedStatements(dbo::Session& session) {
        // Query is prepared once, reused multiple times
        auto query = session.find<User>().where("email = ?");
        
        std::vector<std::string> emails = {"user1@example.com", "user2@example.com"};
        
        for (const auto& email : emails) {
            auto user = query.bind(email);
            if (user) {
                processUser(user);
            }
        }
    }
    
    // Batch operations
    static void batchInserts(dbo::Session& session) {
        dbo::Transaction transaction(session);
        
        // Insert multiple records in one transaction
        for (int i = 0; i < 1000; ++i) {
            auto user = session.addNew<User>();
            user.modify()->username = "user_" + std::to_string(i);
            user.modify()->email = "user" + std::to_string(i) + "@example.com";
            user.modify()->createdAt = Wt::WDateTime::currentDateTime();
            
            // Flush periodically to avoid memory buildup
            if (i % 100 == 0) {
                session.flush();
            }
        }
        
        transaction.commit();
    }
    
    // Eager loading to avoid N+1 queries
    static void eagerLoading(dbo::Session& session) {
        dbo::Transaction transaction(session);
        
        // Bad: N+1 query problem
        auto users = session.find<User>();
        for (const auto& user : users) {
            // Each access to profile triggers a separate query
            if (user->profile) {
                std::cout << user->profile->biography << std::endl;
            }
        }
        
        // Better: Use JOIN to load related data
        auto usersWithProfiles = session.query<dbo::ptr<User>>(
            "SELECT u FROM users u LEFT JOIN user_profile p ON u.id = p.user_id");
        
        for (const auto& user : usersWithProfiles) {
            if (user->profile) {
                std::cout << user->profile->biography << std::endl;
            }
        }
    }
    
    // Pagination for large result sets
    static void paginatedQueries(dbo::Session& session, int page, int pageSize) {
        dbo::Transaction transaction(session);
        
        int offset = page * pageSize;
        
        auto results = session.find<User>()
            .where("is_active = ?")
            .bind(true)
            .orderBy("created_at DESC")
            .limit(pageSize)
            .offset(offset);
        
        // Also get total count for pagination info
        int totalCount = session.query<int>("SELECT COUNT(*) FROM users")
            .where("is_active = ?")
            .bind(true);
        
        processPaginatedResults(results, totalCount, page, pageSize);
    }
    
private:
    static void processUser(dbo::ptr<User> user) { /* Process user */ }
    static void processPaginatedResults(const dbo::collection<dbo::ptr<User>>& results,
                                      int total, int page, int pageSize) { /* Process results */ }
};
```

### Connection Management

```cpp
class PerformanceOptimizedSession {
public:
    PerformanceOptimizedSession() {
        setupOptimizedConnection();
    }
    
private:
    void setupOptimizedConnection() {
        auto connection = std::make_unique<dbo::backend::Postgres>(
            "host=localhost dbname=app user=app password=secret");
        
        // Connection-specific optimizations
        connection->setProperty("synchronous_commit", "off");  // PostgreSQL
        connection->setProperty("wal_buffers", "16MB");
        connection->setProperty("checkpoint_segments", "32");
        
        // Connection pooling settings
        connection->setProperty("max_connections", "100");
        connection->setProperty("shared_buffers", "256MB");
        
        session_.setConnection(std::move(connection));
        
        // Map classes with optimizations
        session_.mapClass<User>("users");
        session_.mapClass<UserProfile>("user_profiles");
        
        // Create indexes for frequently queried fields
        createOptimizedIndexes();
    }
    
    void createOptimizedIndexes() {
        try {
            dbo::Transaction transaction(session_);
            
            // Create indexes for common queries
            session_.execute("CREATE INDEX CONCURRENTLY IF NOT EXISTS "
                           "idx_users_email ON users(email)");
            
            session_.execute("CREATE INDEX CONCURRENTLY IF NOT EXISTS "
                           "idx_users_active_created ON users(is_active, created_at)");
            
            session_.execute("CREATE INDEX CONCURRENTLY IF NOT EXISTS "
                           "idx_user_profile_user_id ON user_profiles(user_id)");
            
            transaction.commit();
        } catch (const dbo::Exception& e) {
            log("warning") << "Index creation failed: " << e.what();
        }
    }
    
    dbo::Session session_;
};
```

### Caching Strategies

```cpp
template<typename T>
class CachedRepository {
public:
    CachedRepository(dbo::Session& session, size_t maxCacheSize = 1000)
        : session_(session), maxCacheSize_(maxCacheSize) {}
    
    dbo::ptr<T> findById(typename dbo::dbo_traits<T>::IdType id) {
        // Check cache first
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            // Move to front (LRU)
            accessOrder_.splice(accessOrder_.begin(), accessOrder_, it->second.second);
            return it->second.first;
        }
        
        // Load from database
        dbo::Transaction transaction(session_);
        auto object = session_.load<T>(id);
        
        if (object) {
            addToCache(id, object);
        }
        
        return object;
    }
    
    void invalidate(typename dbo::dbo_traits<T>::IdType id) {
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            accessOrder_.erase(it->second.second);
            cache_.erase(it);
        }
    }
    
    void clear() {
        cache_.clear();
        accessOrder_.clear();
    }
    
private:
    using IdType = typename dbo::dbo_traits<T>::IdType;
    using CacheEntry = std::pair<dbo::ptr<T>, 
                                std::list<IdType>::iterator>;
    
    dbo::Session& session_;
    size_t maxCacheSize_;
    std::unordered_map<IdType, CacheEntry> cache_;
    std::list<IdType> accessOrder_;  // LRU order
    
    void addToCache(IdType id, dbo::ptr<T> object) {
        // Remove oldest if at capacity
        if (cache_.size() >= maxCacheSize_) {
            IdType oldest = accessOrder_.back();
            accessOrder_.pop_back();
            cache_.erase(oldest);
        }
        
        // Add to front
        accessOrder_.push_front(id);
        cache_[id] = std::make_pair(object, accessOrder_.begin());
    }
};

// Usage
void demonstrateCaching(dbo::Session& session) {
    CachedRepository<User> userRepo(session, 500);
    
    // First access loads from database
    auto user1 = userRepo.findById(1);
    
    // Second access uses cache
    auto user1_cached = userRepo.findById(1);
    
    // Invalidate when data changes
    {
        dbo::Transaction transaction(session);
        user1.modify()->lastLogin = Wt::WDateTime::currentDateTime();
        transaction.commit();
    }
    userRepo.invalidate(1);  // Remove from cache
}
```

## Best Practices

### 1. Session and Transaction Management

```cpp
class BestPracticeExample {
public:
    // Good: Use RAII for transaction management
    void goodTransactionPattern(dbo::Session& session) {
        dbo::Transaction transaction(session);
        
        try {
            // Perform database operations
            auto user = session.addNew<User>();
            user.modify()->username = "good_user";
            
            // Transaction commits automatically
        } catch (const std::exception& e) {
            // Transaction rolls back automatically
            log("error") << "Transaction failed: " << e.what();
            throw;
        }
    }
    
    // Good: Keep transactions short
    void shortTransactions(dbo::Session& session) {
        // Prepare data outside transaction
        std::vector<UserData> userData = prepareUserData();
        
        // Short transaction for database operations
        {
            dbo::Transaction transaction(session);
            
            for (const auto& data : userData) {
                auto user = session.addNew<User>();
                user.modify()->username = data.username;
                user.modify()->email = data.email;
            }
            
            // Quick commit
        }
    }
    
    // Good: Handle concurrent modifications gracefully
    void handleConcurrency(dbo::Session& session) {
        const int maxRetries = 3;
        int retryCount = 0;
        
        while (retryCount < maxRetries) {
            try {
                dbo::Transaction transaction(session);
                
                auto user = session.find<User>().where("id = ?").bind(1);
                if (user) {
                    user.modify()->loginAttempts++;
                    transaction.commit();
                    break;  // Success
                }
                
            } catch (const dbo::StaleObjectException& e) {
                retryCount++;
                log("warning") << "Retry " << retryCount << " due to concurrent modification";
                
                if (retryCount >= maxRetries) {
                    throw;  // Give up
                }
                
                // Brief delay before retry
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * retryCount));
            }
        }
    }
    
private:
    struct UserData {
        std::string username;
        std::string email;
    };
    
    std::vector<UserData> prepareUserData() {
        // Prepare data outside database transaction
        return {{"user1", "user1@example.com"}, {"user2", "user2@example.com"}};
    }
};
```

### 2. Query Optimization

```cpp
class QueryOptimizationGuide {
public:
    // Good: Use specific queries instead of loading all data
    void efficientQueries(dbo::Session& session) {
        dbo::Transaction transaction(session);
        
        // Bad: Load all users then filter in memory
        // auto allUsers = session.find<User>();
        // for (const auto& user : allUsers) {
        //     if (user->isActive) { processUser(user); }
        // }
        
        // Good: Filter in database
        auto activeUsers = session.find<User>().where("is_active = ?").bind(true);
        for (const auto& user : activeUsers) {
            processUser(user);
        }
    }
    
    // Good: Use projections for partial data
    void useProjections(dbo::Session& session) {
        dbo::Transaction transaction(session);
        
        // Only load needed fields
        auto userSummaries = session.query<boost::tuple<std::string, std::string>>(
            "SELECT username, email FROM users WHERE is_active = ?")
            .bind(true);
        
        for (const auto& summary : userSummaries) {
            std::cout << boost::get<0>(summary) << ": " << boost::get<1>(summary) << std::endl;
        }
    }
    
    // Good: Use appropriate indexes
    void designForIndexes(dbo::Session& session) {
        // Design queries to use indexes effectively
        
        // Good: Uses index on (is_active, created_at)
        auto recentActiveUsers = session.find<User>()
            .where("is_active = ? AND created_at > ?")
            .bind(true)
            .bind(Wt::WDateTime::currentDateTime().addDays(-30))
            .orderBy("created_at DESC");
        
        // Consider creating composite indexes for common query patterns
        createCompositeIndexes(session);
    }
    
private:
    void processUser(dbo::ptr<User> user) { /* Process user */ }
    
    void createCompositeIndexes(dbo::Session& session) {
        try {
            session.execute("CREATE INDEX IF NOT EXISTS idx_user_active_created "
                          "ON users(is_active, created_at) WHERE is_active = true");
        } catch (const dbo::Exception& e) {
            log("warning") << "Index creation failed: " << e.what();
        }
    }
};
```

### 3. Error Handling and Logging

```cpp
class DatabaseErrorHandling {
public:
    // Comprehensive error handling
    bool performDatabaseOperation(dbo::Session& session) {
        try {
            dbo::Transaction transaction(session);
            
            // Database operations
            auto user = session.addNew<User>();
            user.modify()->username = "test_user";
            user.modify()->email = "test@example.com";
            
            transaction.commit();
            return true;
            
        } catch (const dbo::StaleObjectException& e) {
            log("warning") << "Concurrent modification detected: " << e.what();
            return handleConcurrentModification(session);
            
        } catch (const dbo::Exception& e) {
            log("error") << "Database error: " << e.what();
            return false;
            
        } catch (const std::exception& e) {
            log("error") << "Unexpected error: " << e.what();
            return false;
        }
    }
    
    // Proper logging for database operations
    void loggedDatabaseOperations(dbo::Session& session) {
        log("info") << "Starting user creation process";
        
        try {
            dbo::Transaction transaction(session);
            
            log("debug") << "Creating new user";
            auto user = session.addNew<User>();
            user.modify()->username = "logged_user";
            
            log("debug") << "Committing transaction";
            transaction.commit();
            
            log("info") << "User created successfully with ID: " << user.id();
            
        } catch (const std::exception& e) {
            log("error") << "Failed to create user: " << e.what();
            throw;
        }
    }
    
private:
    bool handleConcurrentModification(dbo::Session& session) {
        // Implement retry logic or conflict resolution
        return false;
    }
};
```

### 4. Testing Database Code

```cpp
class DatabaseTesting {
public:
    // Use in-memory database for testing
    static std::unique_ptr<dbo::Session> createTestSession() {
        auto session = std::make_unique<dbo::Session>();
        
        // SQLite in-memory database
        auto connection = std::make_unique<dbo::backend::Sqlite3>(":memory:");
        session->setConnection(std::move(connection));
        
        // Map test classes
        session->mapClass<User>("users");
        session->mapClass<UserProfile>("user_profiles");
        
        // Create test schema
        session->createTables();
        
        return session;
    }
    
    // Test fixture for database tests
    class DatabaseTestFixture {
    public:
        DatabaseTestFixture() : session_(createTestSession()) {
            setupTestData();
        }
        
        dbo::Session& session() { return *session_; }
        
    private:
        std::unique_ptr<dbo::Session> session_;
        
        void setupTestData() {
            dbo::Transaction transaction(*session_);
            
            // Create test users
            auto user1 = session_->addNew<User>();
            user1.modify()->username = "test_user_1";
            user1.modify()->email = "test1@example.com";
            user1.modify()->isActive = true;
            
            auto user2 = session_->addNew<User>();
            user2.modify()->username = "test_user_2";
            user2.modify()->email = "test2@example.com";
            user2.modify()->isActive = false;
            
            transaction.commit();
        }
    };
    
    // Example unit test
    void testUserQueries() {
        DatabaseTestFixture fixture;
        auto& session = fixture.session();
        
        dbo::Transaction transaction(session);
        
        // Test active user query
        auto activeUsers = session.find<User>().where("is_active = ?").bind(true);
        assert(activeUsers.size() == 1);
        
        // Test user lookup by email
        auto user = session.find<User>().where("email = ?").bind("test1@example.com");
        assert(user);
        assert(user->username == "test_user_1");
        
        transaction.commit();
    }
};
```

---

*This comprehensive guide covers the essential aspects of database integration with Wt::Dbo. For authentication integration, see [Authentication System](authentication-system.md).*
