#include "game.h"
#include "constants.h"
#include <cstdio>
#include <cmath>

Game::Game() {
    window = nullptr;
    renderer = nullptr;
    running = false;
    player = nullptr;
    textManager = nullptr;
    levelManager = nullptr;
    downwellGenerator = nullptr;
    currentRun = nullptr;
    currentState = STATE_LOBBY;
    previousState = STATE_LOBBY;
    cameraY = 0;
    worldHeight = 0;
    playerNearStartDoor = false;
    currentSegment = 0;
    currentBombJackLevel = nullptr;
    bombJackCookiesCollected = 0;
    selectedUpgradeIndex = 0;
    hasJumpedThisPress = false;
    hasInteractedThisPress = false;
    transitionTimer = 0;
    playerReturnX = 0;
    playerReturnY = 0;
}

Game::~Game() {
    clean();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    
    window = SDL_CreateWindow("Cookie Thief",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    if (!window) {
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        return false;
    }
    
    textManager = new TextManager();
    if (!textManager->init()) {
        printf("Failed to initialize TextManager!\n");
        return false;
    }
    
    textManager->loadFont("title", "PressStart2P.ttf", 32);
    textManager->loadFont("normal", "PressStart2P.ttf", 16);
    textManager->loadFont("small", "PressStart2P.ttf", 12);
    
    player = new Player(100, 100);
    levelManager = new LevelManager();
    levelManager->initializeLevels();
    downwellGenerator = new DownwellGenerator();
    currentRun = new GameRun();
    
    loadLobby();
    
    running = true;
    return true;
}

void Game::loadLobby() {
    currentState = STATE_LOBBY;
    cleanCurrentLevel();
    
    // lobby platforms
    platforms.push_back({0, 550, 800, 50, {100, 100, 100, 255}});
    platforms.push_back({300, 450, 200, 20, {139, 69, 19, 255}});
    
    player->reset(400, 400);
    cameraY = 0;
    
    // start door
    startRunDoor = {350, 380, 100, 90};
    // TODO: move persistant stats into a seperate stats page
    printf("=== LOBBY LOADED ===\n");
    printf("Persistent Stats:\n");
    printf("  Total Cookies: %d\n", persistentStats.totalCookies);
    printf("  Total Deaths: %d\n", persistentStats.totalDeaths);
    printf("  Highest Floor: %d\n", persistentStats.highestFloorReached);
    printf("  Total Playthroughs: %d\n", persistentStats.totalPlaythroughs);
}

void Game::startNewRun() {
    printf("\n=== STARTING NEW RUN ===\n");
    currentRun->startNewRun();
    persistentStats.totalPlaythroughs++;
    
    player->hearts = STARTING_HEARTS;
    player->maxHearts = STARTING_HEARTS;
    player->energy = MAX_ENERGY;
    player->maxEnergy = MAX_ENERGY;
    player->isDead = false;
    player->isInvincible = false;
    player->invincibilityTimer = 0;
    
    currentState = STATE_RUN_INTRO;
    transitionTimer = 2.0f;
    currentSegment = 0;
}

void Game::generateDownwellSegment() {
    printf("\n=== GENERATING DOWNWELL SEGMENT %d ===\n", currentSegment + 1);
    cleanCurrentLevel();
    
    currentRun->advanceFloor();
    int difficulty = currentSegment;
    
    DownwellSegment segment = downwellGenerator->generateSegment(
        currentRun->getCurrentFloor(), 
        difficulty
    );
    
    platforms = segment.platforms;
    cookies = segment.cookies;
    enemies = segment.enemies;
    worldHeight = segment.segmentHeight;
    
    // Generate side doors (bomb jack/ shop doors)
    int doorCount = 1 + (difficulty / 2);
    if (doorCount > 3) doorCount = 3;
    
    for (int i = 0; i < doorCount; i++) {
        SideDoor door;
        door.worldY = (worldHeight / (doorCount + 1)) * (i + 1);
        
        bool onLeftWall = (rand() % 2 == 0);
        
        Platform alcove;
        if (onLeftWall) {
            alcove.x = PIT_LEFT - 60;
            alcove.width = 90;
        } else {
            alcove.x = PIT_RIGHT - 30;
            alcove.width = 90;
        }
        
        alcove.y = door.worldY;
        alcove.height = 20;
        alcove.color = {80, 60, 40, 255};
        
        platforms.push_back(alcove);
        
        door.rect = {
            onLeftWall ? (int)(PIT_LEFT - 55) : (int)(PIT_RIGHT + 5),
            (int)(alcove.y - 70),
            50,
            70
        };
        
        door.worldY = alcove.y;
        door.type = (i % 2 == 0) ? ROOM_BOMB_JACK : ROOM_SHOP;
        door.used = false;
        door.onLeftWall = onLeftWall;
        sideDoors.push_back(door);

        // collision when jumping into ceiling above aclove
        Collider ceiling;
        ceiling.rect = {
            onLeftWall ? (int)(PIT_LEFT - 80) : (int)(PIT_RIGHT - 30),
            (int)(door.worldY - 110),
            90,
            20
        };
        ceiling.isWorldSpace = true;
        alcoveCeilings.push_back(ceiling);
    }
        
    // Start player in CENTER
    player->setPosition(PIT_LEFT + PIT_WIDTH / 2, 50);
    cameraY = 0;
    currentState = STATE_DOWNWELL;
    
    printf("Segment ready: Floor %d, %d platforms, %d cookies, %d enemies, %d doors\n",
           currentRun->getCurrentFloor(), (int)platforms.size(), (int)cookies.size(), 
           (int)enemies.size(), (int)sideDoors.size());
}

void Game::enterSideRoom(RoomType type) {
    printf("Entering side room type %d\n", type);
    
    // save position
    playerReturnX = player->x;
    playerReturnY = player->y;
    previousState = STATE_DOWNWELL;
    
    // save downwell state
    savedPlatforms = platforms;
    savedSideDoors = sideDoors;
    savedWorldHeight = worldHeight;
    savedAlcoveCeilings = alcoveCeilings;
    
    // copy cookies (ptr)
    savedCookies.clear();
    for (auto* cookie : cookies) {
        if (!cookie->collected) {
            savedCookies.push_back(new Cookie(cookie->x, cookie->y));
        }
    }
    
    // copy enemies (ptr)
    savedEnemies.clear();
    for (auto* enemy : enemies) {
        savedEnemies.push_back(new Enemy(enemy->x, enemy->y));
    }
    
    // enter bomb jack side level or enter shop
    if (type == ROOM_BOMB_JACK) {
        int levelIndex = currentSegment % levelManager->getLevelCount();
        currentBombJackLevel = levelManager->getLevel(levelIndex);
        
        if (currentBombJackLevel) {
            cleanCurrentLevel();
            
            platforms = currentBombJackLevel->platforms;
            for (auto* cookie : currentBombJackLevel->cookies) {
                cookies.push_back(new Cookie(cookie->x, cookie->y));
            }
            for (auto* enemy : currentBombJackLevel->enemies) {
                enemies.push_back(new Enemy(enemy->x, enemy->y));
            }
            
            player->setPosition(currentBombJackLevel->playerStartX, currentBombJackLevel->playerStartY);
            bombJackCookiesCollected = 0;
            cameraY = 0;
            currentState = STATE_BOMB_JACK;
        }
    } else if (type == ROOM_SHOP) {
        shopIsFromSideRoom = true;
        currentState = STATE_SHOP;
        selectedUpgradeIndex = 0;
    }
    
    hasInteractedThisPress = true;
}

void Game::exitSideRoom() {
    printf("Exiting side room\n");
    //TODO: add invinsability briefly after leaving

    // mark door as used
    for (auto& door : savedSideDoors) {
        if (fabs(door.worldY - playerReturnY) < 100) {
            if (door.type == ROOM_BOMB_JACK) {
                door.used = true;
            }
            break;
        }
    }
    
    // restore state
    cleanCurrentLevel();
    
    platforms = savedPlatforms;
    sideDoors = savedSideDoors;
    worldHeight = savedWorldHeight;
    alcoveCeilings = savedAlcoveCeilings;
    
    // Restore cookies
    cookies.clear();
    for (auto* cookie : savedCookies) {
        cookies.push_back(new Cookie(cookie->x, cookie->y));
    }
    
    // Restore enemies  
    enemies.clear();
    for (auto* enemy : savedEnemies) {
        enemies.push_back(new Enemy(enemy->x, enemy->y));
    }
    
    // Clear saved state
    savedCookies.clear();
    savedEnemies.clear();
    
    // Teleport player back
    player->setPosition(playerReturnX, playerReturnY);
    
    // Center camera on player
    cameraY = playerReturnY - SCREEN_HEIGHT / 2;
    if (cameraY < 0) cameraY = 0;
    if (cameraY > worldHeight - SCREEN_HEIGHT) {
        cameraY = worldHeight - SCREEN_HEIGHT;
    }
    
    currentState = STATE_DOWNWELL;
}

void Game::completeDownwellSegment() {
    printf("\n=== DOWNWELL SEGMENT COMPLETE ===\n");
    printf("Floor %d cleared!\n", currentRun->getCurrentFloor());
    
    currentRun->getStats().downwellLevelsCleared++;
    currentState = STATE_DOWNWELL_COMPLETE;
}

void Game::endRun(bool victory) {
    printf("\n=== RUN ENDED ===\n");
    
    // Update persistent stats
    RunStats& runStats = currentRun->getStats();
    persistentStats.totalCookies += runStats.cookiesThisRun;
    persistentStats.totalDistanceFell += runStats.distanceFell;
    persistentStats.totalJumps += runStats.jumpsThisRun;
    persistentStats.downwellLevelsCleared += runStats.downwellLevelsCleared;
    persistentStats.bombJackLevelsCleared += runStats.bombJackLevelsCleared;
    
    if (!victory) {
        persistentStats.totalDeaths++;
    }
    
    if (currentRun->getCurrentFloor() > persistentStats.highestFloorReached) {
        persistentStats.highestFloorReached = currentRun->getCurrentFloor();
    }
    
    currentRun->endRun();
    currentState = victory ? STATE_RUN_COMPLETE : STATE_GAME_OVER;
}

void Game::applyUpgradesToPlayer() {
    // Apply all purchased upgrades to the player
    // TODO: implement all upgrades
    player->maxHearts = STARTING_HEARTS + currentRun->getBonusHearts();
    if (player->hearts > player->maxHearts) {
        player->hearts = player->maxHearts;
    }
    
    player->maxEnergy = MAX_ENERGY + currentRun->getMaxEnergyBonus();
    if (player->energy > player->maxEnergy) {
        player->energy = player->maxEnergy;
    }
}

void Game::cleanCurrentLevel() {
    platforms.clear();
    sideDoors.clear();
    alcoveCeilings.clear();
    
    for (auto* cookie : cookies) {
        delete cookie;
    }
    cookies.clear();
    
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
}

SDL_Rect Game::worldToScreen(SDL_Rect worldRect) {
    return {
        worldRect.x,
        (int)(worldRect.y - cameraY),
        worldRect.w,
        worldRect.h
    };
}

float Game::worldToScreenY(float worldY) {
    return worldY - cameraY;
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
    
    return false;
}

void Game::handleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        
        // check for restart when dead
        if (player->isDead && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_r) {
                loadLobby();
            }
            continue;
        }
        
        if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            // start run from lobby
            if (currentState == STATE_LOBBY && event.key.keysym.sym == SDLK_e) {
                if (playerNearStartDoor) {
                    startNewRun();
                }
            }
            
            // end run go lobby
            if (currentState == STATE_RUN_COMPLETE || currentState == STATE_GAME_OVER) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    loadLobby();
                }
            }
            
            // downwell level to between level shop
            if (currentState == STATE_DOWNWELL_COMPLETE && event.key.keysym.sym == SDLK_SPACE) {
                currentSegment++;
                currentState = STATE_SHOP;
                selectedUpgradeIndex = 0;
                shopIsFromSideRoom = false;
            }
            
            // shop for in level and between level
            if (currentState == STATE_SHOP) {
                const std::vector<Upgrade>& upgrades = currentRun->getAvailableUpgrades();
                
                if (event.key.keysym.sym == SDLK_UP) {
                    selectedUpgradeIndex--;
                    if (selectedUpgradeIndex < 0) selectedUpgradeIndex = upgrades.size();
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    selectedUpgradeIndex++;
                    if (selectedUpgradeIndex > (int)upgrades.size()) selectedUpgradeIndex = 0;
                }
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    if (selectedUpgradeIndex == (int)upgrades.size()) {
                        // "Continue"
                        if (shopIsFromSideRoom) {
                            exitSideRoom();
                        } else {
                            currentSegment++;
                            generateDownwellSegment();
                        }
                    } else if (selectedUpgradeIndex < (int)upgrades.size()) {
                        int runCookies = currentRun->getStats().cookiesThisRun;
                        
                        if (currentRun->purchaseUpgrade(upgrades[selectedUpgradeIndex].type, runCookies)) {
                            currentRun->getStats().cookiesThisRun = runCookies;
                            applyUpgradesToPlayer();
                            printf("Purchased upgrade! Cookies remaining: %d\n", runCookies);
                        } else {
                            printf("Not enough cookies! Need %d, have %d\n", 
                                upgrades[selectedUpgradeIndex].cost, runCookies);
                        }
                    }
                }
            }
            
            // leave bomb jack level when no more cookies
            if (currentState == STATE_BOMB_JACK && event.key.keysym.sym == SDLK_e) {
                if (currentBombJackLevel && bombJackCookiesCollected >= currentBombJackLevel->requiredCookies) {
                    currentRun->getStats().bombJackLevelsCleared++;
                    exitSideRoom();
                }
            }
            
            // side doors in downwell
            if (currentState == STATE_DOWNWELL && event.key.keysym.sym == SDLK_e && !hasInteractedThisPress) {
                SDL_Rect playerWorldRect = player->getRect();
                
                printf("Checking door interactions - Player at world Y: %.1f\n", player->y);
                
                for (auto& door : sideDoors) {
                    if (!door.used) {
                        SDL_Rect doorWorldRect = door.rect;
                        
                        printf("  Door at world Y: %.1f, door rect: %d,%d,%d,%d\n", 
                               door.worldY, doorWorldRect.x, doorWorldRect.y, doorWorldRect.w, doorWorldRect.h);
                        
                        if (SDL_HasIntersection(&playerWorldRect, &doorWorldRect)) {
                            printf("  ENTERING DOOR TYPE %d\n", door.type);
                            enterSideRoom(door.type);
                            break;
                        }
                    }
                }
                hasInteractedThisPress = true;
            }
            
            // jump or glide
            if (event.key.keysym.sym == SDLK_SPACE) {
                if (isPlayerOnGround()) {
                    player->onGround = true;
                    player->jump();
                    currentRun->getStats().jumpsThisRun++;
                    hasJumpedThisPress = true;
                } else if (!hasJumpedThisPress) {
                    player->startGliding();
                }
            }
        }
        
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_SPACE) {
                hasJumpedThisPress = false;
                player->stopGliding();
            }
            if (event.key.keysym.sym == SDLK_e) {
                hasInteractedThisPress = false;
            }
        }
    }
    
    // movement
    if (!player->isDead && (currentState == STATE_DOWNWELL || currentState == STATE_BOMB_JACK || currentState == STATE_LOBBY)) {
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A]) {
            player->moveLeft();
        } else if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D]) {
            player->moveRight();
        } else {
            player->stopMoving();
        }
    }
}

void Game::update() {
    if (currentState == STATE_RUN_INTRO) {
        transitionTimer -= 1.0f / FPS;
        if (transitionTimer <= 0) {
            generateDownwellSegment();
        }
        return;
    }
    
    if (currentState == STATE_LOBBY) {
        updateLobby();
    } else if (currentState == STATE_DOWNWELL) {
        updateDownwell();
    } else if (currentState == STATE_BOMB_JACK) {
        updateBombJack();
    }
    
    if (player->onGround) {
        hasJumpedThisPress = false;
    }
}

void Game::updateLobby() {
    player->update();
    checkPlatformCollisions();
    
    // near start door in lobby
    SDL_Rect playerRect = player->getRect();
    playerNearStartDoor = SDL_HasIntersection(&playerRect, &startRunDoor);
}

void Game::updateBombJack() {
    player->update();
    checkPlatformCollisions();
    checkCookieCollisions();
    checkEnemyCollisions();
    
    for (auto* enemy : enemies) {
        enemy->update(*player);
    }
    
    // bomb jack win condition
    if (currentBombJackLevel && bombJackCookiesCollected >= currentBombJackLevel->requiredCookies) {
        printf("All cookies collected! Auto-exiting Bomb Jack level...\n");
        currentRun->getStats().bombJackLevelsCleared++;
        exitSideRoom();
        return;
    }
    
    if (player->isDead) {
        endRun(false);
    }
}

void Game::checkPlatformCollisions() {
    player->onGround = false;
    
    SDL_Rect playerRect = player->getRect();
    int playerLeft = playerRect.x;
    int playerRight = playerRect.x + playerRect.w;
    int playerTop = playerRect.y;
    int playerBottom = playerRect.y + playerRect.h;
    
    int prevPlayerLeft = playerLeft - player->velocityX;
    int prevPlayerRight = playerRight - player->velocityX;
    int prevPlayerTop = playerTop - player->velocityY;
    int prevPlayerBottom = playerBottom - player->velocityY;
    
    for (auto& platform : platforms) {
        SDL_Rect platformRect = platform.getRect();
        int platformLeft = platformRect.x;
        int platformRight = platformRect.x + platformRect.w;
        int platformTop = platformRect.y;
        int platformBottom = platformRect.y + platformRect.h;
        
        if (SDL_HasIntersection(&playerRect, &platformRect)) {
            // can phase through platforms from bellow only
            if (player->velocityY >= 0 && prevPlayerBottom <= platformTop + 3) {
                player->y = platformTop - player->height;
                player->velocityY = 0;
                player->onGround = true;
            }
            else if (playerBottom > platformTop + 3 && playerTop < platformBottom) {
                if (player->velocityX > 0 && prevPlayerRight <= platformLeft + 2) {
                    player->x = platformLeft - player->width;
                    player->velocityX = 0;
                }
                else if (player->velocityX < 0 && prevPlayerLeft >= platformRight - 2) {
                    player->x = platformRight;
                    player->velocityX = 0;
                }
            }
        }
    }
    
    // downwell walls
    if (currentState == STATE_DOWNWELL) {
        std::vector<SDL_Rect> leftAlcoveZones;
        std::vector<SDL_Rect> rightAlcoveZones;
        
        for (const auto& door : sideDoors) {
            SDL_Rect alcoveZone = {
                door.onLeftWall ? (int)(PIT_LEFT - 80) : (int)PIT_RIGHT,
                (int)(door.worldY - 90),
                80,
                130
            };
            
            if (door.onLeftWall) {
                leftAlcoveZones.push_back(alcoveZone);
            } else {
                rightAlcoveZones.push_back(alcoveZone);
            }
        }
        
        SDL_Rect playerRect = player->getRect();
        
        if (player->x < PIT_LEFT) {
            bool inLeftAlcove = false;
            for (const auto& zone : leftAlcoveZones) {
                if (SDL_HasIntersection(&playerRect, &zone)) {
                    inLeftAlcove = true;
                    break;
                }
            }
            
            // alclove depth (width)
            if (inLeftAlcove) {
                if (player->x < PIT_LEFT - 80) {
                    player->x = PIT_LEFT - 80;
                    player->velocityX = 0;
                }
            } else {
                // default
                player->x = PIT_LEFT;
                player->velocityX = 0;
            }
        }
        
        if (player->x + player->width > PIT_RIGHT) {
            bool inRightAlcove = false;
            for (const auto& zone : rightAlcoveZones) {
                if (SDL_HasIntersection(&playerRect, &zone)) {
                    inRightAlcove = true;
                    break;
                }
            }
            // alclove depth (width)
            if (inRightAlcove) {
                if (player->x + player->width > PIT_RIGHT + 80) {
                    player->x = PIT_RIGHT + 80 - player->width;
                    player->velocityX = 0;
                }
            } else {
                // default
                player->x = PIT_RIGHT - player->width;
                player->velocityX = 0;
            }
        }
    }
        
    // bomb jack level box
    if (currentState == STATE_BOMB_JACK) {
        if (player->x < ARENA_LEFT + 15) {
            player->x = ARENA_LEFT + 15;
            player->velocityX = 0;
        }
        if (player->x + player->width > ARENA_LEFT + ARENA_WIDTH - 15) {
            player->x = ARENA_LEFT + ARENA_WIDTH - 15 - player->width;
            player->velocityX = 0;
        }
        if (player->y < ARENA_TOP + 15) {
            player->y = ARENA_TOP + 15;
            player->velocityY = 0;
        }
        if (player->y + player->height > ARENA_TOP + ARENA_HEIGHT - 15) {
            player->y = ARENA_TOP + ARENA_HEIGHT - 15 - player->height;
            player->velocityY = 0;
            player->onGround = true;
        }
    }
    
    // lobby screen
    if (currentState == STATE_LOBBY) {
        if (player->x < 0) {
            player->x = 0;
            player->velocityX = 0;
        }
        if (player->x + player->width > SCREEN_WIDTH) {
            player->x = SCREEN_WIDTH - player->width;
            player->velocityX = 0;
        }
        if (player->y + player->height >= SCREEN_HEIGHT - 1) {
            player->y = SCREEN_HEIGHT - player->height;
            player->velocityY = 0;
            player->onGround = true;
        }
    }
}

void Game::checkAlcoveCeilingCollisions() {
    SDL_Rect playerRect = player->getRect();
    
    for (const auto& ceiling : alcoveCeilings) {
        SDL_Rect ceilingRect = ceiling.rect;
        
        // ceiling for aclove
        if (player->velocityY < 0 && SDL_HasIntersection(&playerRect, &ceilingRect)) {
            player->y = ceilingRect.y + ceilingRect.h;
            player->velocityY = 0;
            break;
        }
    }
}

void Game::checkCookieCollisions() {
    for (auto* cookie : cookies) {
        if (!cookie->collected && cookie->checkCollision(*player)) {
            cookie->collected = true;
            persistentStats.totalCookies++;
            currentRun->getStats().cookiesThisRun++;
            
            if (currentState == STATE_BOMB_JACK) {
                bombJackCookiesCollected++;
            }
            
            player->restoreEnergy(COOKIE_ENERGY_RESTORE);
            printf("Cookie! Total: %d, This run: %d\n", 
                   persistentStats.totalCookies, currentRun->getStats().cookiesThisRun);
        }
    }
}

void Game::checkEnemyCollisions() {
    for (auto* enemy : enemies) {
        if (enemy->checkCollision(*player)) {
            if (!player->isInvincible) {
                player->loseHeart();
            }
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_RenderClear(renderer);
    
    if (currentState == STATE_LOBBY) {
        renderLobby();
    } else if (currentState == STATE_RUN_INTRO) {
        renderRunIntro();
    } else if (currentState == STATE_DOWNWELL) {
        renderDownwell();
    } else if (currentState == STATE_BOMB_JACK) {
        renderBombJack();
    } else if (currentState == STATE_SHOP) {
        renderShop();
    } else if (currentState == STATE_DOWNWELL_COMPLETE) {
        renderDownwellComplete();
    } else if (currentState == STATE_RUN_COMPLETE) {
        renderRunComplete();
    } else if (currentState == STATE_GAME_OVER) {
        renderGameOver();
    }
    
    SDL_RenderPresent(renderer);
}

void Game::renderLobby() {
    // Platforms
    for (const auto& platform : platforms) {
        platform.render(renderer);
    }
    
    // Door
    SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
    SDL_RenderFillRect(renderer, &startRunDoor);
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderDrawRect(renderer, &startRunDoor);
    
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_Rect knob = {startRunDoor.x + startRunDoor.w - 15, startRunDoor.y + startRunDoor.h / 2 - 5, 10, 10};
    SDL_RenderFillRect(renderer, &knob);
    
    if (playerNearStartDoor) {
        SDL_Color white = {255, 255, 255, 255};
        textManager->renderText(renderer, "Press E to Start Heist", "small", 
                               startRunDoor.x + startRunDoor.w / 2, startRunDoor.y - 30, white, true);
    }
    
    player->render(renderer, false);
    
    // Stats
    SDL_Color gold = {255, 215, 0, 255};
    textManager->renderText(renderer, "COOKIE THIEF HQ", "normal", SCREEN_WIDTH / 2, 30, gold, true);
    
    SDL_Color white = {255, 255, 255, 255};
    char text[64];
    sprintf(text, "Total Cookies: %d", persistentStats.totalCookies);
    textManager->renderText(renderer, text, "small", 10, 70, white);
    sprintf(text, "Highest Floor: %d", persistentStats.highestFloorReached);
    textManager->renderText(renderer, text, "small", 10, 95, white);
    sprintf(text, "Total Deaths: %d", persistentStats.totalDeaths);
    textManager->renderText(renderer, text, "small", 10, 120, white);
}

void Game::renderRunIntro() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color red = {255, 100, 100, 255};
    textManager->renderText(renderer, "STEALING THE RECIPE...", "title", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, red, true);
    
    SDL_Color white = {255, 255, 255, 255};
    textManager->renderText(renderer, "ALARM TRIGGERED!", "normal", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 30, white, true);
}


void Game::updateDownwell() {
    float oldY = player->y;
    player->update();
    checkPlatformCollisions();
    checkAlcoveCeilingCollisions();
    checkCookieCollisions();
    checkEnemyCollisions();
    
    // Center camera on player vertically
    float targetCameraY = player->y - SCREEN_HEIGHT / 2;
    
    // smooth camera
    float lerpFactor = 0.2f;
    cameraY += (targetCameraY - cameraY) * lerpFactor;
    
    // Clamp camera
    if (cameraY < 0) cameraY = 0;
    if (cameraY > worldHeight - SCREEN_HEIGHT) {
        cameraY = worldHeight - SCREEN_HEIGHT;
    }
    
    // Update enemies
    for (auto* enemy : enemies) {
        enemy->update(*player);
    }
    
    // Check if reached bottom
    // TODO : fix buffer for bottom sprite when added
    // platforms, enemies and cookies shouldnt spawn x from bottom
    if (player->y >= worldHeight - 200) {
        completeDownwellSegment();
    }
    
    // distance tracker (down)
    if (player->y > oldY) {
        currentRun->getStats().distanceFell += (int)(player->y - oldY);
    }
    
    if (player->isDead) {
        endRun(false);
    }
}

void Game::renderDownwellWalls() {
    // dark walls
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    
    SDL_Rect leftWall = {0, 0, PIT_LEFT, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &leftWall);
    
    SDL_Rect rightWall = {PIT_RIGHT, 0, SCREEN_WIDTH - PIT_RIGHT, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &rightWall);
    
    // wall edge
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(renderer, PIT_LEFT, 0, PIT_LEFT, SCREEN_HEIGHT);
    SDL_RenderDrawLine(renderer, PIT_RIGHT, 0, PIT_RIGHT, SCREEN_HEIGHT);
}

void Game::renderDownwell() {
    renderDownwellWalls();

    for (const auto& door : sideDoors) {
    SDL_Rect screenDoor = worldToScreen(door.rect);
    
    if (screenDoor.y + screenDoor.h >= 0 && screenDoor.y <= SCREEN_HEIGHT) {
        if (door.used) {
            // bomb jack door
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderFillRect(renderer, &screenDoor);
            
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            SDL_RenderDrawLine(renderer, screenDoor.x, screenDoor.y,
                             screenDoor.x + screenDoor.w, screenDoor.y + screenDoor.h);
            SDL_RenderDrawLine(renderer, screenDoor.x + screenDoor.w, screenDoor.y,
                             screenDoor.x, screenDoor.y + screenDoor.h);
        } else {
            // active door
            if (door.type == ROOM_SHOP) {
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // ywllow: shops
            } else {
                SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // brown: bomb Jack
            }
            SDL_RenderFillRect(renderer, &screenDoor);
            
            // Door
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &screenDoor);
            
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect knob = {
                door.onLeftWall ? screenDoor.x + screenDoor.w - 10 : screenDoor.x + 5,
                screenDoor.y + screenDoor.h / 2 - 3,
                6, 6
            };
            SDL_RenderFillRect(renderer, &knob);
            
            // promp in proximity
            SDL_Rect playerScreenRect = {
                (int)player->x, 
                (int)(player->y - cameraY), 
                (int)player->width, 
                (int)player->height
            };
            
            if (SDL_HasIntersection(&playerScreenRect, &screenDoor)) {
                SDL_Color white = {255, 255, 255, 255};
                const char* label = door.type == ROOM_SHOP ? "SHOP [E]" : "LEVEL [E]";
                textManager->renderText(renderer, label, "small", 
                                       screenDoor.x + screenDoor.w / 2, 
                                       screenDoor.y - 15, white, true);
            }
        }
    }
}
    // NOW RENDER

    // platforms
    for (const auto& platform : platforms) {
        SDL_Rect screenRect = worldToScreen(platform.getRect());
        
        if (screenRect.y + screenRect.h >= 0 && screenRect.y <= SCREEN_HEIGHT) {
            SDL_SetRenderDrawColor(renderer, platform.color.r, platform.color.g, platform.color.b, platform.color.a);
            SDL_RenderFillRect(renderer, &screenRect);
        }
    }
    
    // cookies
    for (auto* cookie : cookies) {
        if (!cookie->collected) {
            SDL_Rect worldRect = cookie->getRect();
            SDL_Rect screenRect = worldToScreen(worldRect);
            
            if (screenRect.y + screenRect.h >= 0 && screenRect.y <= SCREEN_HEIGHT) {
                SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
                SDL_RenderFillRect(renderer, &screenRect);
            }
        }
    }
    
    // enemies
    for (auto* enemy : enemies) {
        SDL_Rect worldRect = enemy->getRect();
        SDL_Rect screenRect = worldToScreen(worldRect);
        
        if (screenRect.y + screenRect.h >= 0 && screenRect.y <= SCREEN_HEIGHT) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(renderer, &screenRect);
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect hat = {screenRect.x + 5, screenRect.y - 10, 22, 10};
            SDL_RenderFillRect(renderer, &hat);
        }
    }
    
    // side doors
    for (const auto& door : sideDoors) {
    SDL_Rect screenDoor = worldToScreen(door.rect);
    
    if (screenDoor.y + screenDoor.h >= 0 && screenDoor.y <= SCREEN_HEIGHT) {
            if (door.used) {
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            } else {
                if (door.type == ROOM_SHOP) {
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // ywllow: shops
                } else {
                SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // brown: bomb Jack
                }
            SDL_RenderFillRect(renderer, &screenDoor);
            
            // Door
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &screenDoor);
            
            if (!door.used) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_Rect knob = {
                    door.onLeftWall ? screenDoor.x + screenDoor.w - 10 : screenDoor.x + 5,
                    screenDoor.y + screenDoor.h / 2 - 3,
                    6, 6
                };
                SDL_RenderFillRect(renderer, &knob);
            }
            
            // promp in proximity
            if (!door.used) {
                SDL_Rect playerScreenRect = {
                    (int)player->x, 
                    (int)(player->y - cameraY), 
                    (int)player->width, 
                    (int)player->height
                };
                
                if (SDL_HasIntersection(&playerScreenRect, &screenDoor)) {
                    SDL_Color white = {255, 255, 255, 255};
                    const char* label = door.type == ROOM_SHOP ? "SHOP [E]" : "LEVEL [E]";
                    textManager->renderText(renderer, label, "small", 
                                        screenDoor.x + screenDoor.w / 2, 
                                        screenDoor.y - 15, white, true);
                }
            }
        }
    }
    
    // player
    SDL_Rect playerScreenRect = {
        (int)player->x,
        (int)(player->y - cameraY),
        (int)player->width,
        (int)player->height
    };
    
    if (!player->isDead) {
        if (player->isInvincible) {
            int blinkInterval = (int)(player->invincibilityTimer * 10) % 2;
            if (blinkInterval == 0) {
                renderUI();
                renderSideStats();
                return;
            }
        }

        if (player->isGliding) {
            SDL_SetRenderDrawColor(renderer, 191, 64, 191, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 112, 41, 99, 255);
        }
        SDL_RenderFillRect(renderer, &playerScreenRect);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect hair = {playerScreenRect.x, playerScreenRect.y, playerScreenRect.w, playerScreenRect.h / 3};
        SDL_RenderFillRect(renderer, &hair);

        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
        SDL_Rect face = {playerScreenRect.x + 5, playerScreenRect.y + 5, 22, 15};
        SDL_RenderFillRect(renderer, &face);

        // glide bar
        int barWidth = 30;
        int barHeight = 4;
        int barX = playerScreenRect.x + 1;
        int barY = playerScreenRect.y - 10;
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect barBg = {barX, barY, barWidth, barHeight};
        SDL_RenderFillRect(renderer, &barBg);

        float glidePercent = player->glideTime / MAX_GLIDE_TIME;
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_Rect glideMeter = {barX, barY, (int)(barWidth * glidePercent), barHeight};
        SDL_RenderFillRect(renderer, &glideMeter);
        
        // energy bar
        int energyBarY = barY - 8;
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect energyBarBg = {barX, energyBarY, barWidth, barHeight};
        SDL_RenderFillRect(renderer, &energyBarBg);
        
        float energyPercent = player->energy / player->maxEnergy;
        if (energyPercent > 0.5f) {
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        } else if (energyPercent > 0.25f) {
            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
        } else if (energyPercent > 0) {
            SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        }
        SDL_Rect energyMeter = {barX, energyBarY, (int)(barWidth * energyPercent), barHeight};
        SDL_RenderFillRect(renderer, &energyMeter);
    }
    
    renderUI();
    renderSideStats();
    }
}

void Game::renderSideStats() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255};
    
    // stats
    char text[64];
    int leftX = 20;
    int y = 80;
    
    textManager->renderText(renderer, "FLOOR", "small", leftX, y, gold);
    y += 25;
    sprintf(text, "%d", currentRun->getCurrentFloor());
    textManager->renderText(renderer, text, "normal", leftX, y, white);
    y += 50;
    
    textManager->renderText(renderer, "COOKIES", "small", leftX, y, gold);
    y += 25;
    sprintf(text, "%d", currentRun->getStats().cookiesThisRun);
    textManager->renderText(renderer, text, "normal", leftX, y, white);
    y += 50;
    
    // stage progression/ depth %
    float depthPercent = 0;
    if (worldHeight > 0) {
        depthPercent = (player->y / worldHeight) * 100.0f;
        if (depthPercent > 100) depthPercent = 100;
    }
    textManager->renderText(renderer, "DEPTH", "small", leftX, y, gold);
    y += 25;
    sprintf(text, "%.0f%%", depthPercent);
    textManager->renderText(renderer, text, "normal", leftX, y, white);
}

void Game::renderBombJack() {
    for (const auto& platform : platforms) {
        platform.render(renderer);
    }
    
    for (auto* cookie : cookies) {
        cookie->render(renderer);
    }
    
    for (auto* enemy : enemies) {
        enemy->render(renderer);
    }
    
    player->render(renderer, true);
    
    // cookie progress in bomb jack level
    if (currentBombJackLevel) {
        SDL_Color white = {255, 255, 255, 255};
        char text[32];
        sprintf(text, "Cookies: %d/%d", bombJackCookiesCollected, currentBombJackLevel->requiredCookies);
        textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, 30, white, true);
    }
    
    renderHearts();
}

void Game::renderShop() {
    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color gold = {255, 215, 0, 255};
    textManager->renderText(renderer, "UPGRADE SHOP", "title", SCREEN_WIDTH / 2, 50, gold, true);
    
    SDL_Color white = {255, 255, 255, 255};
    char cookieText[32];
    // USE RUN COOKIES, NOT PERSISTENT COOKIES //////////////////////////////////////////////////////
    sprintf(cookieText, "Cookies: %d", currentRun->getStats().cookiesThisRun);
    textManager->renderText(renderer, cookieText, "normal", SCREEN_WIDTH / 2, 100, white, true);
    
    const std::vector<Upgrade>& upgrades = currentRun->getAvailableUpgrades();
    
    int startY = 160;
    int spacing = 60;
    
    for (int i = 0; i < (int)upgrades.size(); i++) {
        const Upgrade& upgrade = upgrades[i];
        
        bool selected = (i == selectedUpgradeIndex);
        SDL_Color color = selected ? (SDL_Color){255, 255, 100, 255} : (SDL_Color){200, 200, 200, 255};
        
        if (upgrade.purchased) {
            color = {100, 255, 100, 255};
        }
        
        char line[128];
        if (upgrade.purchased) {
            sprintf(line, "[OWNED] %s", upgrade.name.c_str());
        } else {
            sprintf(line, "%s - %d cookies", upgrade.name.c_str(), upgrade.cost);
        }
        
        textManager->renderText(renderer, line, "small", 100, startY + i * spacing, color);
        
        if (selected && !upgrade.purchased) {
            SDL_Color desc = {180, 180, 180, 255};
            textManager->renderText(renderer, upgrade.description.c_str(), "small", 
                                   120, startY + i * spacing + 20, desc);
        }
    }
    
    // Continue option
    bool continueSelected = (selectedUpgradeIndex == (int)upgrades.size());
    SDL_Color continueColor = continueSelected ? (SDL_Color){100, 255, 100, 255} : (SDL_Color){200, 200, 200, 255};
    textManager->renderText(renderer, ">>> CONTINUE >>>", "normal", 
                           SCREEN_WIDTH / 2, startY + upgrades.size() * spacing + 40, continueColor, true);
    
    SDL_Color hint = {150, 150, 150, 255};
    textManager->renderText(renderer, "Arrow Keys + Enter", "small", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, hint, true);
}

void Game::renderDownwellComplete() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    
    SDL_Color green = {100, 255, 100, 255};
    textManager->renderText(renderer, "FLOOR CLEARED!", "title", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 50, green, true);
    
    SDL_Color white = {255, 255, 255, 255};
    char text[64];
    sprintf(text, "Floor %d Complete", currentRun->getCurrentFloor());
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, white, true);
    
    textManager->renderText(renderer, "Press SPACE for Shop", "normal", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 60, white, true);
}

void Game::renderRunComplete() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color gold = {255, 215, 0, 255};
    textManager->renderText(renderer, "RUN COMPLETE!", "title", SCREEN_WIDTH / 2, 80, gold, true);
    
    SDL_Color white = {255, 255, 255, 255};
    RunStats& stats = currentRun->getStats();
    
    int y = 180;
    char text[64];
    
    sprintf(text, "Floors Cleared: %d", currentRun->getCurrentFloor());
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 40;
    
    sprintf(text, "Cookies Collected: %d", stats.cookiesThisRun);
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 40;
    
    sprintf(text, "Distance Fell: %d", stats.distanceFell);
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 40;
    
    sprintf(text, "Jumps: %d", stats.jumpsThisRun);
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    
    SDL_Color hint = {200, 200, 200, 255};
    textManager->renderText(renderer, "Press SPACE to return to lobby", "small", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60, hint, true);
}

void Game::renderGameOver() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color red = {255, 50, 50, 255};
    textManager->renderText(renderer, "CAUGHT!", "title", SCREEN_WIDTH / 2, 80, red, true);
    
    SDL_Color white = {255, 255, 255, 255};
    RunStats& stats = currentRun->getStats();
    
    int y = 180;
    char text[64];
    
    sprintf(text, "Made it to Floor: %d", currentRun->getCurrentFloor());
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 40;
    
    sprintf(text, "Cookies Stolen: %d", stats.cookiesThisRun);
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 40;
    
    sprintf(text, "Distance Fell: %d", stats.distanceFell);
    textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    
    SDL_Color hint = {200, 200, 200, 255};
    textManager->renderText(renderer, "Press R to return to lobby", "small", 
                           SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60, hint, true);
}

void Game::renderHearts() {
    int heartSize = 20;
    int heartSpacing = 25;
    int startX = SCREEN_WIDTH - 10 - (player->maxHearts * heartSpacing);
    int startY = 10;
    
    for (int i = 0; i < player->maxHearts; i++) {
        int heartX = startX + (i * heartSpacing);
        
        if (i < player->hearts) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        }
        
        SDL_Rect heart = {heartX, startY, heartSize, heartSize};
        SDL_RenderFillRect(renderer, &heart);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &heart);
    }
}

void Game::renderUI() {
    renderHearts();
    
    SDL_Color white = {255, 255, 255, 255};
    char text[64];
    
    sprintf(text, "Floor %d", currentRun->getCurrentFloor());
    textManager->renderText(renderer, text, "small", 10, 10, white);
    
    sprintf(text, "Cookies: %d", currentRun->getStats().cookiesThisRun);
    textManager->renderText(renderer, text, "small", 10, 30, white);
    
    float depthPercent = 0;
    if (worldHeight > 0) {
        depthPercent = (player->y / worldHeight) * 100.0f;
        if (depthPercent > 100) depthPercent = 100;
    }
    sprintf(text, "Depth: %.0f%%", depthPercent);
    textManager->renderText(renderer, text, "small", 10, 50, white);
}

void Game::run() {
    Uint32 frameStart;
    int frameTime;
    
    while (running) {
        frameStart = SDL_GetTicks();
        
        handleEvents();
        
        if (!player->isDead || currentState == STATE_LOBBY) {
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
    cleanCurrentLevel();

    for (auto* cookie : savedCookies) {
        delete cookie;
    }
    savedCookies.clear();
    
    for (auto* enemy : savedEnemies) {
        delete enemy;
    }
    savedEnemies.clear();
    
    if (player) {
        delete player;
        player = nullptr;
    }
    
    if (textManager) {
        delete textManager;
        textManager = nullptr;
    }
    
    if (levelManager) {
        delete levelManager;
        levelManager = nullptr;
    }
    
    if (downwellGenerator) {
        delete downwellGenerator;
        downwellGenerator = nullptr;
    }
    
    if (currentRun) {
        delete currentRun;
        currentRun = nullptr;
    }
    
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