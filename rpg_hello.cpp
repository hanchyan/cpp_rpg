#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <sqlite3.h>


std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();                  // Get current time
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);  // Convert to time_t for formatting
    std::tm* localTime = std::localtime(&timeNow);                // Convert to local time

    // Use a local ostringstream named timestamp to format the time
    std::ostringstream timestamp;                                   // Initialize ostringstream
    timestamp << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");  // Format time
    return timestamp.str();                                        // Return formatted time as string
}
// function for db
void logAction(sqlite3* db, const std::string& playerName, const std::string& action, const std::string& memo = "") {
    std::string timestamp = getCurrentTimestamp();
    std::string sql = "INSERT INTO game_log (player_name, action, memo, timestamp) VALUES ('" +
                      playerName + "', '" + action + "', '" + memo + "', '" + timestamp + "');";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error logging action: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}
// end function for db


class Player {
public:
    std::string name;
    int health;
    int strength;

    Player(std::string playerName, int playerHealth, int playerStrength) 
        : name(playerName), health(playerHealth), strength(playerStrength) {}

    void displayStatus() {
        std::cout << "\nPlayer: " << name << "\n"
                  << "Health: " << health << "\n"
                  << "Strength: " << strength << "\n\n" <<getCurrentTimestamp();
    }

    void rest() {
        std::cout << name << " takes a rest and recovers 10 health.\n" <<getCurrentTimestamp();
        health += 10;
    }

    bool isAlive() {
        return health > 0;
    }
};

class Enemy {
public:
    std::string type;
    int health;
    int strength;

    Enemy(std::string enemyType, int enemyHealth, int enemyStrength) 
        : type(enemyType), health(enemyHealth), strength(enemyStrength) {}

    void displayStatus() {
        std::cout << "Enemy: " << type << "\n"
                  << "Health: " << health << "\n"
                  << "Strength: " << strength << "\n\n"<<getCurrentTimestamp();
    }

    bool isAlive() {
        return health > 0;
    }
};

void attack(Player& player, Enemy& enemy) {
    std::cout << player.name << " attacks the " << enemy.type << "!\n";
    enemy.health -= player.strength;
    if (enemy.isAlive()) {
        std::cout << enemy.type << " strikes back!\n" <<getCurrentTimestamp();
        player.health -= enemy.strength;
    }
}

int main() {
// start db code
    sqlite3* db;
char* errMsg = nullptr;

// Open or create the database
if (sqlite3_open("game_data.db", &db)) {
    std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
    return 1;
}

// SQL statement to create a table if it doesn't exist
const char* createTableSQL = "CREATE TABLE IF NOT EXISTS game_log ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "player_name TEXT NOT NULL,"
                             "action TEXT NOT NULL,"
                             "timestamp TEXT NOT NULL,"
                             "memo TEXT);";

if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::cerr << "Error creating table: " << errMsg << std::endl;
    sqlite3_free(errMsg);
}
// end db code
    std::srand(static_cast<unsigned>(std::time(0))); // Random seed for enemy encounters
    std::cout << "Welcome to the RPG Adventure!\n";
    std::cout << "Enter your character's name: ";

    std::string playerName;
    std::getline(std::cin, playerName);

    Player player(playerName, 100, 15);
    std::cout << "Hello, " << player.name << "!\n" <<getCurrentTimestamp();
    player.displayStatus();

    char action;
    while (player.isAlive()) {
        std::cout << "Choose an action - (e)xplore, (r)est, or (q)uit: " <<getCurrentTimestamp();
        std::cin >> action;

        if (action == 'q') {
            logAction(db, player.name, "quit");
            std::cout << "Goodbye, brave " << player.name << "!\n" <<getCurrentTimestamp();
            break;
        } else if (action == 'm') {
        std::cout << "Enter your memo: ";
        std::cin.ignore();  // Clear newline from previous input
        std::string memo;
        std::getline(std::cin, memo);  // Get the full memo input from the player
        logAction(db, player.name, "memo", memo);  // Log the memo to the database
        std::cout << "Memo saved.\n";

        } else if (action == 'e') {
            // Encounter a random enemy
            // logs action to db
            logAction(db, player.name, "explore");
            Enemy enemy("Goblin", 30 + std::rand() % 20, 5 + std::rand() % 5);
            std::cout << "\nA wild " << enemy.type << " appears!\n" <<getCurrentTimestamp();
            enemy.displayStatus();

            while (player.isAlive() && enemy.isAlive()) {
                std::cout << "Choose an action - (a)ttack or (r)un: "<<getCurrentTimestamp();
                std::cin >> action;

                if (action == 'a') {
                    logAction(db, player.name, "attacked");
                    attack(player, enemy);
                    player.displayStatus();
                    enemy.displayStatus();
                } else if (action == 'r') {
                    std::cout << player.name << " ran away from the " << enemy.type << "!\n" <<getCurrentTimestamp();
                    break;
                }
            }

            if (!player.isAlive()) {
                logAction(db, player.name, "died");
                std::cout << player.name << " has fallen in battle. Game Over!\n" <<getCurrentTimestamp();
                break;
            } else if (!enemy.isAlive()) {
                logAction(db, player.name, "beat an enemy");
                std::cout << "The " << enemy.type << " has been defeated!\n" <<getCurrentTimestamp();
            }
        } else if (action == 'r') {
            logAction(db, player.name, "took a rest");
            player.rest();
            player.displayStatus();
        } else {
            std::cout << "Invalid action. Please try again.\n" <<getCurrentTimestamp();
        }
    }
    sqlite3_close(db);
    return 0;
}