#include "Game.h"
#include "Constants.h"
#include <cstdio>

Game::Game() {
    window = nullptr;
    renderer = nullptr;
    running = false;
    player = nullptr;
    textManager = nullptr;
    cookieCount = 0;
    totalCookiesCollected = 0;
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
    
    // Initialize text manager
    textManager = new TextManager();
    if (!textManager->init()) {
        printf("Failed to initialize TextManager!\n");
        return false;
    }
    
    // Load fonts (temp for now while finding 8bit font I made previously)
    // temp font: https://fonts.google.com/specimen/Press+Start+2P
    textManager->loadFont("title", "PressStart2P.ttf", 32);
    textManager->loadFont("normal", "PressStart2P.ttf", 16);
    textManager->loadFont("small", "PressStart2P.ttf", 12);

    // Copyright 2012 The Press Start 2P Project Authors (cody@zone38.net), with Reserved Font Name "Press Start 2P".

    // This Font Software is licensed under the SIL Open Font License, Version 1.1.
    // This license is copied below, and is also available with a FAQ at:
    // https://openfontlicense.org


    // -----------------------------------------------------------
    // SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
    // -----------------------------------------------------------

    // PREAMBLE
    // The goals of the Open Font License (OFL) are to stimulate worldwide
    // development of collaborative font projects, to support the font creation
    // efforts of academic and linguistic communities, and to provide a free and
    // open framework in which fonts may be shared and improved in partnership
    // with others.

    // The OFL allows the licensed fonts to be used, studied, modified and
    // redistributed freely as long as they are not sold by themselves. The
    // fonts, including any derivative works, can be bundled, embedded, 
    // redistributed and/or sold with any software provided that any reserved
    // names are not used by derivative works. The fonts and derivatives,
    // however, cannot be released under any other type of license. The
    // requirement for fonts to remain under this license does not apply
    // to any document created using the fonts or their derivatives.

    // DEFINITIONS
    // "Font Software" refers to the set of files released by the Copyright
    // Holder(s) under this license and clearly marked as such. This may
    // include source files, build scripts and documentation.

    // "Reserved Font Name" refers to any names specified as such after the
    // copyright statement(s).

    // "Original Version" refers to the collection of Font Software components as
    // distributed by the Copyright Holder(s).

    // "Modified Version" refers to any derivative made by adding to, deleting,
    // or substituting -- in part or in whole -- any of the components of the
    // Original Version, by changing formats or by porting the Font Software to a
    // new environment.

    // "Author" refers to any designer, engineer, programmer, technical
    // writer or other person who contributed to the Font Software.

    // PERMISSION & CONDITIONS
    // Permission is hereby granted, free of charge, to any person obtaining
    // a copy of the Font Software, to use, study, copy, merge, embed, modify,
    // redistribute, and sell modified and unmodified copies of the Font
    // Software, subject to the following conditions:

    // 1) Neither the Font Software nor any of its individual components,
    // in Original or Modified Versions, may be sold by itself.

    // 2) Original or Modified Versions of the Font Software may be bundled,
    // redistributed and/or sold with any software, provided that each copy
    // contains the above copyright notice and this license. These can be
    // included either as stand-alone text files, human-readable headers or
    // in the appropriate machine-readable metadata fields within text or
    // binary files as long as those fields can be easily viewed by the user.

    // 3) No Modified Version of the Font Software may use the Reserved Font
    // Name(s) unless explicit written permission is granted by the corresponding
    // Copyright Holder. This restriction only applies to the primary font name as
    // presented to the users.

    // 4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
    // Software shall not be used to promote, endorse or advertise any
    // Modified Version, except to acknowledge the contribution(s) of the
    // Copyright Holder(s) and the Author(s) or with their explicit written
    // permission.

    // 5) The Font Software, modified or unmodified, in part or in whole,
    // must be distributed entirely under this license, and must not be
    // distributed under any other license. The requirement for fonts to
    // remain under this license does not apply to any document created
    // using the Font Software.

    // TERMINATION
    // This license becomes null and void if any of the above conditions are
    // not met.

    // DISCLAIMER
    // THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
    // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
    // OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
    // COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    // INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
    // DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    // FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
    // OTHER DEALINGS IN THE FONT SOFTWARE.

    
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
        
        // Check for restart when dead
        if (player->isDead && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_r) {
                player->reset(100, 100);
                printf("Player reset from game over!\n");
            }
            continue;
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
            
            // Press R to reset (will spawn in lobby later)
            if (event.key.keysym.sym == SDLK_r) {
                player->reset(100, 100);
                printf("Player reset!\n");
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
    
    // !process movement when dead
    if (player->isDead) {
        return;
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
            totalCookiesCollected++;
            
            // Restore energy (Sugar rush!)
            player->restoreEnergy(COOKIE_ENERGY_RESTORE);
            
            printf("Cookie collected! Sugar rush! Count: %d, Total: %d\n", 
                   cookieCount, totalCookiesCollected);
        }
    }
}

void Game::checkEnemyCollisions() {
    for (auto& enemy : enemies) {
        if (enemy->checkCollision(*player)) {
            if (!player->isInvincible) {
                player->loseHeart();
            }
        }
    }
}

void Game::renderHearts() {
    int heartSize = 20;
    int heartSpacing = 25;
    int startX = SCREEN_WIDTH - 10 - (player->maxHearts * heartSpacing);
    int startY = 10;
    
    for (int i = 0; i < player->maxHearts; i++) {
        int heartX = startX + (i * heartSpacing);
        
        if (i < player->hearts) {
            // Heart (red)
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            // Heart capacity (dark gray)
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        }
        
        SDL_Rect heart = {heartX, startY, heartSize, heartSize};
        SDL_RenderFillRect(renderer, &heart);
        
        // Draw heart outline
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &heart);
    }
}

void Game::renderGameOver() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Game Over box
    int boxWidth = 500;
    int boxHeight = 350;
    int boxX = (SCREEN_WIDTH - boxWidth) / 2;
    int boxY = (SCREEN_HEIGHT - boxHeight) / 2;
    
    SDL_SetRenderDrawColor(renderer, 69, 69, 69, 255);
    SDL_Rect gameOverBox = {boxX, boxY, boxWidth, boxHeight};
    SDL_RenderFillRect(renderer, &gameOverBox);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {boxX + i, boxY + i, boxWidth - i*2, boxHeight - i*2};
        SDL_RenderDrawRect(renderer, &border);
    }
    
    SDL_Color red = {255, 50, 50, 255};
    textManager->renderText(renderer, "GAME OVER", "title", 
                           SCREEN_WIDTH / 2, boxY + 60, red, true);
    
    SDL_Color gold = {255, 215, 0, 255};
    textManager->renderText(renderer, "Cookies Collected", "normal", 
                           SCREEN_WIDTH / 2, boxY + 140, gold, true);
    
    // Cookie icon
    SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
    SDL_Rect cookieIcon = {SCREEN_WIDTH / 2 - 15, boxY + 180, 30, 30};
    SDL_RenderFillRect(renderer, &cookieIcon);
    
    // Cookies Collected
    SDL_Color white = {255, 255, 255, 255};
    char countText[32];
    sprintf(countText, "%d", totalCookiesCollected);
    textManager->renderText(renderer, countText, "title", 
                           SCREEN_WIDTH / 2, boxY + 235, white, true);
    
    // "Press R to Restart" prompt
    Uint32 ticks = SDL_GetTicks();
    bool showText = (ticks / 500) % 2 == 0;
    
    if (showText) {
        SDL_Color gray = {200, 200, 200, 255};
        textManager->renderText(renderer, "Press R to Restart", "normal", 
                               SCREEN_WIDTH / 2, boxY + boxHeight - 50, gray, true);
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
    
    // Render player (only if not dead)
    if (!player->isDead) {
        player->render(renderer);
    }
    
    // Render cookie counter
    for (int i = 0; i < cookieCount; i++) {
        SDL_Rect cookieIcon = {10 + i * 25, 10, 20, 20};
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
        SDL_RenderFillRect(renderer, &cookieIcon);
    }
    
    // Render hearts
    renderHearts();
    
    // Render game over screen if dead (change later with background demo gameplay while in end screen)
    if (player->isDead) {
        renderGameOver();
    }
    
    SDL_RenderPresent(renderer);
}

void Game::run() {
    Uint32 frameStart;
    int frameTime;
    
    while (running) {
        frameStart = SDL_GetTicks();
        
        handleEvents();
        
        // !update if player is dead (but still render)
        if (!player->isDead) {
            update();
        }
        
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
    
    if (textManager) {
        delete textManager;
        textManager = nullptr;
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