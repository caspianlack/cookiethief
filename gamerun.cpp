#include "GameRun.h"
#include "Constants.h"
#include <algorithm>

GameRun::GameRun() {
    active = false;
    currentFloor = 0;
}

void GameRun::startNewRun() {
    active = true;
    currentFloor = 1;
    stats.reset();
    purchasedUpgrades.clear();
    initializeUpgrades();
}

void GameRun::endRun() {
    active = false;
}

void GameRun::initializeUpgrades() {
    availableUpgrades.clear();
    
    Upgrade glideDuration;
    glideDuration.type = UPGRADE_GLIDE_DURATION;
    glideDuration.cost = 10;
    glideDuration.purchased = false;
    glideDuration.name = "Extended Glide";
    glideDuration.description = "+50% glide time";
    availableUpgrades.push_back(glideDuration);
    
    Upgrade cookieMagnet;
    cookieMagnet.type = UPGRADE_COOKIE_MAGNET;
    cookieMagnet.cost = 12;
    cookieMagnet.purchased = false;
    cookieMagnet.name = "Cookie Magnet";
    cookieMagnet.description = "Attract nearby cookies";
    availableUpgrades.push_back(cookieMagnet);
    
    Upgrade extraLife;
    extraLife.type = UPGRADE_EXTRA_LIFE;
    extraLife.cost = 20;
    extraLife.purchased = false;
    extraLife.name = "Extra Heart";
    extraLife.description = "+1 max heart";
    availableUpgrades.push_back(extraLife);
    
    Upgrade maxEnergy;
    maxEnergy.type = UPGRADE_MAX_ENERGY;
    maxEnergy.cost = 8;
    maxEnergy.purchased = false;
    maxEnergy.name = "Sugar Boost";
    maxEnergy.description = "+25 max energy";
    availableUpgrades.push_back(maxEnergy);
}

bool GameRun::purchaseUpgrade(UpgradeType type, int& cookies) {
    // Find the upgrade
    for (auto& upgrade : availableUpgrades) {
        if (upgrade.type == type && !upgrade.purchased) {
            if (cookies >= upgrade.cost) {
                cookies -= upgrade.cost;
                upgrade.purchased = true;
                purchasedUpgrades.push_back(type);
                return true;
            }
            return false; // Not enough cookies
        }
    }
    return false; // Upgrade not found or already purchased
}

bool GameRun::hasUpgrade(UpgradeType type) const {
    return std::find(purchasedUpgrades.begin(), purchasedUpgrades.end(), type) 
           != purchasedUpgrades.end();
}

float GameRun::getGlideDurationMultiplier() const {
    return hasUpgrade(UPGRADE_GLIDE_DURATION) ? 1.5f : 1.0f;
}

// bool GameRun::hasDoubleJump() const {
//     return hasUpgrade(UPGRADE_DOUBLE_JUMP);
// }

int GameRun::getBonusHearts() const {
    return hasUpgrade(UPGRADE_EXTRA_LIFE) ? 1 : 0;
}

float GameRun::getMaxEnergyBonus() const {
    return hasUpgrade(UPGRADE_MAX_ENERGY) ? 25.0f : 0.0f;
}