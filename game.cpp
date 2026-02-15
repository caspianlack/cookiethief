/**
 * game.cpp - Core game logic for Cookie Thief
 *
 * Main responsibilities:
 * - Game state management (lobby, runs, levels, shops)
 * - Run progression and statistics tracking
 * - Collision detection and physics
 * - Rendering all game states
 * - Input handling
 */

#include "game.h"
#include "constants.h"
#include <cstdio>
#include <cmath>

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

/**
 * Constructor - Initialize all game state to defaults
 */
Game::Game()
{
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
    currentSegment = 0;
    currentBombJackLevel = nullptr;
    bombJackCookiesCollected = 0;
    selectedUpgradeIndex = 0;
    hasJumpedThisPress = false;
    hasInteractedThisPress = false;
    transitionTimer = 0;
    playerReturnX = 0;
    playerReturnY = 0;
    playerReturnX = 0;
    playerReturnY = 0;
    maxDepthReached = 0;
    interactionTimer = 0.0f;
}

Game::~Game()
{
    clean();
}

/**
 * Initialize SDL, create window/renderer, load fonts, set up managers
 * Returns: true on success, false on failure
 */
bool Game::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return false;
    }

    window = SDL_CreateWindow("Cookie Thief",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window)
    {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        return false;
    }

    // Initialize text rendering system
    textManager = new TextManager();
    if (!textManager->init())
    {
        printf("Failed to initialize TextManager!\n");
        return false;
    }

    // Initialize Texture Manager (Sprite System)
    textureManager = new TextureManager();
    if (!textureManager->init(renderer))
    {
        printf("Failed to initialize TextureManager!\n");
        return false;
    }

    // --- LOAD SPRITES FROM FILES ---
    
    // Player Sprite Sheet
    if (!textureManager->loadTexture("player", "assets/player_sheet.png"))
    {
        printf("WARNING: Failed to load player sprite sheet. Falling back to simple rects pending.\n");
    }

    // Enemy
    if (textureManager->loadTexture("enemy_sheet", "assets/enemy_sheet.png"))
    {
        // For now alias "enemy" to "enemy_sheet" or handle in render
        // Actually, let's keep "enemy" as the texture name if possible, or load straight to "enemy"
        // But the file is "enemy_sheet.png" (frame based).
        // Let's load as "enemy" for simplicity in existing code
        textureManager->loadTexture("enemy", "assets/enemy_sheet.png");
    }

    // Baker
    textureManager->loadTexture("baker", "assets/baker.png");

    // Items
    textureManager->loadTexture("cookie", "assets/cookie.png");
    textureManager->loadTexture("recipe", "assets/recipe.png");
    textureManager->loadTexture("platform", "assets/platform.png"); // If we want textured platforms later

    // Load fonts at different sizes for UI hierarchy
    textManager->loadFont("title", "PressStart2P.ttf", 32);
    textManager->loadFont("normal", "PressStart2P.ttf", 16);
    textManager->loadFont("small", "PressStart2P.ttf", 12);

    // Create core game objects
    player = new Player(100, 100);
    levelManager = new LevelManager();
    levelManager->initializeLevels();
    downwellGenerator = new DownwellGenerator();
    currentRun = new GameRun();

    // Start in lobby
    loadLobby();

    running = true;
    return true;
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

/**
 * Load the lobby - safe hub where players can view stats and start runs
 */
void Game::loadLobby()
{
    currentState = STATE_LOBBY;
    cleanCurrentLevel();

    // Create simple lobby platforms
    platforms.push_back({0, 550, 800, 50, {100, 100, 100, 255}}); // Ground
    platforms.push_back({300, 450, 200, 20, {139, 69, 19, 255}}); // Center platform

    player->reset(400, 400);
    cameraY = 0;

    // Position of the recipe to steal
    recipeRect = {385, 380, 30, 40}; // Small golden rectangle
    interactionTimer = 0.0f;
    playerNearRecipe = false;
}

/**
 * Start a new heist run - resets player stats and begins descent
 */
void Game::startNewRun()
{
    currentRun->startNewRun();
    persistentStats.totalPlaythroughs++;

    // Reset player to starting condition
    player->hearts = STARTING_HEARTS;
    player->maxHearts = STARTING_HEARTS;
    player->energy = MAX_ENERGY;
    player->maxEnergy = MAX_ENERGY;
    player->isDead = false;
    player->isInvincible = false;
    player->invincibilityTimer = 0;

    // Show "stealing recipe" transition
    currentState = STATE_RUN_INTRO;
    transitionTimer = 2.0f;
    currentSegment = 0;
}

/**
 * Generate a procedural Downwell segment with platforms, enemies, and side doors
 * Each segment gets progressively harder based on floor number
 */
void Game::generateDownwellSegment()
{
    cleanCurrentLevel();
    maxDepthReached = 0; // Reset depth tracker for new segment

    currentRun->advanceFloor();
    int difficulty = currentSegment;

    // Generate the vertical level
    DownwellSegment segment = downwellGenerator->generateSegment(
        currentRun->getCurrentFloor(),
        difficulty);

    platforms = segment.platforms;
    cookies = segment.cookies;
    enemies = segment.enemies;
    worldHeight = segment.segmentHeight;

    // Spawn The Baker (Chaser)
    // Start him above the screen so he drops in menacingly
    baker = new Enemy(PIT_LEFT + PIT_WIDTH / 2 - 30, -200, ENEMY_BAKER, difficulty);
    printf("THE BAKER IS COMING! Spawned at (%.1f, %.1f)\n", baker->x, baker->y);

    // Generate side doors to Bomb Jack levels or shops
    // More doors appear as difficulty increases
    int doorCount = 1 + (difficulty / 2);
    if (doorCount > 3)
        doorCount = 3;

    for (int i = 0; i < doorCount; i++)
    {
        SideDoor door;
        door.worldY = (worldHeight / (doorCount + 1)) * (i + 1);

        // Randomly place on left or right wall
        bool onLeftWall = (rand() % 2 == 0);

        // Create alcove platform for the door
        Platform alcove;
        if (onLeftWall)
        {
            alcove.x = PIT_LEFT - 60;
            alcove.width = 90;
        }
        else
        {
            alcove.x = PIT_RIGHT - 30;
            alcove.width = 90;
        }

        alcove.y = door.worldY;
        alcove.height = 20;
        alcove.color = {80, 60, 40, 255};

        platforms.push_back(alcove);

        // Position door on the alcove
        door.rect = {
            onLeftWall ? (int)(PIT_LEFT - 55) : (int)(PIT_RIGHT + 5),
            (int)(alcove.y - 70),
            50,
            70};

        door.worldY = alcove.y;
        door.type = (i % 2 == 0) ? ROOM_BOMB_JACK : ROOM_SHOP; // Alternate types
        door.used = false;
        door.onLeftWall = onLeftWall;
        sideDoors.push_back(door);

        // Add collision ceiling above alcove to prevent jumping through
        Collider ceiling;
        ceiling.rect = {
            onLeftWall ? (int)(PIT_LEFT - 80) : (int)(PIT_RIGHT - 30),
            (int)(door.worldY - 110),
            90,
            20};
        ceiling.isWorldSpace = true;
        alcoveCeilings.push_back(ceiling);
    }

    // Start player centered at top
    player->setPosition(PIT_LEFT + PIT_WIDTH / 2, 50);
    cameraY = 0;
    currentState = STATE_DOWNWELL;

    printf("Segment ready: Floor %d, %d platforms, %d cookies, %d enemies, %d doors\n",
           currentRun->getCurrentFloor(), (int)platforms.size(), (int)cookies.size(),
           (int)enemies.size(), (int)sideDoors.size());
}

/**
 * Enter a side room (Bomb Jack level or shop) from Downwell
 * Saves the current Downwell state to restore later
 */
void Game::enterSideRoom(RoomType type)
{
    printf("Entering side room type %d\n", type);

    // Save return position
    playerReturnX = player->x;
    playerReturnY = player->y;
    previousState = STATE_DOWNWELL;

    // Save entire Downwell state
    savedPlatforms = platforms;
    savedSideDoors = sideDoors;
    savedWorldHeight = worldHeight;
    savedAlcoveCeilings = alcoveCeilings;

    // Deep copy cookies (only uncollected ones)
    savedCookies.clear();
    for (auto *cookie : cookies)
    {
        if (!cookie->collected)
        {
            savedCookies.push_back(new Cookie(cookie->x, cookie->y));
        }
    }

    // Deep copy enemies
    savedEnemies.clear();
    for (auto *enemy : enemies)
    {
        savedEnemies.push_back(new Enemy(enemy->x, enemy->y));
    }

    // Load appropriate room type
    if (type == ROOM_BOMB_JACK)
    {
        // Load a Bomb Jack platforming challenge
        int levelIndex = currentSegment % levelManager->getLevelCount();
        currentBombJackLevel = levelManager->getLevel(levelIndex);

        if (currentBombJackLevel)
        {
            cleanCurrentLevel();

            platforms = currentBombJackLevel->platforms;
            for (auto *cookie : currentBombJackLevel->cookies)
            {
                cookies.push_back(new Cookie(cookie->x, cookie->y));
            }
            for (auto *enemy : currentBombJackLevel->enemies)
            {
                enemies.push_back(new Enemy(enemy->x, enemy->y));
            }

            player->setPosition(currentBombJackLevel->playerStartX, currentBombJackLevel->playerStartY);
            bombJackCookiesCollected = 0;
            cameraY = 0;
            currentState = STATE_BOMB_JACK;
        }
    }
    else if (type == ROOM_SHOP)
    {
        shopIsFromSideRoom = true;
        currentState = STATE_SHOP;
        selectedUpgradeIndex = 0;
    }

    hasInteractedThisPress = true;
}

/**
 * Exit side room and return to Downwell at saved position
 * Restores the Downwell state exactly as it was
 */
void Game::exitSideRoom()
{
    printf("Exiting side room\n");

    // Mark the door as used (Bomb Jack levels only - shops can be revisited)
    for (auto &door : savedSideDoors)
    {
        if (fabs(door.worldY - playerReturnY) < 100)
        {
            if (door.type == ROOM_BOMB_JACK)
            {
                door.used = true;
            }
            break;
        }
    }

    // Restore Downwell state
    cleanCurrentLevel();

    platforms = savedPlatforms;
    sideDoors = savedSideDoors;
    worldHeight = savedWorldHeight;
    alcoveCeilings = savedAlcoveCeilings;

    // Restore cookies
    cookies.clear();
    for (auto *cookie : savedCookies)
    {
        cookies.push_back(new Cookie(cookie->x, cookie->y));
    }

    // Restore enemies
    enemies.clear();
    for (auto *enemy : savedEnemies)
    {
        enemies.push_back(new Enemy(enemy->x, enemy->y));
    }

    // Clear saved state
    savedCookies.clear();
    savedEnemies.clear();

    // Teleport player back to where they entered
    player->setPosition(playerReturnX, playerReturnY);

    // Center camera on player
    cameraY = playerReturnY - SCREEN_HEIGHT / 2;
    if (cameraY < 0)
        cameraY = 0;
    if (cameraY > worldHeight - SCREEN_HEIGHT)
    {
        cameraY = worldHeight - SCREEN_HEIGHT;
    }

    currentState = STATE_DOWNWELL;
}

/**
 * Called when player reaches the bottom of a Downwell segment
 * Transitions to shop before next segment
 */
void Game::completeDownwellSegment()
{
    printf("\n=== DOWNWELL SEGMENT COMPLETE ===\n");
    printf("Floor %d cleared!\n", currentRun->getCurrentFloor());

    currentRun->getStats().downwellLevelsCleared++;
    currentState = STATE_DOWNWELL_COMPLETE;
}

/**
 * End the current run (either victory or death)
 * Updates persistent statistics and shows summary
 */
void Game::endRun(bool victory)
{
    printf("\n=== RUN ENDED ===\n");

    // Merge run stats into persistent stats
    RunStats &runStats = currentRun->getStats();
    persistentStats.totalCookies += runStats.cookiesThisRun;
    persistentStats.totalDistanceFell += runStats.distanceFell;
    persistentStats.totalJumps += runStats.jumpsThisRun;
    persistentStats.downwellLevelsCleared += runStats.downwellLevelsCleared;
    persistentStats.bombJackLevelsCleared += runStats.bombJackLevelsCleared;

    if (!victory)
    {
        persistentStats.totalDeaths++;
    }

    // Track highest floor reached
    if (currentRun->getCurrentFloor() > persistentStats.highestFloorReached)
    {
        persistentStats.highestFloorReached = currentRun->getCurrentFloor();
    }

    currentRun->endRun();
    if (!victory) {
        previousState = currentState;  // Save state to render underneath death overlay
    }
    currentState = victory ? STATE_RUN_COMPLETE : STATE_GAME_OVER;
}

/**
 * Apply purchased upgrades to player stats
 * Called after buying upgrades in shop
 */
void Game::applyUpgradesToPlayer()
{
    // Increase max hearts based on purchases
    player->maxHearts = STARTING_HEARTS + currentRun->getBonusHearts();
    if (player->hearts > player->maxHearts)
    {
        player->hearts = player->maxHearts;
    }

    // Increase max energy based on purchases
    player->maxEnergy = MAX_ENERGY + currentRun->getMaxEnergyBonus();
    if (player->energy > player->maxEnergy)
    {
        player->energy = player->maxEnergy;
    }
}

/**
 * Clean up all dynamic objects in current level
 * Called before loading new level or exiting
 */
void Game::cleanCurrentLevel()
{
    platforms.clear();
    sideDoors.clear();
    alcoveCeilings.clear();

    for (auto *cookie : cookies)
    {
        delete cookie;
    }
    cookies.clear();

    for (auto *enemy : enemies)
    {
        delete enemy;
    }
    enemies.clear();

    if (baker)
    {
        delete baker;
        baker = nullptr;
    }

    for (auto *proj : projectiles)
    {
        delete proj;
    }
    projectiles.clear();
}

// ============================================================================
// COORDINATE TRANSFORMATIONS
// ============================================================================

/**
 * Convert world coordinates to screen coordinates (accounting for camera)
 * Used in Downwell to render objects relative to camera position
 */
SDL_Rect Game::worldToScreen(SDL_Rect worldRect)
{
    return {
        worldRect.x,
        (int)(worldRect.y - cameraY),
        worldRect.w,
        worldRect.h};
}

/**
 * Convert a single world Y coordinate to screen Y
 */
float Game::worldToScreenY(float worldY)
{
    return worldY - cameraY;
}

// ============================================================================
// COLLISION DETECTION
// ============================================================================

/**
 * Check if player is standing on a platform
 * Used for jump input validation
 */
bool Game::isPlayerOnGround()
{
    SDL_Rect playerRect = player->getRect();

    for (auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();

        int playerBottom = playerRect.y + playerRect.h;
        int platformTop = platformRect.y;

        // Check if player is just touching platform from above
        if (playerBottom >= platformTop && playerBottom <= platformTop + 5)
        {
            if (playerRect.x + playerRect.w > platformRect.x &&
                playerRect.x < platformRect.x + platformRect.w)
            {
                return true;
            }
        }
    }

    return false;
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

/**
 * Process all input events (keyboard, window events)
 * Handles state-specific input (lobby, shop navigation, gameplay)
 */
void Game::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running = false;
        }

        // Death state - allow restart
        if (player->isDead && event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_r)
            {
                loadLobby();
            }
            continue;
        }

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            // LOBBY: Start run (Held interaction now handled in updateLobby)
            // if (currentState == STATE_LOBBY && event.key.keysym.sym == SDLK_e)
            // {
            //     if (playerNearStartDoor)
            //     {
            //         startNewRun();
            //     }
            // }

            // RUN END: Return to lobby
            if (currentState == STATE_RUN_COMPLETE || currentState == STATE_GAME_OVER)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    loadLobby();
                }
            }

            // DOWNWELL COMPLETE: Proceed to shop
            if (currentState == STATE_DOWNWELL_COMPLETE && event.key.keysym.sym == SDLK_SPACE)
            {
                currentSegment++;
                currentState = STATE_SHOP;
                selectedUpgradeIndex = 0;
                shopIsFromSideRoom = false;
            }

            // SHOP: Navigate and purchase
            if (currentState == STATE_SHOP)
            {
                const std::vector<Upgrade> &upgrades = currentRun->getAvailableUpgrades();

                if (event.key.keysym.sym == SDLK_UP)
                {
                    selectedUpgradeIndex--;
                    if (selectedUpgradeIndex < 0)
                        selectedUpgradeIndex = upgrades.size();
                }
                if (event.key.keysym.sym == SDLK_DOWN)
                {
                    selectedUpgradeIndex++;
                    if (selectedUpgradeIndex > (int)upgrades.size())
                        selectedUpgradeIndex = 0;
                }
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
                {
                    if (selectedUpgradeIndex == (int)upgrades.size())
                    {
                        // "Continue" option selected
                        if (shopIsFromSideRoom)
                        {
                            exitSideRoom();
                        }
                        else
                        {
                            currentSegment++;
                            generateDownwellSegment();
                        }
                    }
                    else if (selectedUpgradeIndex < (int)upgrades.size())
                    {
                        // Try to purchase upgrade
                        int runCookies = currentRun->getStats().cookiesThisRun;

                        if (currentRun->purchaseUpgrade(upgrades[selectedUpgradeIndex].type, runCookies))
                        {
                            currentRun->getStats().cookiesThisRun = runCookies;
                            applyUpgradesToPlayer();
                            printf("Purchased upgrade! Cookies remaining: %d\n", runCookies);
                        }
                        else
                        {
                            printf("Not enough cookies! Need %d, have %d\n",
                                   upgrades[selectedUpgradeIndex].cost, runCookies);
                        }
                    }
                }
            }

            // BOMB JACK: Auto-exit when complete (also has manual exit)
            if (currentState == STATE_BOMB_JACK && event.key.keysym.sym == SDLK_e)
            {
                if (currentBombJackLevel && bombJackCookiesCollected >= currentBombJackLevel->requiredCookies)
                {
                    currentRun->getStats().bombJackLevelsCleared++;
                    exitSideRoom();
                }
            }

            // DOWNWELL: Side door interaction
            if (currentState == STATE_DOWNWELL && event.key.keysym.sym == SDLK_e && !hasInteractedThisPress)
            {
                SDL_Rect playerWorldRect = player->getRect();

                for (auto &door : sideDoors)
                {
                    if (!door.used)
                    {
                        SDL_Rect doorWorldRect = door.rect;

                        if (SDL_HasIntersection(&playerWorldRect, &doorWorldRect))
                        {
                            enterSideRoom(door.type);
                            break;
                        }
                    }
                }
                hasInteractedThisPress = true;
            }

            // JUMP/GLIDE: Space bar
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                if (isPlayerOnGround())
                {
                    player->onGround = true;
                    player->jump();
                    currentRun->getStats().jumpsThisRun++;
                    hasJumpedThisPress = true;
                }
                else if (!hasJumpedThisPress)
                {
                    // Hold space while airborne to glide
                    player->startGliding();
                }
            }
        }

        // Key release handling
        if (event.type == SDL_KEYUP)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                hasJumpedThisPress = false;
                player->stopGliding();
            }
            if (event.key.keysym.sym == SDLK_e)
            {
                hasInteractedThisPress = false;
            }
        }
    }

    // Continuous movement (not event-based)
    if (!player->isDead && (currentState == STATE_DOWNWELL || currentState == STATE_BOMB_JACK || currentState == STATE_LOBBY))
    {
        const Uint8 *keyState = SDL_GetKeyboardState(NULL);
        if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A])
        {
            player->moveLeft();
        }
        else if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
        {
            player->moveRight();
        }
        else
        {
            player->stopMoving();
        }
    }
}

// ============================================================================
// GAME UPDATES
// ============================================================================

/**
 * Main update loop - calls state-specific update functions
 */
void Game::update()
{
    // Handle run intro transition
    if (currentState == STATE_RUN_INTRO)
    {
        transitionTimer -= 1.0f / FPS;
        if (transitionTimer <= 0)
        {
            generateDownwellSegment();
        }
        return;
    }

    // State-specific updates
    if (currentState == STATE_LOBBY)
    {
        updateLobby();
    }
    else if (currentState == STATE_DOWNWELL)
    {
        updateDownwell();
    }
    else if (currentState == STATE_BOMB_JACK)
    {
        updateBombJack();
    }

    // Reset jump flag when on ground
    if (player->onGround)
    {
        hasJumpedThisPress = false;
    }
}

/**
 * Update lobby state - player can move around, infinite glide/energy
 */
void Game::updateLobby()
{
    player->update();
    checkPlatformCollisions();

    // Infinite resources in lobby (safe space)
    player->glideTime = MAX_GLIDE_TIME;
    player->energy = MAX_ENERGY;

    // Check if player is near the recipe
    SDL_Rect playerRect = player->getRect();
    playerNearRecipe = SDL_HasIntersection(&playerRect, &recipeRect);

    // Handle stealing interaction
    if (playerNearRecipe)
    {
        const Uint8 *keyState = SDL_GetKeyboardState(NULL);
        if (keyState[SDL_SCANCODE_E])
        {
            interactionTimer += 1.0f / 60.0f; // Assuming 60 FPS update
            if (interactionTimer >= RECIPE_STEAL_TIME)
            {
                startNewRun();
            }
        }
        else
        {
            // Reset if key released
            interactionTimer = 0.0f;
        }
    }
    else
    {
        // Reset if walked away
        interactionTimer = 0.0f;
    }
}

/**
 * Update Bomb Jack level - platforming challenge with cookie collection goal
 */
void Game::updateBombJack()
{
    player->update();
    checkPlatformCollisions();
    checkCookieCollisions();
    checkEnemyCollisions();

    // Update enemies
    for (auto *enemy : enemies)
    {
        enemy->update(*player, platforms, &projectiles);
    }

    // Auto-exit when all cookies collected
    if (currentBombJackLevel && bombJackCookiesCollected >= currentBombJackLevel->requiredCookies)
    {
        printf("All cookies collected! Auto-exiting Bomb Jack level...\n");
        currentRun->getStats().bombJackLevelsCleared++;
        exitSideRoom();
        return;
    }

    // Death check - wait for death animation and fade to complete
    if (player->isDead)
    {
        if (player->deathTimer >= 2.0f)
        {
            endRun(false);
        }
    }
}

/**
 * Update Downwell segment - vertical descent with camera following
 * Core gameplay loop: fall, avoid enemies, collect cookies, reach bottom
 */
void Game::updateDownwell()
{
    float oldY = player->y;

    player->update();
    checkPlatformCollisions();
    checkAlcoveCeilingCollisions();
    checkCookieCollisions();
    checkEnemyCollisions();

    // Track maximum depth reached (for UI percentage)
    if (player->y > maxDepthReached)
    {
        maxDepthReached = player->y;
    }

    // Smooth camera following - centers on player vertically
    float targetCameraY = player->y - SCREEN_HEIGHT / 2;
    float lerpFactor = 0.2f; // Smooth interpolation
    cameraY += (targetCameraY - cameraY) * lerpFactor;

    // Clamp camera to world bounds
    if (cameraY < 0)
        cameraY = 0;
    if (cameraY > worldHeight - SCREEN_HEIGHT)
    {
        cameraY = worldHeight - SCREEN_HEIGHT;
    }

    // Update Enemies
    for (auto *enemy : enemies)
    {
        enemy->update(*player, platforms, &projectiles);
    }

    // UPDATE BAKER
    if (baker)
    {
        baker->update(*player); // He ignores platforms
        
        // Baker Collision (INSTA-KILL or heavy damage)
        if (baker->checkCollision(*player))
        {
            if (!player->isInvincible)
            {
                player->hearts = 0; // INSTA-DEATH
                player->isDead = true;
                player->deathTimer = 0.0f;  // Start death animation timer
                player->deathFadeAlpha = 0.0f;  // Start with no overlay
                printf("CAUGHT BY THE BAKER!\n");
                // Don't call endRun here - let the death timer handle it
            }
        }
    }

    for (auto *proj : projectiles)
    {
        proj->update();
    }

    // Clean up inactive projectiles logic moved into cleanProjectiles()
    checkProjectileCollisions();
    cleanProjectiles();

    // Check if player reached exit hole at bottom
    if (player->y >= worldHeight - 40)
    {
        float holeCenterX = PIT_LEFT + PIT_WIDTH / 2;
        float holeWidth = 120.0f;

        // Check if player is within the hole horizontally
        if (player->x + player->width / 2 > holeCenterX - holeWidth / 2 &&
            player->x + player->width / 2 < holeCenterX + holeWidth / 2)
        {
            printf("Player went through the exit hole!\n");
            completeDownwellSegment();
        }
    }

    // Track distance fallen (only count downward movement)
    if (player->y > oldY)
    {
        currentRun->getStats().distanceFell += (int)(player->y - oldY);
    }

    // Death check - wait for death animation and fade to complete before ending run
    if (player->isDead)
    {
        // Death animation is 0.8s, fade is 1s = 1.8s total
        // Add a small buffer, call endRun after 2 seconds
        if (player->deathTimer >= 2.0f)
        {
            endRun(false);
        }
    }
}

/**
 * Platform collision detection with directional checks
 * Handles landing on platforms, wall collisions, and state-specific boundaries
 */
void Game::checkPlatformCollisions()
{
    player->onGround = false;

    SDL_Rect playerRect = player->getRect();
    int playerLeft = playerRect.x;
    int playerRight = playerRect.x + playerRect.w;
    int playerTop = playerRect.y;
    int playerBottom = playerRect.y + playerRect.h;

    // Previous position for directional collision
    int prevPlayerLeft = playerLeft - player->velocityX;
    int prevPlayerRight = playerRight - player->velocityX;
    int prevPlayerTop = playerTop - player->velocityY;
    int prevPlayerBottom = playerBottom - player->velocityY;

    // Check collision with all platforms
    for (auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();
        int platformLeft = platformRect.x;
        int platformRight = platformRect.x + platformRect.w;
        int platformTop = platformRect.y;
        int platformBottom = platformRect.y + platformRect.h;

        if (SDL_HasIntersection(&playerRect, &platformRect))
        {
            // Landing on top of platform (can phase through from below)
            if (player->velocityY >= 0 && prevPlayerBottom <= platformTop + 3)
            {
                player->y = platformTop - player->height;
                player->velocityY = 0;
                player->onGround = true;
            }
            // Side collisions (wall bumping)
            else if (playerBottom > platformTop + 3 && playerTop < platformBottom)
            {
                if (player->velocityX > 0 && prevPlayerRight <= platformLeft + 2)
                {
                    player->x = platformLeft - player->width;
                    player->velocityX = 0;
                }
                else if (player->velocityX < 0 && prevPlayerLeft >= platformRight - 2)
                {
                    player->x = platformRight;
                    player->velocityX = 0;
                }
            }
        }
    }

    // DOWNWELL: Wall boundaries with alcove detection
    if (currentState == STATE_DOWNWELL)
    {
        // Build alcove zones for each door
        std::vector<SDL_Rect> leftAlcoveZones;
        std::vector<SDL_Rect> rightAlcoveZones;

        for (const auto &door : sideDoors)
        {
            SDL_Rect alcoveZone = {
                door.onLeftWall ? (int)(PIT_LEFT - 80) : (int)PIT_RIGHT,
                (int)(door.worldY - 90),
                80,
                130};

            if (door.onLeftWall)
            {
                leftAlcoveZones.push_back(alcoveZone);
            }
            else
            {
                rightAlcoveZones.push_back(alcoveZone);
            }
        }

        SDL_Rect playerRect = player->getRect();

        // Left wall with alcove checking
        if (player->x < PIT_LEFT)
        {
            bool inLeftAlcove = false;
            for (const auto &zone : leftAlcoveZones)
            {
                if (SDL_HasIntersection(&playerRect, &zone))
                {
                    inLeftAlcove = true;
                    break;
                }
            }

            if (inLeftAlcove)
            {
                // Inside alcove - allow deeper penetration
                if (player->x < PIT_LEFT - 80)
                {
                    player->x = PIT_LEFT - 80;
                    player->velocityX = 0;
                }
            }
            else
            {
                // default
                player->x = PIT_LEFT;
                player->velocityX = 0;
            }
        }

        if (player->x + player->width > PIT_RIGHT)
        {
            bool inRightAlcove = false;
            for (const auto &zone : rightAlcoveZones)
            {
                if (SDL_HasIntersection(&playerRect, &zone))
                {
                    inRightAlcove = true;
                    break;
                }
            }
            // alclove depth (width)
            if (inRightAlcove)
            {
                if (player->x + player->width > PIT_RIGHT + 80)
                {
                    player->x = PIT_RIGHT + 80 - player->width;
                    player->velocityX = 0;
                }
            }
            else
            {
                // default
                player->x = PIT_RIGHT - player->width;
                player->velocityX = 0;
            }
        }
    }

    // bomb jack level box
    if (currentState == STATE_BOMB_JACK)
    {
        if (player->x < ARENA_LEFT + 15)
        {
            player->x = ARENA_LEFT + 15;
            player->velocityX = 0;
        }
        if (player->x + player->width > ARENA_LEFT + ARENA_WIDTH - 15)
        {
            player->x = ARENA_LEFT + ARENA_WIDTH - 15 - player->width;
            player->velocityX = 0;
        }
        if (player->y < ARENA_TOP + 15)
        {
            player->y = ARENA_TOP + 15;
            player->velocityY = 0;
        }
        if (player->y + player->height > ARENA_TOP + ARENA_HEIGHT - 15)
        {
            player->y = ARENA_TOP + ARENA_HEIGHT - 15 - player->height;
            player->velocityY = 0;
            player->onGround = true;
        }
    }

    // lobby screen
    if (currentState == STATE_LOBBY)
    {
        if (player->x < 0)
        {
            player->x = 0;
            player->velocityX = 0;
        }
        if (player->x + player->width > SCREEN_WIDTH)
        {
            player->x = SCREEN_WIDTH - player->width;
            player->velocityX = 0;
        }
        if (player->y + player->height >= SCREEN_HEIGHT - 1)
        {
            player->y = SCREEN_HEIGHT - player->height;
            player->velocityY = 0;
            player->onGround = true;
        }
    }
}

void Game::checkAlcoveCeilingCollisions()
{
    SDL_Rect playerRect = player->getRect();

    for (const auto &ceiling : alcoveCeilings)
    {
        SDL_Rect ceilingRect = ceiling.rect;

        // ceiling for aclove
        if (player->velocityY < 0 && SDL_HasIntersection(&playerRect, &ceilingRect))
        {
            player->y = ceilingRect.y + ceilingRect.h;
            player->velocityY = 0;
            break;
        }
    }
}

void Game::checkCookieCollisions()
{
    for (auto *cookie : cookies)
    {
        if (!cookie->collected && cookie->checkCollision(*player))
        {
            cookie->collected = true;
            persistentStats.totalCookies++;
            currentRun->getStats().cookiesThisRun++;

            if (currentState == STATE_BOMB_JACK)
            {
                bombJackCookiesCollected++;
            }

            player->restoreEnergy(COOKIE_ENERGY_RESTORE);
            printf("Cookie! Total: %d, This run: %d\n",
                   persistentStats.totalCookies, currentRun->getStats().cookiesThisRun);
        }
    }
}

void Game::checkEnemyCollisions()
{
    auto it = enemies.begin();
    while (it != enemies.end())
    {
        Enemy *enemy = *it;

        if (enemy->checkCollision(*player))
        {
            SDL_Rect playerRect = player->getRect();
            SDL_Rect enemyRect = enemy->getRect();

            // Calculate if player is coming from above
            // Player's bottom vs enemy's top
            int playerBottom = playerRect.y + playerRect.h;
            int enemyTop = enemyRect.y;
            int enemyMidpoint = enemyRect.y + enemyRect.h / 2;

            // STOMP CHECK: Player is above enemy midpoint AND falling
            bool isAboveEnemy = playerBottom < enemyMidpoint;
            bool isFalling = player->velocityY > 0;

            if (isAboveEnemy && isFalling)
            {
                // printf("💥 STOMPED %s! +5 cookies, energy restored!\n",
                //        enemy->type == ENEMY_PATROL ? "FRYING PAN" :
                //        enemy->type == ENEMY_JUMPER ? "ROLLING PIN" : "WOODEN SPOON");

                // Bounce player up (stronger bounce if falling faster)
                float bounceStrength = -8.0f - (player->velocityY * 0.3f);
                if (bounceStrength < -15.0f)
                    bounceStrength = -15.0f; // max
                player->velocityY = bounceStrength;
                player->onGround = false;

                currentRun->getStats().cookiesThisRun += 5;
                persistentStats.totalCookies += 5;
                player->restoreEnergy(30.0f);

                // Delete enemy
                delete enemy;
                it = enemies.erase(it);
                continue;
            }
            else
            {
                // Side or bottom collision = take damage
                if (!player->isInvincible)
                {
                    player->loseHeart();
                }
            }
        }
        ++it;
    }
}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_RenderClear(renderer);

    if (currentState == STATE_LOBBY)
    {
        renderLobby();
    }
    else if (currentState == STATE_RUN_INTRO)
    {
        renderRunIntro();
    }
    else if (currentState == STATE_DOWNWELL)
    {
        renderDownwell();
    }
    else if (currentState == STATE_BOMB_JACK)
    {
        renderBombJack();
    }
    else if (currentState == STATE_SHOP)
    {
        renderShop();
    }
    else if (currentState == STATE_DOWNWELL_COMPLETE)
    {
        renderDownwellComplete();
    }
    else if (currentState == STATE_RUN_COMPLETE)
    {
        renderRunComplete();
    }
    else if (currentState == STATE_GAME_OVER)
    {
        // Render the game world underneath (from previousState)
        if (previousState == STATE_DOWNWELL)
        {
            renderDownwell();
        }
        else if (previousState == STATE_BOMB_JACK)
        {
            renderBombJack();
        }
        
        // Then render the death overlay on top
        renderGameOver();
    }

    SDL_RenderPresent(renderer);
}

void Game::renderLobby()
{
    // Platforms
    for (const auto &platform : platforms)
    {
        platform.render(renderer);
    }

    // Recipe (The objective)
    // SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gold
    // SDL_RenderFillRect(renderer, &recipeRect);
    
    // // Shine effect
    // SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
    // SDL_Rect shine = {recipeRect.x + 5, recipeRect.y + 5, 5, 5};
    // SDL_RenderFillRect(renderer, &shine);

    textureManager->renderTexture("recipe", recipeRect.x, recipeRect.y, recipeRect.w, recipeRect.h);

    if (playerNearRecipe)
    {
        SDL_Color white = {255, 255, 255, 255};
        
        if (interactionTimer > 0)
        {
            // Progress Bar
            int barWidth = 60;
            int barHeight = 10;
            int barX = recipeRect.x + recipeRect.w / 2 - barWidth / 2;
            int barY = recipeRect.y - 20;

            // Background
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_Rect bg = {barX, barY, barWidth, barHeight};
            SDL_RenderFillRect(renderer, &bg);

            // Fill
            float progress = interactionTimer / RECIPE_STEAL_TIME;
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_Rect fill = {barX + 1, barY + 1, (int)((barWidth - 2) * progress), barHeight - 2};
            SDL_RenderFillRect(renderer, &fill);
            
            textManager->renderText(renderer, "STEALING...", "small",
                                recipeRect.x + recipeRect.w / 2, recipeRect.y - 40, white, true);
        }
        else
        {
            textManager->renderText(renderer, "Hold E to Steal Recipe", "small",
                                recipeRect.x + recipeRect.w / 2, recipeRect.y - 30, white, true);
        }
    }

    // Render Player using Sprite Sheet (32x32 source, scaled to 96x96)
    SDL_Rect playerRect = player->getRect(); // This is the hitbox (27x39)
    
    // Calculate sprite position:
    // The hitbox is offset within the sprite by the defined offsets (scaled)
    // Sprite top-left = hitbox position - (left_offset * scale, top_offset * scale)
    int spriteX = playerRect.x - (HITBOX_LEFT_OFFSET * PLAYER_RENDER_SCALE);
    int spriteY = playerRect.y - (HITBOX_TOP_OFFSET * PLAYER_RENDER_SCALE);

    SDL_Rect srcRect = player->getSpriteSrcRect();
    SDL_RendererFlip flip = player->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    textureManager->renderFrame("player", spriteX, spriteY, 
                               srcRect.x, srcRect.y, PLAYER_SPRITE_SIZE, PLAYER_SPRITE_SIZE, 
                               PLAYER_RENDER_SIZE, PLAYER_RENDER_SIZE, flip);

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

void Game::renderRunIntro()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color red = {255, 100, 100, 255};
    textManager->renderText(renderer, "STEALING THE RECIPE...", "title",
                            SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, red, true);

    SDL_Color white = {255, 255, 255, 255};
    textManager->renderText(renderer, "ALARM TRIGGERED!", "normal",
                            SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 30, white, true);
}

void Game::checkProjectileCollisions()
{
    for (auto *proj : projectiles)
    {
        if (proj->checkCollision(*player))
        {
            if (!player->isInvincible)
            {
                player->loseHeart();
                proj->active = false;
            }
        }
    }
}

void Game::cleanProjectiles()
{
    auto it = projectiles.begin();
    while (it != projectiles.end())
    {
        if (!(*it)->active)
        {
            delete *it;
            it = projectiles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Game::renderDownwellWalls()
{
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

void Game::renderDownwell()
{
    renderDownwellWalls();

    for (const auto &door : sideDoors)
    {
        SDL_Rect screenDoor = worldToScreen(door.rect);

        if (screenDoor.y + screenDoor.h >= 0 && screenDoor.y <= SCREEN_HEIGHT)
        {
            if (door.used)
            {
                // bomb jack door
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
                SDL_RenderFillRect(renderer, &screenDoor);

                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
                SDL_RenderDrawLine(renderer, screenDoor.x, screenDoor.y,
                                   screenDoor.x + screenDoor.w, screenDoor.y + screenDoor.h);
                SDL_RenderDrawLine(renderer, screenDoor.x + screenDoor.w, screenDoor.y,
                                   screenDoor.x, screenDoor.y + screenDoor.h);
            }
            else
            {
                // active door
                if (door.type == ROOM_SHOP)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // ywllow: shops
                }
                else
                {
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
                    6, 6};
                SDL_RenderFillRect(renderer, &knob);

                // promp in proximity
                SDL_Rect playerScreenRect = {
                    (int)player->x,
                    (int)(player->y - cameraY),
                    (int)player->width,
                    (int)player->height};

                if (SDL_HasIntersection(&playerScreenRect, &screenDoor))
                {
                    SDL_Color white = {255, 255, 255, 255};
                    const char *label = door.type == ROOM_SHOP ? "SHOP [E]" : "LEVEL [E]";
                    textManager->renderText(renderer, label, "small",
                                            screenDoor.x + screenDoor.w / 2,
                                            screenDoor.y - 15, white, true);
                }
            }
        }
    }
    // NOW RENDER

    // platforms
    for (const auto &platform : platforms)
    {
        SDL_Rect screenRect = worldToScreen(platform.getRect());

        if (screenRect.y + screenRect.h >= 0 && screenRect.y <= SCREEN_HEIGHT)
        {
            SDL_SetRenderDrawColor(renderer, platform.color.r, platform.color.g, platform.color.b, platform.color.a);
            SDL_RenderFillRect(renderer, &screenRect);
        }
    }

    // Draw Cookies
    for (auto *cookie : cookies)
    {
        if (!cookie->collected)
        {
            SDL_Rect screenRect = worldToScreen(cookie->getRect());
            if (screenRect.y + screenRect.h > 0 && screenRect.y < SCREEN_HEIGHT)
            {
                textureManager->renderTexture("cookie", screenRect.x, screenRect.y, screenRect.w, screenRect.h);
            }
        }
    }

    // Draw Enemies
    for (auto *enemy : enemies)
    {
        SDL_Rect screenRect = worldToScreen(enemy->getRect());
        if (screenRect.y + screenRect.h > 0 && screenRect.y < SCREEN_HEIGHT)
        {
            SDL_RendererFlip flip = enemy->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            textureManager->renderTexture("enemy", screenRect.x, screenRect.y, screenRect.w, screenRect.h, flip);
        }
    }

    // Render THE BAKER
    if (baker)
    {
        SDL_Rect screenRect = worldToScreen(baker->getRect());
        // Always render him even if offscreen? No, waste of time.
        if (screenRect.y + screenRect.h > -100 && screenRect.y < SCREEN_HEIGHT + 100)
        {
            // Use "enemy" sprite but maybe modify color or use a different one if we had it.
            // For now, let's just make him big.
             SDL_RendererFlip flip = baker->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
             
             // Tint him red to show he is dangerous
             SDL_SetTextureColorMod(textureManager->getTexture("enemy"), 255, 100, 100);
             textureManager->renderTexture("enemy", screenRect.x, screenRect.y, screenRect.w, screenRect.h, flip);
             SDL_SetTextureColorMod(textureManager->getTexture("enemy"), 255, 255, 255); // Reset
        }
    }
    
    // Draw Projectiles
    // Default red projectiles for now (can map to a small sprite later)
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    for (auto *proj : projectiles)
    {
        if (proj->active)
        {
            SDL_Rect screenRect = worldToScreen(proj->getRect());
            SDL_RenderFillRect(renderer, &screenRect);
        }
    }

    // Draw Player
    // SDL_Rect worldPlayerRect = player->getRect();
    // SDL_Rect screenPlayerRect = worldToScreen(worldPlayerRect);
    
    // Sprite Rendering with 3x scaled sprites (32x32 source -> 96x96 rendered)
    // Hitbox is (27x39). Sprite is (96x96).
    // Hitbox is positioned within sprite based on offset constants
    SDL_Rect playerRect = player->getRect(); // Hitbox in world space
    SDL_Rect screenPlayerRect = worldToScreen(playerRect); // Hitbox in screen space
    
    // Calculate sprite position in screen space
    // Sprite top-left = hitbox position - (left_offset * scale, top_offset * scale)
    int spriteX = screenPlayerRect.x - (HITBOX_LEFT_OFFSET * PLAYER_RENDER_SCALE);
    int spriteY = screenPlayerRect.y - (HITBOX_TOP_OFFSET * PLAYER_RENDER_SCALE);
    
    SDL_Rect srcRect = player->getSpriteSrcRect();
    SDL_RendererFlip flip = player->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    // Flash if invincible (color modulation)
    if (player->isInvincible && !player->isDead) {
         int blink = (int)(player->invincibilityTimer * 15) % 2;
         if (blink) SDL_SetTextureColorMod(textureManager->getTexture("player"), 200, 200, 255); // Lighter tint
         else SDL_SetTextureColorMod(textureManager->getTexture("player"), 255, 255, 255);
    } else {
         SDL_SetTextureColorMod(textureManager->getTexture("player"), 255, 255, 255);
    }

    textureManager->renderFrame("player", spriteX, spriteY, 
                               srcRect.x, srcRect.y, PLAYER_SPRITE_SIZE, PLAYER_SPRITE_SIZE, // Source rect from sprite sheet
                               PLAYER_RENDER_SIZE, PLAYER_RENDER_SIZE, // Destination size (96x96)
                               flip);
                               
    // Reset Color Mod
    SDL_SetTextureColorMod(textureManager->getTexture("player"), 255, 255, 255);

    // player->render(renderer, false); // Disabled old render
    // side doors
    for (const auto &door : sideDoors)
    {
        SDL_Rect screenDoor = worldToScreen(door.rect);

        if (screenDoor.y + screenDoor.h >= 0 && screenDoor.y <= SCREEN_HEIGHT)
        {
            if (door.used)
            {
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            }
            else
            {
                if (door.type == ROOM_SHOP)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // ywllow: shops
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // brown: bomb Jack
                }
                SDL_RenderFillRect(renderer, &screenDoor);

                // Door
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &screenDoor);

                if (!door.used)
                {
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                    SDL_Rect knob = {
                        door.onLeftWall ? screenDoor.x + screenDoor.w - 10 : screenDoor.x + 5,
                        screenDoor.y + screenDoor.h / 2 - 3,
                        6, 6};
                    SDL_RenderFillRect(renderer, &knob);
                }

                // promp in proximity
                if (!door.used)
                {
                    SDL_Rect playerScreenRect = {
                        (int)player->x,
                        (int)(player->y - cameraY),
                        (int)player->width,
                        (int)player->height};

                    if (SDL_HasIntersection(&playerScreenRect, &screenDoor))
                    {
                        SDL_Color white = {255, 255, 255, 255};
                        const char *label = door.type == ROOM_SHOP ? "SHOP [E]" : "LEVEL [E]";
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
            (int)player->height};

        if (!player->isDead)
        {
            if (player->isInvincible)
            {
                int blinkInterval = (int)(player->invincibilityTimer * 10) % 2;
                if (blinkInterval == 0)
                {
                    renderUI();
                    renderSideStats();
                    return;
                }
            }

            /*
            // Manual Player Rendering (Removed for Sprite)
            if (player->isGliding)
            {
                SDL_SetRenderDrawColor(renderer, 191, 64, 191, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 112, 41, 99, 255);
            }
            SDL_RenderFillRect(renderer, &playerScreenRect);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect hair = {playerScreenRect.x, playerScreenRect.y, playerScreenRect.w, playerScreenRect.h / 3};
            SDL_RenderFillRect(renderer, &hair);

            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            SDL_Rect face = {playerScreenRect.x + 5, playerScreenRect.y + 5, 22, 15};
            SDL_RenderFillRect(renderer, &face);
            */

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
            if (energyPercent > 0.5f)
            {
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            }
            else if (energyPercent > 0.25f)
            {
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            }
            else if (energyPercent > 0)
            {
                SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            }
            SDL_Rect energyMeter = {barX, energyBarY, (int)(barWidth * energyPercent), barHeight};
            SDL_RenderFillRect(renderer, &energyMeter);
        }

        renderUI();
        renderSideStats();
    }
}

void Game::renderSideStats()
{
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
    if (worldHeight > 0)
    {
        depthPercent = (maxDepthReached / worldHeight) * 100.0f; // Use maxDepthReached
        if (depthPercent > 100)
            depthPercent = 100;
    }

    textManager->renderText(renderer, "DEPTH", "small", leftX, y, gold);
    y += 25;
    sprintf(text, "%.0f%%", depthPercent);
    textManager->renderText(renderer, text, "normal", leftX, y, white);
}

void Game::renderBombJack()
{
    for (const auto &platform : platforms)
    {
        platform.render(renderer);
    }

    for (auto *cookie : cookies)
    {
        if (!cookie->collected)
        {
            textureManager->renderTexture("cookie", cookie->x, cookie->y, 
                                        cookie->getRect().w, cookie->getRect().h);
        }
    }

    for (auto *enemy : enemies)
    {
        SDL_RendererFlip flip = enemy->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        
        // Enemy sprites are also 32x32 tiles
        textureManager->renderFrame("enemy", enemy->x, enemy->y + enemy->height - 32, 
                                   enemy->currentFrame * 32, 0, 32, 32, 
                                   32, 32, flip);
    }

    for (auto *proj : projectiles)
    {
        if (proj->active)
        {
            SDL_Rect tick = proj->getRect();
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_RenderFillRect(renderer, &tick);
        }
    }

    // Render Player with sprite sheet (32x32 source -> 96x96 rendered)
    SDL_Rect pRect = player->getRect(); // Hitbox (27x39)
    SDL_Rect srcRect = player->getSpriteSrcRect();
    SDL_RendererFlip flip = player->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    // Calculate sprite position
    // Sprite top-left = hitbox position - (left_offset * scale, top_offset * scale)
    int spriteX = pRect.x - (HITBOX_LEFT_OFFSET * PLAYER_RENDER_SCALE);
    int spriteY = pRect.y - (HITBOX_TOP_OFFSET * PLAYER_RENDER_SCALE);
    
    textureManager->renderFrame("player", spriteX, spriteY,
                               srcRect.x, srcRect.y, PLAYER_SPRITE_SIZE, PLAYER_SPRITE_SIZE,
                               PLAYER_RENDER_SIZE, PLAYER_RENDER_SIZE, flip);
    
    // player->render(renderer, true);

    // cookie progress in bomb jack level
    if (currentBombJackLevel)
    {
        SDL_Color white = {255, 255, 255, 255};
        char text[32];
        sprintf(text, "Cookies: %d/%d", bombJackCookiesCollected, currentBombJackLevel->requiredCookies);
        textManager->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, 30, white, true);
    }

    renderHearts();
}

void Game::renderShop()
{
    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255);
    SDL_RenderClear(renderer);

    SDL_Color gold = {255, 215, 0, 255};
    textManager->renderText(renderer, "UPGRADE SHOP", "title", SCREEN_WIDTH / 2, 50, gold, true);

    SDL_Color white = {255, 255, 255, 255};
    char cookieText[32];
    // USE RUN COOKIES, NOT PERSISTENT COOKIES //////////////////////////////////////////////////////
    sprintf(cookieText, "Cookies: %d", currentRun->getStats().cookiesThisRun);
    textManager->renderText(renderer, cookieText, "normal", SCREEN_WIDTH / 2, 100, white, true);

    const std::vector<Upgrade> &upgrades = currentRun->getAvailableUpgrades();

    int startY = 160;
    int spacing = 60;

    for (int i = 0; i < (int)upgrades.size(); i++)
    {
        const Upgrade &upgrade = upgrades[i];

        bool selected = (i == selectedUpgradeIndex);
        SDL_Color color = selected ? (SDL_Color){255, 255, 100, 255} : (SDL_Color){200, 200, 200, 255};

        if (upgrade.purchased)
        {
            color = {100, 255, 100, 255};
        }

        char line[128];
        if (upgrade.purchased)
        {
            sprintf(line, "[OWNED] %s", upgrade.name.c_str());
        }
        else
        {
            sprintf(line, "%s - %d cookies", upgrade.name.c_str(), upgrade.cost);
        }

        textManager->renderText(renderer, line, "small", 100, startY + i * spacing, color);

        if (selected && !upgrade.purchased)
        {
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

void Game::renderDownwellComplete()
{
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

void Game::renderRunComplete()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color gold = {255, 215, 0, 255};
    textManager->renderText(renderer, "RUN COMPLETE!", "title", SCREEN_WIDTH / 2, 80, gold, true);

    SDL_Color white = {255, 255, 255, 255};
    RunStats &stats = currentRun->getStats();

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

void Game::renderGameOver()
{
    // Draw semi-transparent black overlay (fades in based on player's death timer)
    Uint8 overlayAlpha = (Uint8)player->deathFadeAlpha;
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, overlayAlpha);
    SDL_Rect fullScreen = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &fullScreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // Only show text when overlay is mostly visible
    if (overlayAlpha < 100) return;
    
    // Calculate text alpha based on overlay alpha (fade in with overlay)
    Uint8 textAlpha = overlayAlpha > 200 ? 255 : (Uint8)((overlayAlpha / 200.0f) * 255);

    SDL_Color red = {255, 50, 50, textAlpha};
    textManager->renderText(renderer, "CAUGHT!", "title", SCREEN_WIDTH / 2, 80, red, true);

    SDL_Color white = {255, 255, 255, textAlpha};
    RunStats &stats = currentRun->getStats();

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

    SDL_Color hint = {200, 200, 200, textAlpha};
    textManager->renderText(renderer, "Press R to return to lobby", "small",
                            SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60, hint, true);
}

void Game::renderHearts()
{
    int heartSize = 20;
    int heartSpacing = 25;
    int startX = SCREEN_WIDTH - 10 - (player->maxHearts * heartSpacing);
    int startY = 10;

    for (int i = 0; i < player->maxHearts; i++)
    {
        int heartX = startX + (i * heartSpacing);

        if (i < player->hearts)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        }

        SDL_Rect heart = {heartX, startY, heartSize, heartSize};
        SDL_RenderFillRect(renderer, &heart);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &heart);
    }
}

void Game::renderUI()
{
    renderHearts();

    SDL_Color white = {255, 255, 255, 255};
    char text[64];

    sprintf(text, "Floor %d", currentRun->getCurrentFloor());
    textManager->renderText(renderer, text, "small", 10, 10, white);

    sprintf(text, "Cookies: %d", currentRun->getStats().cookiesThisRun);
    textManager->renderText(renderer, text, "small", 10, 30, white);

    float depthPercent = 0;
    if (worldHeight > 0)
    {
        depthPercent = (player->y / worldHeight) * 100.0f;
        if (depthPercent > 100)
            depthPercent = 100;
    }
    sprintf(text, "Depth: %.0f%%", depthPercent);
    textManager->renderText(renderer, text, "small", 10, 50, white);
}

void Game::run()
{
    Uint32 frameStart;
    int frameTime;

    while (running)
    {
        frameStart = SDL_GetTicks();

        handleEvents();
        update();
        render();

        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
}

void Game::clean()
{
    cleanCurrentLevel();

    for (auto *cookie : savedCookies)
    {
        delete cookie;
    }
    savedCookies.clear();

    for (auto *enemy : savedEnemies)
    {
        delete enemy;
    }
    savedEnemies.clear();

    if (player)
    {
        delete player;
        player = nullptr;
    }

    if (textManager)
    {
        delete textManager;
        textManager = nullptr;
    }

    if (levelManager)
    {
        delete levelManager;
        levelManager = nullptr;
    }

    if (downwellGenerator)
    {
        delete downwellGenerator;
        downwellGenerator = nullptr;
    }

    if (currentRun)
    {
        delete currentRun;
        currentRun = nullptr;
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}