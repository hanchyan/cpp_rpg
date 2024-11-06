#include <iostream>
#include <sqlite3.h>

// Function to create or open the database
int openDatabase(sqlite3** db) {
    int rc = sqlite3_open("game_data.db", db); // Open or create database
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(*db) << std::endl;
        return rc;
    }
    return 0;
}

// Function to create a table
void createTable(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS actions ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "action TEXT NOT NULL, "
                      "timestamp TEXT NOT NULL);";
    
    char* errMsg;
    int rc = sqlite3_exec(db, sql, nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Table created successfully." << std::endl;
    }
}

// Function to insert action into the database
void insertAction(sqlite3* db, const std::string& action, const std::string& timestamp) {
    std::string sql = "INSERT INTO actions (action, timestamp) VALUES ('" + action + "', '" + timestamp + "');";
    char* errMsg;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Action recorded: " << action << " at " << timestamp << std::endl;
    }
}

int main() {
    sqlite3* db;
    if (openDatabase(&db) != 0) {
        return 1;
    }
    
    createTable(db);

    // Sample actions with timestamps
    std::string action1 = "Attack";
    std::string timestamp1 = "2024-11-02 12:00:00"; // Replace with your timestamp function output
    insertAction(db, action1, timestamp1);

    std::string action2 = "Defend";
    std::string timestamp2 = "2024-11-02 12:01:00"; // Replace with your timestamp function output
    insertAction(db, action2, timestamp2);
    
    sqlite3_close(db); // Close the database
    return 0;
}
