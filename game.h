#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Cookie.h"
#include "Platform.h"
#include "TextManager.h"

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    Player* player;
    std::vector<Platform> platforms;
    std::vector<Cookie*> cookies;
    std::vector<Enemy*> enemies;
    int cookieCount;
    int totalCookiesCollected;
    
    TextManager* textManager;
    
    bool hasJumpedThisPress;
    
    void handleEvents();
    void update();
    void render();
    void checkPlatformCollisions();
    void checkCookieCollisions();
    void checkEnemyCollisions();
    bool isPlayerOnGround();
    void renderHearts();
    void renderGameOver();
    
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void clean();
};

#endif