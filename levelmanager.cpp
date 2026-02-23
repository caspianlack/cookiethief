#include "LevelManager.h"
#include <cstdio>
#include "constants.h"

LevelManager::LevelManager()
{
    currentLevelIndex = 0;
}

LevelManager::~LevelManager()
{
    for (auto &level : levels)
    {
        cleanLevel(level);
    }
    levels.clear();
}

void LevelManager::initializeLevels()
{
    // bomb jack levels
    levels.push_back(createLevel1());
    levels.push_back(createLevel2());
    levels.push_back(createLevel3());

    printf("Initialized %d levels\n", (int)levels.size());
}

LevelData *LevelManager::getCurrentLevel()
{
    if (currentLevelIndex >= 0 && currentLevelIndex < (int)levels.size())
    {
        return &levels[currentLevelIndex];
    }
    return nullptr;
}

LevelData *LevelManager::getLevel(int index)
{
    if (index >= 0 && index < (int)levels.size())
    {
        return &levels[index];
    }
    return nullptr;
}

bool LevelManager::loadNextLevel()
{
    currentLevelIndex++;
    if (currentLevelIndex < (int)levels.size())
    {
        printf("Loaded level %d: %s\n", currentLevelIndex + 1, levels[currentLevelIndex].name.c_str());
        return true;
    }

    printf("No more levels! You beat the game!\n");
    currentLevelIndex = levels.size() - 1;
    return false;
}

void LevelManager::resetToFirstLevel()
{
    currentLevelIndex = 0;
}

int LevelManager::getLevelCount()
{
    return levels.size();
}

int LevelManager::getCurrentLevelIndex()
{
    return currentLevelIndex;
}

LevelData LevelManager::createLobby()
{
    LevelData lobby;
    lobby.name = "Lobby - Cookie Thief HQ";
    lobby.playerStartX = 100;
    lobby.playerStartY = 100;
    lobby.requiredCookies = 0;

    lobby.platforms.push_back({0, 552, 800, 48, {100, 100, 100, 255}, PLATFORM_DARK});

    lobby.platforms.push_back({48, 456, 144, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

    lobby.platforms.push_back({312, 408, 192, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

    lobby.platforms.push_back({600, 456, 144, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

    return lobby;
}

// bomb jack levels
// TODO: Design better levels... will do after enemy ai is finalised
LevelData LevelManager::createLevel1()
{
    LevelData level;
    level.name = "Level 1 - The Kitchen";
    level.playerStartX = SCREEN_WIDTH / 2;
    level.playerStartY = ARENA_TOP + 80;
    level.requiredCookies = 5;

    // CREATE CENTERED BOX BOUNDARY
    int wallThickness = 24;

    // Floor
    level.platforms.push_back({(float)ARENA_LEFT,
                               (float)(ARENA_TOP + ARENA_HEIGHT - wallThickness),
                               (float)ARENA_WIDTH,
                               (float)wallThickness,
                               {80, 80, 80, 255},
                               PLATFORM_DARK});

    // Ceiling
    level.platforms.push_back({(float)ARENA_LEFT,
                               (float)ARENA_TOP,
                               (float)ARENA_WIDTH,
                               (float)wallThickness,
                               {80, 80, 80, 255},
                               PLATFORM_DARK});

    // Left
    level.platforms.push_back({(float)ARENA_LEFT,
                               (float)ARENA_TOP,
                               (float)wallThickness,
                               (float)ARENA_HEIGHT,
                               {80, 80, 80, 255},
                               PLATFORM_DARK});

    // Right
    level.platforms.push_back({(float)(ARENA_LEFT + ARENA_WIDTH - wallThickness),
                               (float)ARENA_TOP,
                               (float)wallThickness,
                               (float)ARENA_HEIGHT,
                               {80, 80, 80, 255},
                               PLATFORM_DARK});

    // platforms
    level.platforms.push_back({ARENA_LEFT + 72, ARENA_TOP + 408, 120, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 288, ARENA_TOP + 360, 120, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 96, ARENA_TOP + 264, 144, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 312, ARENA_TOP + 216, 120, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 72, ARENA_TOP + 96, 144, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

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

LevelData LevelManager::createLevel2()
{
    LevelData level;
    level.name = "Level 2 - The Pantry";
    level.playerStartX = SCREEN_WIDTH / 2;
    level.playerStartY = ARENA_TOP + 80;
    level.requiredCookies = 7;

    int wallThickness = 24;

    level.platforms.push_back({(float)ARENA_LEFT, (float)(ARENA_TOP + ARENA_HEIGHT - wallThickness), (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}, PLATFORM_DARK});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}, PLATFORM_DARK});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}, PLATFORM_DARK});
    level.platforms.push_back({(float)(ARENA_LEFT + ARENA_WIDTH - wallThickness), (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}, PLATFORM_DARK});

    // platforms
    level.platforms.push_back({ARENA_LEFT + 48,  ARENA_TOP + 456, 96,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 192, ARENA_TOP + 408, 96,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 360, ARENA_TOP + 456, 96,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

    level.platforms.push_back({ARENA_LEFT + 72,  ARENA_TOP + 312, 96,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 288, ARENA_TOP + 264, 120, 24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

    level.platforms.push_back({ARENA_LEFT + 120, ARENA_TOP + 144, 96,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 312, ARENA_TOP + 144, 96,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 216, ARENA_TOP + 72,  72,  24, {139, 69, 19, 255}, (PlatformType)(rand() % 3)});

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

LevelData LevelManager::createLevel3()
{
    LevelData level;
    level.name = "Level 3 - The Freezer";
    level.playerStartX = SCREEN_WIDTH / 2;
    level.playerStartY = ARENA_TOP + 80;
    level.requiredCookies = 8;

    int wallThickness = 24;

    level.platforms.push_back({(float)ARENA_LEFT, (float)(ARENA_TOP + ARENA_HEIGHT - wallThickness), (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}, PLATFORM_DARK});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)ARENA_WIDTH, (float)wallThickness, {80, 80, 80, 255}, PLATFORM_DARK});
    level.platforms.push_back({(float)ARENA_LEFT, (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}, PLATFORM_DARK});
    level.platforms.push_back({(float)(ARENA_LEFT + ARENA_WIDTH - wallThickness), (float)ARENA_TOP, (float)wallThickness, (float)ARENA_HEIGHT, {80, 80, 80, 255}, PLATFORM_DARK});

    // platforms
    level.platforms.push_back({ARENA_LEFT + 192, ARENA_TOP + 96,  120, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 72,  ARENA_TOP + 192, 120, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 312, ARENA_TOP + 192, 120, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 168, ARENA_TOP + 312, 168, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 48,  ARENA_TOP + 408, 120, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 336, ARENA_TOP + 408, 120, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});
    level.platforms.push_back({ARENA_LEFT + 192, ARENA_TOP + 480, 144, 24, {100, 150, 200, 255}, (PlatformType)(rand() % 3)});

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

void LevelManager::cleanLevel(LevelData &level)
{
    // d all dynamically allocated cookies
    for (auto cookie : level.cookies)
    {
        delete cookie;
    }
    level.cookies.clear();

    // d all dynamically allocated enemies
    for (auto enemy : level.enemies)
    {
        delete enemy;
    }
    level.enemies.clear();

    level.platforms.clear();
}