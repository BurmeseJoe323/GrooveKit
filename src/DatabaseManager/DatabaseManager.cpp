//
// Created by ikera on 4/15/2025.
//

#include "DatabaseManager.h"
#include <filesystem>
#include <iostream>

/// Call backs are used mainly when using a select query.
/// For now this call back just prints and will be useful for
/// debug purposes, but in the future we may want to store results
/// so that we can use it later.
/// @param data 
/// @param argc 
/// @param argv 
/// @param azColName 
/// @return 
static int callback(void* data, int argc, char** argv, char** azColName)
{
    int i;
    fprintf(stderr, "%s: ", (const char*)data);

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager() {
    if (db) sqlite3_close(db);
}

bool DatabaseManager::initialize() {
    const std::string defaultDb = databasePath + "groovekit_template.db";
    userDbPath = databasePath + "groovekit.db";

    // Check if template exists
    if (!std::filesystem::exists(defaultDb)) {
        return false;
    }

    // Only copy if user DB doesn't exist
    if (!std::filesystem::exists(userDbPath)) {
        try {
            // Ensure directory exists
            std::filesystem::create_directories(std::filesystem::path(userDbPath).parent_path());

            // Perform the copy
            std::filesystem::copy(defaultDb, userDbPath);

            // Verify the copy worked
            if (!std::filesystem::exists(userDbPath)) {
                return false;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            return false;
        }
    }

    // Open database with additional flags
    int result = sqlite3_open_v2(userDbPath.c_str(), &db,
                               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                               nullptr);

    if (result != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }

    const char *user = "User1";
    addUser(user);
    return true;
}

bool DatabaseManager::addUser(const std::string &name) {
    sqlite3_stmt *stmt;

    // The query below has a ? in it. This is to prevent sql injection attacks, replacing ? with the name
    const char *query = "INSERT OR IGNORE INTO Users(userName) VALUES(?);";

    // Prepares the SQL statement.
    // 'db' is the database
    // 'query' is the SQL query we provide
    // '-1' is automatically determining the query length
    // '&stmt' is the output parameter for the prepared statement
    // 'nullptr' is unused
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind the username.
    // 'stmt' is the prepared statement.
    // '1' is the index of the parameter,
    //      - For this case, 1 represents the first ?
    // 'name.c_str()' the name converted ton const char * and it's what is being binded
    //`SQLITE_TRANSIENT` SQLite should make its own copy of the string.
    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    // This will finally execute the prepared statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}


bool DatabaseManager::addTutorial(const std::string &name) {
    sqlite3_stmt *stmt;

    // The query below has a ? in it. This is to prevent sql injection attacks, replacing ? with the name
    const char *query = "INSERT OR IGNORE INTO Tutorials(Tutorial) VALUES(?);";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    // This will finally execute the prepared statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool DatabaseManager::addCompletedTutorial(const std::string &tutorialName, const std::string &userName) {
    sqlite3_stmt *stmt;

    // The query below has a ? in it. This is to prevent sql injection attacks, replacing ? with the name
    const char *query = "INSERT OR IGNORE INTO Completed(tutorialName, userName) VALUES(?, ?);";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Setting up bind for the first ?, which in this case is the tutorialName
    rc = sqlite3_bind_text(stmt, 1, tutorialName.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    //Setting up bind for the second ?, or in this case, the userName.
    rc = sqlite3_bind_text(stmt, 2, userName.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    // This will finally execute the prepared statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<const unsigned char*> DatabaseManager::selectCompletedTutorials(const std::string &userName) {
    std::vector<const unsigned char*> completedTutorials;
    int rc;
    sqlite3_stmt *stmt;
    const char* sql = "SELECT tutorialName FROM COMPLETED WHERE userName = ?;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return completedTutorials;
    }

    // This binds the username parameter into our sql statement.
    sqlite3_bind_text(stmt, 1, userName.c_str(), -1, SQLITE_STATIC);

    // Execute the statement and process results
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* tutorialName = sqlite3_column_text(stmt, 0);
        if (tutorialName == nullptr) {
            std::cout << "Something Went Wrong. tutorialName may be null" << std::endl;
            break;
        }
        std::cout << "Tutorial Name: " << tutorialName << std::endl;

        completedTutorials.push_back(tutorialName);

    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Error executing statement" << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    std::cout << "Operation OK!" << std::endl;
    return completedTutorials;
}

