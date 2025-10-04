#include "Game.h"
#include "Constants.h"
#include <cstdio>

Game::Game() {
    window = nullptr;
    renderer = nullptr;
    running = false;
    player = nullptr;
    cookieCount = 0;
    hasJumpedThisPress = false;
}

Game::~Game() {
    clean();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    
    window = SDL_CreateWindow("Cookie Thief - Prototype",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    if (!window) {
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        return false;
    }
    
    player = new Player(100, 100);
    
    // Create platforms (Bomb Jack style)
    platforms.push_back({100, 450, 200, 20, {139, 69, 19, 255}});
    platforms.push_back({400, 400, 150, 20, {139, 69, 19, 255}});
    platforms.push_back({200, 300, 180, 20, {139, 69, 19, 255}});
    platforms.push_back({500, 250, 150, 20, {139, 69, 19, 255}});
    platforms.push_back({150, 150, 200, 20, {139, 69, 19, 255}});
    
    // Cookies on platforms
    cookies.push_back(new Cookie(200, 420));
    cookies.push_back(new Cookie(470, 370));
    cookies.push_back(new Cookie(280, 270));
    cookies.push_back(new Cookie(570, 220));
    cookies.push_back(new Cookie(240, 120));
    
    // Add enemies
    enemies.push_back(new Enemy(600, 100));
    
    running = true;
    return true;
}

bool Game::isPlayerOnGround() {

    SDL_Rect playerRect = player->getRect();
    
    for (auto& platform : platforms) {
        SDL_Rect platformRect = platform.getRect();
        
        int playerBottom = playerRect.y + playerRect.h;
        int platformTop = platformRect.y;
        
        if (playerBottom >= platformTop && playerBottom <= platformTop + 5) {
            if (playerRect.x + playerRect.w > platformRect.x && 
                playerRect.x < platformRect.x + platformRect.w) {
                return true;
            }
        }
    }
    
    // just for development
    if (player->y + player->height >= SCREEN_HEIGHT - 1) {
        return true;
    }
    
    return false;
}

void Game::handleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_SPACE && !event.key.repeat) {
                bool actuallyOnGround = isPlayerOnGround();
                printf("SPACE PRESSED - onGround: %d, actuallyOnGround: %d, hasJumpedThisPress: %d\n", 
                       player->onGround, actuallyOnGround, hasJumpedThisPress);
                
                if (actuallyOnGround) {
                    player->onGround = true;
                    player->jump();
                    hasJumpedThisPress = true;
                    printf("  -> JUMPING\n");
                } else if (!hasJumpedThisPress) {
                    player->startGliding();
                    printf("  -> GLIDING\n");
                }
            }
        }
        
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_SPACE) {
                hasJumpedThisPress = false;
                player->stopGliding();
                printf("SPACE RELEASED - reset flag\n");
            }
        }
    }
    
    // Movement
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A]) {
        player->moveLeft();
    } else if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D]) {
        player->moveRight();
    } else {
        player->stopMoving();
    }
}

void Game::update() {
    player->update();
    
    checkPlatformCollisions();
    checkCookieCollisions();
    
    // Reset ability to jump once onground
    if (player->onGround) {
        hasJumpedThisPress = false;
    }
    
    // Update enemies
    for (auto& enemy : enemies) {
        enemy->update(*player);
    }
    
    checkEnemyCollisions();
}

void Game::checkPlatformCollisions() {
    player->onGround = false;
    
    for (auto& platform : platforms) {
        SDL_Rect playerRect = player->getRect();
        SDL_Rect platformRect = platform.getRect();
        
        if (SDL_HasIntersection(&playerRect, &platformRect)) {
            int playerBottom = playerRect.y + playerRect.h;
            int platformTop = platformRect.y;
            int playerPrevBottom = playerRect.y + playerRect.h - player->velocityY;
            
            // Collisions only from above
            if (player->velocityY > 0 && playerPrevBottom <= platformTop + 2) {
                player->y = platform.y - player->height;
                player->velocityY = 0;
                player->onGround = true;
            }
        }
    }
}

void Game::checkCookieCollisions() {
    for (auto& cookie : cookies) {
        if (!cookie->collected && cookie->checkCollision(*player)) {
            cookie->collected = true;
            cookieCount++;
        }
    }
}

void Game::checkEnemyCollisions() {
    for (auto& enemy : enemies) {
        if (enemy->checkCollision(*player)) {
            // just reset for now
            player->x = 100;
            player->y = 100;
            player->velocityX = 0;
            player->velocityY = 0;
            player->onGround = false;
            player->isGliding = false;
            player->glideTime = MAX_GLIDE_TIME;
        }
    }
}

void Game::render() {
    // background
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_RenderClear(renderer);
    
    // Render platforms
    for (const auto& platform : platforms) {
        platform.render(renderer);
    }
    
    // Render cookies
    for (auto& cookie : cookies) {
        cookie->render(renderer);
    }
    
    // Render enemies
    for (auto& enemy : enemies) {
        enemy->render(renderer);
    }
    
    // Render player
    player->render(renderer);
    
    // Render cookie counter
    for (int i = 0; i < cookieCount; i++) {
        SDL_Rect cookieIcon = {10 + i * 25, 10, 20, 20};
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
        SDL_RenderFillRect(renderer, &cookieIcon);
    }
    
    SDL_RenderPresent(renderer);
}

void Game::run() {
    Uint32 frameStart;
    int frameTime;
    
    while (running) {
        frameStart = SDL_GetTicks();
        
        handleEvents();
        update();
        render();
        
        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
}

void Game::clean() {
    if (player) {
        delete player;
        player = nullptr;
    }
    
    for (auto cookie : cookies) {
        delete cookie;
    }
    cookies.clear();
    
    for (auto enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}