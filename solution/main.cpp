#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "..\lib\cpp\pugixml-1.14\src\pugixml.hpp"

using namespace std;

struct Troop {
    string name;
    int damage;
};

struct Ship {
    int id;
    int strength;
    int timeLimit;
    int destroyedTime = -1; // -1 means not destroyed
};

vector<Troop> troops;
vector<Ship> ships;

void parseXML(const string& filePath) {
    pugi::xml_document doc;
    if (!doc.load_file(filePath.c_str())) {
        cerr << "Error loading XML file!" << endl;
        return;
    }

    pugi::xml_node island = doc.child("Island");

    // Read troops
    for (pugi::xml_node troop : island.children("Troop")) {
        string name = troop.child("Name").text().as_string();
        int damage = troop.child("RateOfDamage").text().as_int();
        troops.push_back({name, damage});
    }

    // Read ships
    for (pugi::xml_node ship : island.children("Ship")) {
        int id = ship.attribute("id").as_int();
        int strength = ship.child("Strength").text().as_int();
        int timeLimit = ship.child("TimeLimit").text().as_int();
        ships.push_back({id, strength, timeLimit});
    }
}
void attackShips() {
    // Sort troops by damage (descending)
    sort(troops.begin(), troops.end(), [](const Troop& a, const Troop& b) {
        return a.damage > b.damage;
    });

    // Sort ships by ID (ascending)
    sort(ships.begin(), ships.end(), [](const Ship& a, const Ship& b) {
        return a.id < b.id;
    });

    vector<Ship*> activeShips;
    for (auto& ship : ships) {
        activeShips.push_back(&ship);
    }

    int currentTime = 0;
    while (!activeShips.empty()) {
        // Sort active ships by time limit (ascending)
        sort(activeShips.begin(), activeShips.end(), [](const Ship* a, const Ship* b) {
            return a->timeLimit < b->timeLimit;
        });

        bool anyShipDestroyed = false;
        
        // Try to attack each ship with available troops
        for (size_t i = 0; i < activeShips.size(); i++) {
            Ship* ship = activeShips[i];
            
            // Try each troop on this ship
            for (const auto& troop : troops) {
                // Check if we can destroy the ship within its time limit
                int remainingTime = ship->timeLimit - currentTime;
                if (remainingTime <= 0) continue;

                ship->strength -= troop.damage;
                
                if (ship->strength <= 0) {
                    ship->destroyedTime = currentTime + 1;
                    activeShips.erase(activeShips.begin() + i);
                    anyShipDestroyed = true;
                    i--; // Adjust index after removal
                    break;
                }
            }
        }

        if (!anyShipDestroyed && !activeShips.empty()) {
            // If no ships were destroyed this round but there are still active ships,
            // increment time
            currentTime++;
        }
        
        // Break if we can't destroy any more ships
        if (!anyShipDestroyed) {
            break;
        }
    }

    // Any remaining ships in activeShips weren't destroyed
    for (auto* ship : activeShips) {
        ship->destroyedTime = -1;  // Mark as not destroyed
    }
}

void printDestroyedShips() {
    vector<pair<int, int>> destroyed;
    for (const auto& ship : ships) {
        if (ship.destroyedTime != -1)
            destroyed.push_back({ship.id, ship.destroyedTime});
    }

    sort(destroyed.begin(), destroyed.end());
    cout << destroyed.size() << ", [";
    for (size_t i = 0; i < destroyed.size(); ++i) {
        cout << "(" << destroyed[i].first << ", " << destroyed[i].second << ")";
        if (i < destroyed.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

void printRemainingShips() {
    vector<pair<int, int>> remaining;
    for (const auto& ship : ships) {
        if (ship.destroyedTime == -1)
            remaining.push_back({ship.id, ship.strength});
    }

    sort(remaining.begin(), remaining.end());
    cout << remaining.size() << ", [";
    for (size_t i = 0; i < remaining.size(); ++i) {
        cout << "(" << remaining[i].first << ", " << remaining[i].second << ")";
        if (i < remaining.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <xml_file>" << endl;
        return 1;
    }

    string path = argv[1];
    parseXML(path);
    attackShips();

    string command;
    while (getline(cin, command)) {
        if (command == "print destroyed ships")
            printDestroyedShips();
        else if (command == "print remaining ships")
            printRemainingShips();
        else if (command == "exit")
            break;
    }

    return 0;
}