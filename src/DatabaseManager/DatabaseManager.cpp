//
// Created by ikera on 4/15/2025.
//

#include "DatabaseManager.h"
#include <filesystem>
#include <iostream>

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

    return true;
}