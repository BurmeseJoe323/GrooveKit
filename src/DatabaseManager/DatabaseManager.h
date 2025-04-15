//
// Created by ikera on 4/15/2025.
//

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


// DatabaseManager.h
#pragma once
#include "../../third-party/database/sqlite3.h"
#include <string>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool initialize();
    sqlite3* getDatabase() const { return db; }

private:
    sqlite3* db = nullptr;
    std::string userDbPath;
    std::string databasePath = "../third-party/database/";
};

#endif //DATABASEMANAGER_H
