#ifndef GAME_RUN_H
#define GAME_RUN_H

#include <vector>
#include <string>

// type of rooms/ levels
enum RoomType {
    ROOM_DOWNWELL,    // Main vertical descent
    ROOM_BOMB_JACK,   // Side platforming challenge
    ROOM_SHOP,        // Upgrade shop
    ROOM_BOSS         // Boss encounter (future)
};

// Upgrade types
enum UpgradeType {
    // UPGRADE_DOUBLE_JUMP,
    UPGRADE_GLIDE_DURATION,
    UPGRADE_COOKIE_MAGNET,
    UPGRADE_EXTRA_LIFE,
    UPGRADE_MAX_ENERGY
};

struct Upgrade {
    UpgradeType type;
    int cost;
    bool purchased;
    std::string name;
    std::string description;
};

// Stats for current run only
struct RunStats {
    int cookiesThisRun;
    int downwellLevelsCleared;
    int bombJackLevelsCleared;
    int jumpsThisRun;
    int distanceFell;
    float timeElapsed;
    int enemiesAvoided;
    
    RunStats() {
        reset();
    }
    
    void reset() {
        cookiesThisRun = 0;
        downwellLevelsCleared = 0;
        bombJackLevelsCleared = 0;
        jumpsThisRun = 0;
        distanceFell = 0;
        timeElapsed = 0.0f;
        enemiesAvoided = 0;
    }
};

// Persistent total stats
struct PersistentStats {
    int totalDeaths;
    int totalCookies;
    int totalDistanceFell;
    int totalJumps;
    int bombJackLevelsCleared;
    int downwellLevelsCleared;
    int highestFloorReached;
    int totalPlaythroughs;
    
    PersistentStats() {
        totalDeaths = 0;
        totalCookies = 0;
        totalDistanceFell = 0;
        totalJumps = 0;
        bombJackLevelsCleared = 0;
        downwellLevelsCleared = 0;
        highestFloorReached = 0;
        totalPlaythroughs = 0;
    }
};

// manages each run
class GameRun {
private:
    RunStats stats;
    int currentFloor;
    bool active;
    std::vector<Upgrade> availableUpgrades;
    std::vector<UpgradeType> purchasedUpgrades;
    
public:
    GameRun();
    
    void startNewRun();
    void endRun();
    bool isActive() const { return active; }
    
    // stats
    RunStats& getStats() { return stats; }
    const RunStats& getStats() const { return stats; }
    
    // floor
    int getCurrentFloor() const { return currentFloor; }
    void advanceFloor() { currentFloor++; }
    
    // upgrades
    void initializeUpgrades();
    const std::vector<Upgrade>& getAvailableUpgrades() const { return availableUpgrades; }
    bool purchaseUpgrade(UpgradeType type, int& cookies);
    bool hasUpgrade(UpgradeType type) const;
    
    float getGlideDurationMultiplier() const;
    // bool hasDoubleJump() const;
    int getBonusHearts() const;
    float getMaxEnergyBonus() const;
};

#endif