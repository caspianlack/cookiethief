#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vector>
#include "player.h"
#include "enemy.h"
#include "cookie.h"
#include "platform.h"
#include "textmanager.h"
#include "levelmanager.h"
#include "gamerun.h"
#include "downwellgenerator.h"
#include "projectile.h"

struct SideDoor
{
    SDL_Rect rect;
    RoomType type;
    bool used;
    float worldY;
    bool onLeftWall;
};

struct Collider
{
    SDL_Rect rect;
    bool isWorldSpace;
};

class Game
{
private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running;

    Player *player;
    std::vector<Platform> platforms;
    std::vector<Cookie *> cookies;
    std::vector<Enemy *> enemies;
    std::vector<SideDoor> sideDoors;
    std::vector<Collider> alcoveCeilings;

    TextManager *textManager;
    LevelManager *levelManager;
    DownwellGenerator *downwellGenerator;
    GameRun *currentRun;
    PersistentStats persistentStats;

    GameState currentState;
    GameState previousState; // returning from side rooms

    // Camera for Downwell
    float cameraY;
    float worldHeight;

    // Lobby
    SDL_Rect startRunDoor;
    bool playerNearStartDoor;

    // Downwell tracking
    int currentSegment;
    float playerReturnX;
    float playerReturnY;
    bool shopIsFromSideRoom;
    float maxDepthReached;

    // Bomb Jack
    LevelData *currentBombJackLevel;
    int bombJackCookiesCollected;

    // Shop
    int selectedUpgradeIndex;

    // Input tracking
    bool hasJumpedThisPress;
    bool hasInteractedThisPress;

    // persistant levels between side and main level (bomb jack and downwell)
    std::vector<Platform> savedPlatforms;
    std::vector<Cookie *> savedCookies;
    std::vector<Collider> savedAlcoveCeilings;
    std::vector<SideDoor> savedSideDoors;
    float savedWorldHeight;

    std::vector<Enemy *> savedEnemies;
    std::vector<Projectile *> projectiles;

    float transitionTimer;

    void handleEvents();
    void update();
    void render();

    // State updates
    void updateLobby();
    void updateDownwell();
    void updateBombJack();
    void updateShop();
    void updateDownwellComplete();
    void updateRunComplete();

    // State rendering
    void renderLobby();
    void renderDownwell();
    void renderBombJack();
    void renderShop();
    void renderDownwellComplete();
    void renderRunComplete();
    void renderRunIntro();
    void renderGameOver();

    // Collisions
    void checkPlatformCollisions();
    void checkCookieCollisions();
    void checkEnemyCollisions();
    void checkAlcoveCeilingCollisions();
    void checkSideDoorInteraction();
    bool isPlayerOnGround();

    // UI
    void renderHearts();
    void renderUI();
    void renderEnergyBar();
    void renderStats();

    // State transitions
    void loadLobby();
    void startNewRun();
    void generateDownwellSegment();
    void enterSideRoom(RoomType type);
    void exitSideRoom();
    void completeDownwellSegment();
    void endRun(bool victory);
    void applyUpgradesToPlayer();
    void renderDownwellWalls();
    void renderSideStats();

    void checkProjectileCollisions();
    void cleanProjectiles();

    // Helper functions
    void cleanCurrentLevel();
    SDL_Rect worldToScreen(SDL_Rect worldRect);
    float worldToScreenY(float worldY);

public:
    Game();
    ~Game();

    bool init();
    void run();
    void clean();
};

#endif