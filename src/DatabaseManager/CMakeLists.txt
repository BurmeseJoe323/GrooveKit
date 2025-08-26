# sqlite
add_library(sqlite3 STATIC
        ${CMAKE_SOURCE_DIR}/third-party/database/sqlite3.c
        ${CMAKE_SOURCE_DIR}/third-party/database/sqlite3.h
)
target_include_directories(sqlite3 PUBLIC ${CMAKE_SOURCE_DIR}/third-party/database)

add_library(database_manager)
target_sources(database_manager PRIVATE DatabaseManager.cpp PUBLIC DatabaseManager.h)
target_include_directories(database_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(database_manager PUBLIC sqlite3)
