//
// Created by ikera on 4/15/2025.
//

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


// DatabaseManager.h
#pragma once
#include "../../third-party/database/sqlite3.h"
#include <string>
#include <vector>
#include <unordered_map>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool initialize();
    sqlite3* getDatabase() const { return db; }

    /// This method will be used to add a Tutorial to the database.
    ///     Note: This can be used if you want to create your own tutorial,
    ///     just link to a button when clicked, create a new tutorial, calling this
    ///     method with the name of the tutorial you chose.
    /// @param name The name of the new tutorial being added.
    /// @return Returns true, if successful query, false other-wise
    ///     NOTE: if name is a duplicate, query will also fail
    bool addTutorial(const std::string &name);

    /// This method is used to track whether a user has completed a tutorial
    /// @param tutorialName The name of the tutorial that the user has completed.
    ///     Note: The tutorial must exist as it is a foreign key referencing the Tutorials table
    /// @param userName The user-name of the user who completed the tutorial
    /// @return Returns true, if successful query, false other-wise
    ///     NOTE: if name is a duplicate, query will also fail
    bool addCompletedTutorial(const std::string &tutorialName, const std::string &userName);

    /// This method is used to store UserNames into the db bundled with the app
    /// @param name The username being passed in by the user
    /// @return Returns true, if successful query, false other-wise
    ///     NOTE: if name is a duplicate, query will also fail
    bool addUser(const std::string &name);

    /// This method will select and return true if the select was a success and false o.w.
    /// @userName The user's userName in the current session.
    /// @return Returns a list of all of the completed tutorials based on the given userName
    bool selectCompletedTutorials(const std::string &userName);

private:
    sqlite3* db = nullptr;
    std::string userDbPath;
    std::string databasePath = "../third-party/database/";
    char *zErrMsg = 0;
};

#endif //DATABASEMANAGER_H
