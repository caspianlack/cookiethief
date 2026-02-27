#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include <vector>
#include <string>
#include "platform.h"
#include "cookie.h"
#include "enemy.h"

struct LevelData
{
    std::vector<Platform> platforms;
    std::vector<Cookie *> cookies;
    std::vector<Enemy *> enemies;
    float playerStartX;
    float playerStartY;
    std::string name;
    int requiredCookies; // bomb jack
};

enum GameState
{
    STATE_MENU,
    STATE_LOBBY,
    STATE_RUN_INTRO,
    STATE_DOWNWELL,
    STATE_SIDE_ROOM_TRANSITION,
    STATE_BOMB_JACK,
    STATE_SHOP,
    STATE_DOWNWELL_COMPLETE,
    STATE_BOSS,
    STATE_RUN_COMPLETE,
    STATE_GAME_OVER
};

class LevelManager
{
private:
    std::vector<LevelData> levels;
    int currentLevelIndex;

public:
    LevelManager();
    ~LevelManager();

    void initializeLevels();
    LevelData *getCurrentLevel();
    LevelData *getLevel(int index);
    bool loadNextLevel();
    void resetToFirstLevel();
    int getLevelCount();
    int getCurrentLevelIndex();

    // lobby
    LevelData createLobby();

    // bomb jack
    LevelData createLevel1();
    LevelData createLevel2();
    LevelData createLevel3();

    void cleanLevel(LevelData &level);
};

#endif