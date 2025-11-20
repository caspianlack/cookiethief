#include "enemy.h"
#include "projectile.h"
#include "platform.h"
#include "constants.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

Enemy::Enemy(float startX, float startY, EnemyType enemyType, int difficulty)
{
    x = startX;
    y = startY;
    type = enemyType;
    width = ENEMY_WIDTH;
    height = ENEMY_HEIGHT;
    velocityY = 0;
    onGround = false;
    facingLeft = false;
    hasFoundEdges = false;
    alertLevel = 0.0f;
    isActive = false;

    float enemyVariation = 0.8f + (rand() % 40) / 100.0f; // 0.8 - 1.2

    switch (type)
    {
    case ENEMY_PATROL:
        speed = PATROL_SPEED * (1.0f + difficulty * ENEMY_SPEED_SCALE_PER_FLOOR) * enemyVariation;
        patrolLeft = x - 50;
        patrolRight = x + 50;
        patrolDirection = 1;
        pauseTimer = 0;
        isPaused = false;
        break;

    case ENEMY_JUMPER:
        speed = JUMPER_SPEED * (1.0f + difficulty * ENEMY_SPEED_SCALE_PER_FLOOR) * enemyVariation;
        jumpCooldown = 0;
        chaseCooldown = 0;
        patrolDirection = 1;
        isSleeping = true;
        wakeUpRange = 150.0f - (difficulty * 15.0f);
        if (wakeUpRange < 80.0f)
            wakeUpRange = 80.0f;

        chaseTimer = 0;
        maxChaseTime = 3.0f + (rand() % 200) / 100.0f;
        hasLostPlayer = false;
        lostPlayerTimer = 0;
        retreatTimer = 0;
        isRetreating = false;

        attackCooldown = 0;
        isAttacking = false;
        attackRange = 45.0f;
        break;

    case ENEMY_SHOOTER:
        speed = SHOOTER_SPEED * (1.0f + difficulty * ENEMY_SPEED_SCALE_PER_FLOOR) * enemyVariation;
        patrolLeft = x - 50;
        patrolRight = x + 50;
        patrolDirection = 1;
        shootCooldown = SHOOTER_INITIAL_COOLDOWN;
        shootInterval = SHOOTER_SHOOT_INTERVAL - (difficulty * SHOOTER_COOLDOWN_REDUCE_PER_FLOOR);
        if (shootInterval < SHOOTER_MIN_SHOOT_INTERVAL)
        {
            shootInterval = SHOOTER_MIN_SHOOT_INTERVAL;
        }
        aimTime = 0;
        isAiming = false;
        hasFoundEdges = false;
        printf("ENEMY_SHOOTER created at (%.1f, %.1f) with shoot interval %.1fs\n",
               x, y, shootInterval);
        break;
    }

    float aggressionRoll = (rand() % 100) / 100.0f;
    if (aggressionRoll > 0.7f)
    {
        alertLevel = 0.3f;
    }
}

bool Enemy::shouldActivate(const Player &player) const
{
    float dx = player.x - x;
    float dy = player.y - y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < ENEMY_ACTIVATION_RANGE;
}

bool Enemy::shouldDeactivate(const Player &player) const
{
    float dx = player.x - x;
    float dy = player.y - y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance > ENEMY_DEACTIVATION_RANGE;
}

void Enemy::update(Player &player, const std::vector<Platform> &platforms, std::vector<Projectile *> *projectiles)
{
    if (!isActive && shouldActivate(player))
    {
        isActive = true;
    }
    else if (isActive && shouldDeactivate(player))
    {
        isActive = false;
        return;
    }

    if (!isActive)
        return;

    float dx = player.x - x;
    float dy = player.y - y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < ENEMY_ALERT_RANGE)
    {
        alertLevel += ENEMY_ALERT_INCREASE_RATE / FPS;
        if (alertLevel > 1.0f)
            alertLevel = 1.0f;
    }
    else
    {
        alertLevel -= ENEMY_ALERT_DECREASE_RATE / FPS;
        if (alertLevel < 0)
            alertLevel = 0;
    }

    switch (type)
    {
    case ENEMY_PATROL:
        updatePatrol(player, platforms);
        applyGravity(platforms);
        break;

    case ENEMY_JUMPER:
        updateJumper(player, platforms);
        break;

    case ENEMY_SHOOTER:
        if (projectiles)
        {
            updateShooter(player, *projectiles, platforms);
        }
        applyGravity(platforms);
        break;
    }
}

void Enemy::findPlatformEdges(const std::vector<Platform> &platforms)
{
    SDL_Rect enemyRect = getRect();
    int enemyBottom = enemyRect.y + enemyRect.h;

    for (const auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();
        int platformTop = platformRect.y;

        if (abs(enemyBottom - platformTop) < 5 &&
            enemyRect.x + enemyRect.w > platformRect.x &&
            enemyRect.x < platformRect.x + platformRect.w)
        {

            patrolLeft = platformRect.x + ENEMY_PLATFORM_MARGIN;
            patrolRight = platformRect.x + platformRect.w - width - ENEMY_PLATFORM_MARGIN;
            hasFoundEdges = true;

            if (x < patrolLeft)
            {
                x = patrolLeft;
                patrolDirection = 1;
            }
            if (x > patrolRight)
            {
                x = patrolRight;
                patrolDirection = -1;
            }

            return;
        }
    }
}

bool Enemy::isOnPlatformEdge(const std::vector<Platform> &platforms, bool checkLeft)
{
    SDL_Rect enemyRect = getRect();

    SDL_Rect checkRect;
    if (checkLeft)
    {
        checkRect = {enemyRect.x - (int)ENEMY_EDGE_CHECK_DISTANCE,
                     enemyRect.y + enemyRect.h, 5, 10};
    }
    else
    {
        checkRect = {enemyRect.x + enemyRect.w + (int)ENEMY_EDGE_CHECK_DISTANCE - 5,
                     enemyRect.y + enemyRect.h, 5, 10};
    }

    for (const auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();
        if (SDL_HasIntersection(&checkRect, &platformRect))
        {
            return false;
        }
    }

    return true;
}

void Enemy::updatePatrol(Player &player, const std::vector<Platform> &platforms)
{
    if (!hasFoundEdges)
    {
        findPlatformEdges(platforms);
    }

    if (isPaused)
    {
        pauseTimer -= 1.0f / FPS;
        if (pauseTimer <= 0)
        {
            isPaused = false;
        }
        return;
    }

    int pauseChance = PATROL_PAUSE_CHANCE * (1.0f + alertLevel);
    if (rand() % (int)pauseChance == 0)
    {
        if (alertLevel > 0.7f)
        {
            isPaused = false;
        }
        else
        {
            isPaused = true;
        }
        float pauseDuration = PATROL_PAUSE_MIN +
                              (rand() % (int)((PATROL_PAUSE_MAX - PATROL_PAUSE_MIN) * 100)) / 100.0f;
        pauseTimer = pauseDuration;
        return;
    }

    // Move faster when alert
    float currentSpeed = speed * (1.0f + alertLevel * 0.5f);
    x += currentSpeed * patrolDirection;

    // Check for platform edges
    if (patrolDirection == -1 && isOnPlatformEdge(platforms, true))
    {
        x -= currentSpeed * patrolDirection;
        patrolDirection = 1;
    }
    else if (patrolDirection == 1 && isOnPlatformEdge(platforms, false))
    {
        x -= currentSpeed * patrolDirection;
        patrolDirection = -1;
    }

    if (x <= patrolLeft)
    {
        x = patrolLeft;
        patrolDirection = 1;
    }
    else if (x >= patrolRight)
    {
        x = patrolRight;
        patrolDirection = -1;
    }

    facingLeft = (patrolDirection == -1);

    float dx = player.x - x;
    float dy = player.y - y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < attackRange && attackCooldown <= 0)
    {
        isAttacking = true;
        attackCooldown = 1.0f;

        if (distance < 40.0f && !player.isInvincible)
        {
            player.loseHeart();
        }
    }

    if (attackCooldown > 0)
    {
        attackCooldown -= 1.0f / FPS;
    }
    if (isAttacking && attackCooldown <= 0.5f)
    {
        isAttacking = false;
    }
}

bool Enemy::hasLandingPlatformAhead(const std::vector<Platform> &platforms, bool checkRight)
{
    float checkDistance = 150.0f;
    float checkX = x + (checkRight ? checkDistance : -checkDistance);
    float checkY = y + 100.0f;

    for (const auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();

        // platform ahead horizontally
        bool isAhead = checkRight ? (platformRect.x > x) : (platformRect.x + platformRect.w < x);

        // within reasonable range
        bool inRange = fabs((platformRect.x + platformRect.w / 2) - checkX) < 100.0f;

        // not too far below
        bool goodHeight = platformRect.y > y && platformRect.y < checkY;

        if (isAhead && inRange && goodHeight)
        {
            return true;
        }
    }

    return false;
}

Platform *Enemy::findPlatformBetween(const std::vector<Platform> &platforms, float startY, float targetY, bool preferRight)
{
    Platform *bestPlatform = nullptr;
    float bestScore = -1.0f;

    for (const auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();
        float platformY = platformRect.y;
        float platformCenterX = platformRect.x + platformRect.w / 2;

        // Is this platform between us and the target vertically?
        bool between = (startY > platformY && platformY > targetY);
        if (!between)
            continue;

        // Calculate score based on how well positioned it is
        float horizontalDistance = fabs(platformCenterX - x);
        float verticalDistance = fabs(platformY - startY);

        // Prefer platforms that are:
        // 1. Not too far horizontally (within jump range)
        // 2. Not too close vertically (need to actually jump)
        // 3. In the direction we want to go
        if (horizontalDistance > 200.0f)
            continue; // Too far
        if (verticalDistance < 30.0f)
            continue; // Too close

        bool correctDirection = preferRight ? (platformCenterX > x) : (platformCenterX < x);

        float score = 100.0f - (horizontalDistance * 0.5f) - (verticalDistance * 0.3f);
        if (correctDirection)
            score += 50.0f; // Bonus for correct direction

        if (score > bestScore)
        {
            bestScore = score;
            bestPlatform = const_cast<Platform *>(&platform);
        }
    }

    return bestPlatform;
}

bool Enemy::isBlockedHorizontally(const std::vector<Platform> &platforms, bool checkRight)
{
    SDL_Rect checkRect = getRect();

    if (checkRight)
    {
        checkRect.x += 5;
        checkRect.w = 20;
    }
    else
    {
        checkRect.x -= 20;
        checkRect.w = 20;
    }

    checkRect.y += 10;
    checkRect.h -= 20;

    for (const auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();
        if (SDL_HasIntersection(&checkRect, &platformRect))
        {
            return true;
        }
    }

    return false;
}

void Enemy::updateJumper(Player &player, const std::vector<Platform> &platforms)
{
    float dx = player.x - x;
    float dy = player.y - y;
    float distance = sqrt(dx * dx + dy * dy);

    if (isSleeping && distance < wakeUpRange)
    {
        isSleeping = false;
        chaseTimer = 0;
        hasLostPlayer = false;
        isRetreating = false;
        retreatTimer = 0;
        printf("Baker woke up! Chase begins!\n");
    }

    if (isSleeping)
    {
        applyGravity(platforms);
        return;
    }

    // RETREAT BEHAVIOR - Need to improve either respawn player on taking damage or hit over because gravity is applied to both enemies and players
    if (isRetreating)
    {
        retreatTimer -= 1.0f / FPS;

        if (retreatTimer <= 0)
        {
            printf("Jumper finished retreating, going back to sleep\n");
            isSleeping = true;
            isRetreating = false;
            chaseTimer = 0;
            hasLostPlayer = false;
            applyGravity(platforms);
            return;
        }

        if (onGround && fabs(dx) > 10)
        {
            float retreatSpeed = speed * 0.7f;
            x += (dx > 0 ? -retreatSpeed : retreatSpeed);
            facingLeft = (dx > 0);
        }

        if (onGround && jumpCooldown <= 0)
        {
            bool blocked = isBlockedHorizontally(platforms, dx < 0);
            bool atEdge = (dx > 0) ? isOnPlatformEdge(platforms, true)
                                   : isOnPlatformEdge(platforms, false);

            if (blocked || atEdge)
            {
                velocityY = JUMPER_JUMP_FORCE;
                onGround = false;
                jumpCooldown = JUMPER_JUMP_COOLDOWN;
            }
        }

        applyGravity(platforms);
        return;
    }

    if (distance > JUMPER_CHASE_RANGE * 1.2f)
    { // 20% beyond chase range
        if (!hasLostPlayer)
        {
            hasLostPlayer = true;
            lostPlayerTimer = 0;
            printf("Jumper lost sight of player...\n");
        }

        lostPlayerTimer += 1.0f / FPS;

        if (lostPlayerTimer > 2.0f)
        {
            printf("Jumper gave up chase, going back to sleep\n");
            isSleeping = true;
            hasLostPlayer = false;
            lostPlayerTimer = 0;
            chaseTimer = 0;
            applyGravity(platforms);
            return;
        }
    }
    else
    {
        if (hasLostPlayer)
        {
            hasLostPlayer = false;
            lostPlayerTimer = 0;
            printf("Jumper spotted player again!\n");
        }
    }

    if (distance < JUMPER_CHASE_RANGE)
    {
        chaseTimer += 1.0f / FPS;

        if (chaseTimer > maxChaseTime)
        {
            printf("Jumper getting tired, backing off!\n");
            isRetreating = true;
            retreatTimer = 2.0f + (rand() % 150) / 100.0f;
            chaseTimer = 0;
            applyGravity(platforms);
            return;
        }
    }

    chaseCooldown -= 1.0f / FPS;
    jumpCooldown -= 1.0f / FPS;

    if (distance < JUMPER_AGGRESSIVE_RANGE && chaseCooldown <= 0)
    {
        float chaseSpeed = speed * JUMPER_AGGRESSIVE_SPEED_MULT;
        if (fabs(dx) > 5)
        {
            x += (dx > 0 ? chaseSpeed : -chaseSpeed);
            facingLeft = (dx < 0);
        }

        if (distance < attackRange && attackCooldown <= 0)
        {
            isAttacking = true;
            attackCooldown = 1.2f;
            if (distance < 35.0f && !player.isInvincible)
            {
                player.loseHeart();
                printf("Jumper hit player with rolling pin! Backing off now.\n");

                isRetreating = true;
                retreatTimer = 1.5f + (rand() % 100) / 100.0f;
                chaseTimer = 0;
                isAttacking = false;
                return;
            }
        }

        if (attackCooldown > 0)
        {
            attackCooldown -= 1.0f / FPS;
        }
        if (isAttacking && attackCooldown <= 0.6f)
        {
            isAttacking = false;
        }

        if (isAttacking)
        {
            applyGravity(platforms);
            return;
        }

        if (onGround && jumpCooldown <= 0)
        {
            bool shouldJump = false;

            if (dy < -30)
            {
                shouldJump = true;
            }

            bool willWalkOffEdge = (dx > 0) ? isOnPlatformEdge(platforms, false)
                                            : isOnPlatformEdge(platforms, true);
            if (willWalkOffEdge && fabs(dx) > 20)
            {
                if (hasLandingPlatformAhead(platforms, dx > 0))
                {
                    shouldJump = true;
                }
            }

            if (!shouldJump && isBlockedHorizontally(platforms, dx > 0))
            {
                shouldJump = true;
            }

            if (shouldJump)
            {
                velocityY = JUMPER_AGGRESSIVE_JUMP_FORCE;
                onGround = false;
                jumpCooldown = JUMPER_AGGRESSIVE_JUMP_COOLDOWN;
            }
        }
    }
    else if (distance < JUMPER_CHASE_RANGE && chaseCooldown <= 0)
    {
        if (fabs(dx) > 5)
        {
            x += (dx > 0 ? speed : -speed);
            facingLeft = (dx < 0);
        }

        if (onGround && jumpCooldown <= 0)
        {
            bool shouldJump = false;

            if (dy < -50)
            {
                Platform *targetPlatform = findPlatformBetween(platforms, y, player.y, dx > 0);

                if (targetPlatform)
                {
                    shouldJump = true;
                }
            }

            bool willWalkOffEdge = (dx > 0) ? isOnPlatformEdge(platforms, false) : isOnPlatformEdge(platforms, true);

            if (willWalkOffEdge)
            {
                if (fabs(dx) > 20)
                {
                    if (hasLandingPlatformAhead(platforms, dx > 0))
                    {
                        shouldJump = true;
                    }
                }
                else
                {
                    x -= (dx > 0 ? speed : -speed);
                }
            }

            if (!shouldJump && isBlockedHorizontally(platforms, dx > 0))
            {
                shouldJump = true;
            }

            if (shouldJump)
            {
                velocityY = JUMPER_JUMP_FORCE;
                onGround = false;
                jumpCooldown = JUMPER_JUMP_COOLDOWN;
            }
        }
    }

    else if (distance >= JUMPER_CHASE_RANGE)
    {
        if (chaseCooldown <= 0)
        {
            chaseCooldown = JUMPER_CHASE_COOLDOWN;
        }

        if (onGround)
        {
            float idleSpeed = speed * 0.3f;
            x += idleSpeed * patrolDirection;

            bool atEdge = (patrolDirection == 1) ? isOnPlatformEdge(platforms, false) : isOnPlatformEdge(platforms, true);
            if (atEdge)
            {
                x -= idleSpeed * patrolDirection;
                patrolDirection *= -1;
            }
        }
    }

    applyGravity(platforms);
}

bool Enemy::hasLineOfSight(Player &player, const std::vector<Platform> &platforms)
{
    float dx = player.x - x;
    float dy = player.y - y;

    int steps = 10;
    for (int i = 1; i < steps; i++)
    {
        float checkX = x + (dx * i / steps);
        float checkY = y + (dy * i / steps);

        SDL_Rect checkPoint = {(int)checkX, (int)checkY, 4, 4};

        for (const auto &platform : platforms)
        {
            SDL_Rect platformRect = platform.getRect();
            if (SDL_HasIntersection(&checkPoint, &platformRect))
            {
                return false;
            }
        }
    }
    return true;
}

void Enemy::updateShooter(Player &player, std::vector<Projectile *> &projectiles, const std::vector<Platform> &platforms)
{
    if (!hasFoundEdges)
    {
        findPlatformEdges(platforms);
    }

    facingLeft = (player.x < x);

    float cooldownRate = 1.0f + (alertLevel * 0.3f);
    shootCooldown -= cooldownRate / FPS;

    if (!isAiming)
    {
        float currentSpeed = speed * (1.0f + alertLevel * 0.5f);
        x += currentSpeed * patrolDirection;

        bool shouldTurnAround = false;

        if (patrolDirection == -1)
        {
            if (isOnPlatformEdge(platforms, true) || x <= patrolLeft)
            {
                shouldTurnAround = true;
                x = patrolLeft;
            }
        }
        else
        {
            if (isOnPlatformEdge(platforms, false) || x >= patrolRight)
            {
                shouldTurnAround = true;
                x = patrolRight;
            }
        }

        if (shouldTurnAround)
        {
            patrolDirection *= -1;
        }

        // Only shoot if we have line of sight
        if (shootCooldown <= 0 && hasLineOfSight(player, platforms))
        {
            isAiming = true;
            aimTime = SHOOTER_AIM_TIME;
            printf("Shooter starting to aim! Player at (%.1f, %.1f), Enemy at (%.1f, %.1f)\n",
                   player.x, player.y, x, y);
        }
    }
    else
    {
        aimTime -= 1.0f / FPS;

        if (aimTime <= 0)
        {
            printf("SHOOTER FIRING PROJECTILE!\n");
            shootAtPlayer(player, projectiles);
            printf("Projectiles in vector: %d\n", (int)projectiles.size());

            isAiming = false;
            shootCooldown = shootInterval;
        }
    }
}

void Enemy::shootAtPlayer(Player &player, std::vector<Projectile *> &projectiles)
{
    // Predict where player will be, accounting for velocity AND gravity
    float timeToTarget = PROJECTILE_PREDICTION_TIME;
    float predictedX = player.x + (player.velocityX * timeToTarget * FPS);
    float predictedY = player.y + (player.velocityY * timeToTarget * FPS);

    if (!player.onGround)
    {
        predictedY += 0.5f * GRAVITY * timeToTarget * timeToTarget * FPS * FPS;
    }

    // Add slight randomness based on alert level (less alert = less accurate)
    float accuracy = 0.2f + (alertLevel * 0.3f);
    float randomOffset = (1.0f - accuracy) * 50.0f;
    predictedX += (rand() % 100 - 50) * randomOffset / 50.0f;
    predictedY += (rand() % 100 - 50) * randomOffset / 50.0f;

    float dx = predictedX - (x + width / 2);
    float dy = predictedY - (y + height / 2);
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < 10)
    {
        printf("Player too close to shoot (distance: %.1f)\n", distance);
        return;
    }

    // Normalize and scale
    float vx = (dx / distance) * PROJECTILE_SPEED;
    float vy = (dy / distance) * PROJECTILE_SPEED;

    printf("Creating projectile at (%.1f, %.1f) with velocity (%.1f, %.1f)\n",
           x + width / 2 - PROJECTILE_WIDTH / 2, y + height / 2 - PROJECTILE_HEIGHT / 2, vx, vy);

    Projectile *proj = new Projectile(
        x + width / 2 - PROJECTILE_WIDTH / 2,
        y + height / 2 - PROJECTILE_HEIGHT / 2,
        vx, vy);
    projectiles.push_back(proj);

    printf("Projectile created! Active: %d\n", proj->active);
}

void Enemy::applyGravity(const std::vector<Platform> &platforms)
{
    if (!onGround)
    {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED)
        {
            velocityY = MAX_FALL_SPEED;
        }
    }

    y += velocityY;

    onGround = false;
    SDL_Rect enemyRect = getRect();

    for (const auto &platform : platforms)
    {
        SDL_Rect platformRect = platform.getRect();

        if (SDL_HasIntersection(&enemyRect, &platformRect))
        {
            if (velocityY >= 0)
            {
                int enemyBottom = enemyRect.y + enemyRect.h;
                int platformTop = platformRect.y;

                if (enemyBottom >= platformTop && enemyBottom <= platformTop + 10)
                {
                    y = platformTop - height;
                    velocityY = 0;
                    onGround = true;
                }
            }
        }
    }
}

void Enemy::render(SDL_Renderer *renderer, float cameraY)
{
    SDL_Rect rect = getRect();
    rect.y -= cameraY;

    if (type == ENEMY_JUMPER && isSleeping)
    {
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderFillRect(renderer, &rect);

        // ZZZ
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
        SDL_Rect z1 = {rect.x + rect.w, rect.y - 10, 8, 8};
        SDL_Rect z2 = {rect.x + rect.w + 5, rect.y - 18, 10, 10};
        SDL_Rect z3 = {rect.x + rect.w + 10, rect.y - 28, 12, 12};
        SDL_RenderFillRect(renderer, &z1);
        SDL_RenderFillRect(renderer, &z2);
        SDL_RenderFillRect(renderer, &z3);
        return;
    }

    else if (type == ENEMY_JUMPER && !isSleeping)
    {
        if (isRetreating)
        {
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 150);
            SDL_Rect retreatBar = {rect.x, rect.y - 5, rect.w, 3};
            SDL_RenderFillRect(renderer, &retreatBar);
        }
        else if (chaseTimer > 0)
        {
            float exhaustionPercent = chaseTimer / maxChaseTime;
            if (exhaustionPercent > 0.5f)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, (Uint8)(exhaustionPercent * 150));
                SDL_Rect exhaustionBar = {rect.x, rect.y - 5, (int)(rect.w * exhaustionPercent), 3};
                SDL_RenderFillRect(renderer, &exhaustionBar);
            }
        }
    }

    // Baker body (grey)
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &rect);

    // Chef hat (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect hat = {rect.x + 5, rect.y - 10, 22, 10};
    SDL_RenderFillRect(renderer, &hat);

    // Alert indicator (red glow when alert)
    if (alertLevel > 0.3f)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, (Uint8)(alertLevel * 100));
        SDL_Rect alertGlow = {rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4};
        SDL_RenderDrawRect(renderer, &alertGlow);
    }

    if (type == ENEMY_JUMPER && !isSleeping && chaseTimer > 0)
    {
        float exhaustionPercent = chaseTimer / maxChaseTime;
        if (exhaustionPercent > 0.6f)
        { // Show when 60% exhausted
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, (Uint8)(exhaustionPercent * 150));
            SDL_Rect exhaustionBar = {rect.x, rect.y - 5, (int)(rect.w * exhaustionPercent), 3};
            SDL_RenderFillRect(renderer, &exhaustionBar);
        }
    }
    // Lost player indicator for jumper
    if (type == ENEMY_JUMPER && hasLostPlayer)
    {
        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 150);
        // ?
        SDL_Rect q = {rect.x + rect.w / 2 - 4, rect.y - 20, 8, 12};
        SDL_RenderFillRect(renderer, &q);
    }

    // Inactive indicator (blue tint when inactive)
    if (!isActive)
    {
        SDL_SetRenderDrawColor(renderer, 100, 100, 200, 100);
        SDL_RenderFillRect(renderer, &rect);
    }

    // Weapon indicator
    SDL_Rect weapon;
    switch (type)
    {
    case ENEMY_PATROL:
        if (type == ENEMY_PATROL && isAttacking)
        {
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 200);
            // Draw swing arc - need to fix
            SDL_Rect attackArc = {
                facingLeft ? rect.x - 25 : rect.x + rect.w,
                rect.y + 10,
                25, 30};
            SDL_RenderFillRect(renderer, &attackArc);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            if (facingLeft)
            {
                weapon = {rect.x - 5, rect.y + 20, 10, 15};
            }
            else
            {
                weapon = {rect.x + rect.w - 5, rect.y + 20, 10, 15};
            }
        }
        break;

    case ENEMY_JUMPER:
        SDL_SetRenderDrawColor(renderer, 200, 150, 100, 255);
        if (facingLeft)
        {
            weapon = {rect.x - 5, rect.y + 22, 15, 10};
        }
        else
        {
            weapon = {rect.x + rect.w - 10, rect.y + 22, 15, 10};
        }
        break;

    case ENEMY_SHOOTER:
        SDL_SetRenderDrawColor(renderer, 180, 140, 100, 255);

        if (isAiming)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);
            SDL_Rect aimIndicator = {rect.x + 8, rect.y + 15, 16, 4};
            SDL_RenderFillRect(renderer, &aimIndicator);
        }

        if (facingLeft)
        {
            weapon = {rect.x - 2, rect.y + 15, 8, 20};
        }
        else
        {
            weapon = {rect.x + rect.w - 6, rect.y + 15, 8, 20};
        }
        SDL_RenderFillRect(renderer, &weapon);
        break;
    }
    SDL_RenderFillRect(renderer, &weapon);
}

bool Enemy::checkCollision(Player &player)
{
    SDL_Rect playerRect = player.getRect();
    SDL_Rect enemyRect = getRect();

    return SDL_HasIntersection(&playerRect, &enemyRect);
}

SDL_Rect Enemy::getRect()
{
    return {(int)x, (int)y, (int)width, (int)height};
}