#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "player.h"
#include "enemy.h"
#include "cookie.h"
#include "platform.h"
#include "textmanager.h"
#include "levelmanager.h"
#include "gamerun.h"
#include "downwellgenerator.h"
#include "projectile.h"
#include "texturemanager.h"
#include "screen.h"

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
    Enemy *baker; // The persistent chaser
    std::vector<SideDoor> sideDoors;
    std::vector<Collider> alcoveCeilings;

    TextManager *textManager;
    TextureManager *textureManager;
    LevelManager *levelManager;
    DownwellGenerator *downwellGenerator;
    GameRun *currentRun;
    PersistentStats persistentStats;

    ScreenManager screenManager;

    GameState currentState;
    GameState previousState; // returning from side rooms

    // Camera for Downwell
    float cameraY;
    float worldHeight;

    // Lobby
    SDL_Rect recipeRect;
    bool playerNearRecipe;
    float interactionTimer;
    const float RECIPE_STEAL_TIME = 1.0f;

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

    // Screen shake
    float shakeTimer;     // Time remaining for shake
    float shakeIntensity; // Max pixel offset during shake
    
    // Frame dumping
    bool isRecording;
    int frameCounter;
    void captureFrame();

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
    void updatePlatforms();
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
    void startNewRun();
    void generateDownwellSegment();
    void enterSideRoom(RoomType type);
    void exitSideRoom();
    void completeDownwellSegment();
    void endRun(bool victory);
    void applyUpgradesToPlayer();
    void renderDownwellWalls();
    void renderSideStats();
    void renderTexturedPlatform(const Platform& platform);

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

    // --- Public interface for Screen subclasses ---
    void loadLobby();   // moved to public so GameOverOverlay can call it
    void triggerHeartLoss(); // pushes HeartLossOverlay onto the screen stack
    void exitBombJackRoom(); // completes BJ level and returns to Downwell

    // Getters for screen rendering helpers
    SDL_Renderer*  getRenderer()       const { return renderer; }
    TextManager*   getTextManager()    const { return textManager; }
    TextureManager* getTextureManager() const { return textureManager; }
    GameRun*       getCurrentRun()     const { return currentRun; }
    Player*        getPlayer()         const { return player; }
    float          getCameraY()        const { return cameraY; }
};

#endif