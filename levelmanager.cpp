#include "LevelManager.h"
#include <cstdio>
#include "constants.h"

LevelManager::LevelManager() {
    currentLevelIndex = 0;
}

LevelManager::~LevelManager() {
    for (auto& level : levels) {
        cleanLevel(level);
    }
    levels.clear();
}

void LevelManager::initializeLevels() {
    // bomb jack levels
    levels.push_back(createLevel1());
    levels.push_back(createLevel2());
    levels.push_back(createLevel3());
    
    printf("Initialized %d levels\n", (int)levels.size());
}

LevelData* LevelManager::getCurrentLevel() {
    if (currentLevelIndex >= 0 && currentLevelIndex < (int)levels.size()) {
        return &levels[currentLevelIndex];
    }
    return nullptr;
}

LevelData* LevelManager::getLevel(int index) {
    if (index >= 0 && index < (int)levels.size()) {
        return &levels[index];
    }
    return nullptr;
}

bool LevelManager::loadNextLevel() {
    currentLevelIndex++;
    if (currentLevelIndex < (int)levels.size()) {
        printf("Loaded level %d: %s\n", currentLevelIndex + 1, levels[currentLevelIndex].name.c_str());
        return true;
    }
    
    printf("No more levels! You beat the game!\n");
    currentLevelIndex = levels.size() - 1;
    return false;
}

void LevelManager::resetToFirstLevel() {
    currentLevelIndex = 0;
}

int LevelManager::getLevelCount() {
    return levels.size();
}

int LevelManager::getCurrentLevelIndex() {
    return currentLevelIndex;
}

LevelData LevelManager::createLobby() {
    LevelData lobby;
    lobby.name = "Lobby - Cookie Thief HQ";
    lobby.playerStartX = 100;
    lobby.playerStartY = 100;
    lobby.requiredCookies = 0;
    
    lobby.platforms.push_back({0, 550, 800, 50, {100, 100, 100, 255}});
    
    lobby.platforms.push_back({50, 450, 150, 20, {139, 69, 19, 255}});
    
    lobby.platforms.push_back({300, 400, 200, 20, {139, 69, 19, 255}});
    
    lobby.platforms.push_back({600, 450, 150, 20, {139, 69, 19, 255}});
    
    return lobby;
}

// bomb jack levels
// TODO: Design better levels... will do after enemy ai is finalised
LevelData LevelManager::createLevel1() {
    LevelData level;
    level.name = "Level 1 - The Kitchen";
    level.playerStartX = SCREEN_WIDTH / 2;
    level.playerStartY = ARENA_TOP + 80;
    level.requiredCookies = 5;
    
    // CREATE CENTERED BOX BOUNDARY
    int wallThickness = 15;
    
    // Floor
    level.platforms.push_back({
        (float)ARENA_LEFT, 
        (float)(ARENA_TOP + ARENA_HEIGHT - wallThickness), 
        (float)ARENA_WIDTH, 
        (float)wallThickness, 
        {80, 80, 80, 255}
    });
    
    // Ceiling
    level.platforms.push_back({
        (float)ARENA_LEFT, 
        (float)ARENA_TOP, 
        (float)ARENA_WIDTH, 
        (float)wallThickness, 
        {80, 80, 80, 255}
    });
    
    // Left
    level.platforms.push_back({
        (float)ARENA_LEFT, 
        (float)ARENA_TOP, 
        (float)wallThickness, 
        (float)ARENA_HEIGHT, 
        {80, 80, 80, 255}
    });
    
    // Right
    level.platforms.push_back({
        (float)(ARENA_LEFT + ARENA_WIDTH - wallThickness), 
        (float)ARENA_TOP, 
        (float)wallThickness, 
        (float)ARENA_HEIGHT, 
        {80, 80, 80, 255}
    });
    
    // platforms
    level.platforms.push_back({ARENA_LEFT + 80, ARENA_TOP + 400, 120, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 300, ARENA_TOP + 350, 120, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 100, ARENA_TOP + 250, 150, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 320, ARENA_TOP + 200, 120, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 80, ARENA_TOP + 100, 140, 15, {139, 69, 19, 255}});
    
    // cookies
    level.cookies.push_back(new Cookie(ARENA_LEFT + 130, ARENA_TOP + 370));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 350, ARENA_TOP + 320));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 165, ARENA_TOP + 220));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 370, ARENA_TOP + 170));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 140, ARENA_TOP + 70));
    
    // enemy
    level.enemies.push_back(new Enemy(ARENA_LEFT + 350, ARENA_TOP + 80));
    
    return level;
}

LevelData LevelManager::createLevel2() {
    LevelData level;
    level.name = "Level 2 - The Pantry";
    level.playerStartX = SCREEN_WIDTH / 2;
    level.playerStartY = ARENA_TOP + 80;
    level.requiredCookies = 7;
    
    int wallThickness = 15;
    
    level.platforms.push_back({(float)ARENA_LEFT, (float)(ARENA_TOP + ARENA_HEIGHT - wallThickness), (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}});
    level.platforms.push_back({(float)(ARENA_LEFT + ARENA_WIDTH - wallThickness), (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}});
    
    // platforms
    level.platforms.push_back({ARENA_LEFT + 50, ARENA_TOP + 450, 100, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 200, ARENA_TOP + 400, 100, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 350, ARENA_TOP + 450, 100, 15, {139, 69, 19, 255}});
    
    level.platforms.push_back({ARENA_LEFT + 80, ARENA_TOP + 300, 100, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 280, ARENA_TOP + 250, 120, 15, {139, 69, 19, 255}});
    
    level.platforms.push_back({ARENA_LEFT + 120, ARENA_TOP + 150, 100, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 320, ARENA_TOP + 150, 100, 15, {139, 69, 19, 255}});
    level.platforms.push_back({ARENA_LEFT + 220, ARENA_TOP + 80, 80, 15, {139, 69, 19, 255}});
    
    // cookies
    level.cookies.push_back(new Cookie(ARENA_LEFT + 90, ARENA_TOP + 420));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 240, ARENA_TOP + 370));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 390, ARENA_TOP + 420));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 120, ARENA_TOP + 270));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 330, ARENA_TOP + 220));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 160, ARENA_TOP + 120));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 250, ARENA_TOP + 50));
    
    // enemies
    level.enemies.push_back(new Enemy(ARENA_LEFT + 250, ARENA_TOP + 60));
    level.enemies.push_back(new Enemy(ARENA_LEFT + 380, ARENA_TOP + 400));
    
    return level;
}

LevelData LevelManager::createLevel3() {
    LevelData level;
    level.name = "Level 3 - The Freezer";
    level.playerStartX = SCREEN_WIDTH / 2;
    level.playerStartY = ARENA_TOP + 80;
    level.requiredCookies = 8;
    
    int wallThickness = 15;
    
    level.platforms.push_back({(float)ARENA_LEFT, (float)(ARENA_TOP + ARENA_HEIGHT - wallThickness), (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}});
    level.platforms.push_back({(float)(ARENA_LEFT + ARENA_WIDTH - wallThickness), (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}});
    
    // platforms
    level.platforms.push_back({ARENA_LEFT + 200, ARENA_TOP + 100, 100, 15, {100, 150, 200, 255}});
    level.platforms.push_back({ARENA_LEFT + 80, ARENA_TOP + 200, 100, 15, {100, 150, 200, 255}});
    level.platforms.push_back({ARENA_LEFT + 320, ARENA_TOP + 200, 100, 15, {100, 150, 200, 255}});
    level.platforms.push_back({ARENA_LEFT + 170, ARENA_TOP + 300, 160, 15, {100, 150, 200, 255}});
    level.platforms.push_back({ARENA_LEFT + 50, ARENA_TOP + 400, 120, 15, {100, 150, 200, 255}});
    level.platforms.push_back({ARENA_LEFT + 330, ARENA_TOP + 400, 120, 15, {100, 150, 200, 255}});
    level.platforms.push_back({ARENA_LEFT + 180, ARENA_TOP + 490, 140, 15, {100, 150, 200, 255}});
    
    // cookies
    level.cookies.push_back(new Cookie(ARENA_LEFT + 240, ARENA_TOP + 70));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 120, ARENA_TOP + 170));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 360, ARENA_TOP + 170));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 240, ARENA_TOP + 270));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 100, ARENA_TOP + 370));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 380, ARENA_TOP + 370));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 220, ARENA_TOP + 460));
    level.cookies.push_back(new Cookie(ARENA_LEFT + 280, ARENA_TOP + 460));
    
    // enemies
    level.enemies.push_back(new Enemy(ARENA_LEFT + 100, ARENA_TOP + 350));
    level.enemies.push_back(new Enemy(ARENA_LEFT + 250, ARENA_TOP + 250));
    level.enemies.push_back(new Enemy(ARENA_LEFT + 380, ARENA_TOP + 150));
    
    return level;
}

void LevelManager::cleanLevel(LevelData& level) {
    // d all dynamically allocated cookies
    for (auto cookie : level.cookies) {
        delete cookie;
    }
    level.cookies.clear();
    
    // d all dynamically allocated enemies
    for (auto enemy : level.enemies) {
        delete enemy;
    }
    level.enemies.clear();
    
    level.platforms.clear();
}