#include "player.h"
#include "constants.h"
#include <cstdio>
#include <cmath>

Player::Player(float startX, float startY)
{
    x = startX;
    y = startY;
    width = 32;
    height = 48;
    velocityX = 0;
    velocityY = 0;
    onGround = false;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;

    // Initialize energy/health (Sugar Rush system)
    energy = MAX_ENERGY;
    maxEnergy = MAX_ENERGY;
    hearts = STARTING_HEARTS;
    maxHearts = MAX_HEARTS;
    isDead = false;
    isInvincible = false;
    invincibilityTimer = 0.0f;

    // Initialize stomp variables
    isStomping = false;
    stompBounce = -10.0f;

    // Animation
    animState = IDLE;
    animTimer = 0.0f;
    currentFrame = 0;
    facingLeft = false;
}

void Player::updateAnimation()
{
    // 1. Determine State
    if (isDead) {
        animState = DIE;
    } else if (isGliding) {
        animState = GLIDE;
    } else if (!onGround) {
        // Jump or Fall?
        if (velocityY < 0) animState = JUMP;
        else animState = JUMP; // Use jump frame for falling too, or maybe second frame of jump
    } else {
        if (fabs(velocityX) > 0.1f) {
            animState = WALK;
        } else {
            animState = IDLE;
        }
    }

    // 2. Update Timer & Frame
    animTimer += 1.0f / 60.0f; // Assuming 60 FPS update
    float frameDuration = 0.2f; // Default 5 FPS

    int numFrames = 1;
    int row = 0;

    // Aseprite sheet mapping (4 columns × 12 rows):
    // Row 2 (index 1): Idle - 2 frames
    // Row 5 (index 4): Walk - 2 frames
    // Row 11 (index 10): Death/Faint - 4 frames
    // We'll use row 4 for jump/glide as well for now
    
    switch (animState)
    {
    case IDLE:
        numFrames = 2; row = 1; frameDuration = 0.5f; break;
    case WALK:
        numFrames = 2; row = 4; frameDuration = 0.15f; break;
    case JUMP:
        numFrames = 2; row = 4; frameDuration = 0.2f; break; // Use walk frames for jump
    case DIE:
        numFrames = 4; row = 10; frameDuration = 0.2f; break; // 4 frames for death
    case GLIDE:
        numFrames = 1; row = 1; break; // Use first idle frame for glide
    default: break;
    }

    if (animTimer >= frameDuration)
    {
        animTimer = 0;
        currentFrame = (currentFrame + 1) % numFrames;
        
        // Don't loop death animation
        if (animState == DIE && currentFrame == numFrames - 1) {
            currentFrame = numFrames - 1;
        }
    }
}

SDL_Rect Player::getSpriteSrcRect()
{
    int row = 0;
    switch (animState) {
        case IDLE: row = 1; break;  // Row 2 in Aseprite (index 1)
        case WALK: row = 4; break;  // Row 5 in Aseprite (index 4)
        case JUMP: row = 4; break;  // Use walk row for jump
        case DIE:  row = 10; break; // Row 11 in Aseprite (index 10)
        case GLIDE: row = 4; break; // Use walk row for glide
        default: row = 1; break;
    }
    
    // Fall logic: use 2nd frame of Walk row
    if (animState == JUMP && velocityY > 0) {
        // Let animation cycle naturally
    }

    // Return source rectangle (32x32 tiles)
    return {currentFrame * SPRITE_SIZE, row * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
}

void Player::jump()
{
    if (onGround)
    {
        // Weaker jump when sluggish (no energy)
        if (isSluggish())
        {
            velocityY = SLUGGISH_JUMP_FORCE;
            // printf("    Player::jump() called - SLUGGISH jump, velocityY set to %f\n", velocityY);
        }
        else
        {
            velocityY = JUMP_FORCE;
            // printf("    Player::jump() called - velocityY set to %f\n", velocityY);
        }
        onGround = false;
        isGliding = false;
    }
    else
    {
        // printf("    Player::jump() BLOCKED - not on ground\n");
    }
}

void Player::startGliding()
{
    // Can't glide when sluggish (no energy)
    if (isSluggish())
    {
        // printf("    Can't glide - no energy! Need cookies!\n");
        return;
    }

    // Can only glide if not on ground, has glide time
    if (!onGround && glideTime > 0)
    {
        isGliding = true;
    }
}

void Player::stopGliding()
{
    isGliding = false;
}

void Player::moveLeft()
{
    // Slower movement when sluggish
    if (isSluggish())
    {
        velocityX = -SLUGGISH_MOVE_SPEED;
    }
    else
    {
        velocityX = -MOVE_SPEED;
    }
}

void Player::moveRight()
{
    // Slower movement when sluggish
    if (isSluggish())
    {
        velocityX = SLUGGISH_MOVE_SPEED;
    }
    else
    {
        velocityX = MOVE_SPEED;
    }
}

void Player::stopMoving()
{
    velocityX = 0;
}

void Player::restoreEnergy(float amount)
{
    energy += amount;
    if (energy > maxEnergy)
    {
        energy = maxEnergy;
    }
    // printf("Energy restored! Current energy: %.1f\n", energy);
}

bool Player::isSluggish()
{
    return energy <= 0;
}

void Player::loseHeart()
{
    if (isInvincible)
    {
        return; // Don't lose hearts while invincible
    }

    hearts--;
    isInvincible = true;
    invincibilityTimer = INVINCIBILITY_TIME;

    if (hearts <= 0)
    {
        hearts = 0;
        isDead = true;
        // printf("Player died! Game Over\n");
    }
    else
    {
        // printf("Hit by enemy! Hearts remaining: %d (Invincible for %.1fs)\n", hearts, INVINCIBILITY_TIME);
    }
}

// FULL RESET... want to keep some data persistent when moving between levels... like hearts
void Player::reset(float startX, float startY)
{
    x = startX;
    y = startY;
    velocityX = 0;
    velocityY = 0;

    hearts = STARTING_HEARTS;
    maxHearts = STARTING_HEARTS;
    energy = MAX_ENERGY;
    maxEnergy = MAX_ENERGY;

    onGround = false;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;
    isDead = false;
    isInvincible = false;
    invincibilityTimer = 0;
}

void Player::update()
{
    if (isDead)
    {
        return;
    }

    // Update invincibility timer
    if (isInvincible)
    {
        invincibilityTimer -= 1.0f / FPS;
        if (invincibilityTimer <= 0)
        {
            isInvincible = false;
            invincibilityTimer = 0;
        }
    }

    // ONLY drain energy when gliding (not just existing!)
    if (isGliding && glideTime > 0 && !onGround)
    {
        energy -= ENERGY_DRAIN_RATE / FPS;
        if (energy < 0)
        {
            energy = 0;
        }
    }

    // Stop gliding if energy runs out
    if (isSluggish() && isGliding)
    {
        isGliding = false;
        printf("Sugar crash! Can't glide anymore - need cookies!\n");
    }

    // Stomp detection
    if (velocityY > 0 && !onGround)
    {
        isStomping = true;
    }
    else
    {
        isStomping = false;
    }

    // Gliding: constant fall speed
    if (isGliding && glideTime > 0 && !onGround)
    {
        velocityY = GLIDE_FALL_SPEED;
        glideTime -= 1.0f / FPS;

        if (glideTime <= 0)
        {
            glideTime = 0;
            isGliding = false;
        }
    }
    else
    {
        // Gravity when not gliding
        velocityY += GRAVITY;
        if (!onGround)
        {
            isGliding = false;
        }

        // Cap fall speed
        if (velocityY > MAX_FALL_SPEED)
        {
            velocityY = MAX_FALL_SPEED;
        }
    }

    // Reset glide when on ground
    if (onGround)
    {
        glideTime = MAX_GLIDE_TIME;
    }

    x += velocityX;
    y += velocityY;

    // Update facing direction
    if (velocityX < -0.1f) facingLeft = true;
    if (velocityX > 0.1f) facingLeft = false;

    updateAnimation();
}

void Player::render(SDL_Renderer *renderer, bool showBars)
{
    if (isDead)
    {
        return; // Don't render if dead
    }

    // Flash player when invincible (blink effect)
    if (isInvincible)
    {
        // Blink every 0.1 seconds
        int blinkInterval = (int)(invincibilityTimer * 10) % 2;
        if (blinkInterval == 0)
        {
            return; // Skip rendering this frame to create blink effect
        }
    }

    if (isGliding)
    {
        SDL_SetRenderDrawColor(renderer, 191, 64, 191, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 112, 41, 99, 255);
    }
    SDL_Rect playerRect = getRect();
    SDL_RenderFillRect(renderer, &playerRect);

    // Draw Neesa more human for test
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect hair = {(int)x, (int)y, (int)width, (int)height / 3}; // Full width, 1/3 height, at top
    SDL_RenderFillRect(renderer, &hair);

    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    SDL_Rect face = {(int)x + 5, (int)y + 5, 22, 15};
    SDL_RenderFillRect(renderer, &face);

    // Only show bars if requested (hide in lobby)
    if (!showBars)
    {
        return;
    }

    // Draw glide bar above player
    int barWidth = 30;
    int barHeight = 4;
    int barX = (int)x + 1;
    int barY = (int)y - 10;

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect barBg = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &barBg);

    float glidePercent = glideTime / MAX_GLIDE_TIME;
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_Rect glideMeter = {barX, barY, (int)(barWidth * glidePercent), barHeight};
    SDL_RenderFillRect(renderer, &glideMeter);

    // Draw energy bar above glide bar (Sugar Rush meter)
    int energyBarY = barY - 8;
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect energyBarBg = {barX, energyBarY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &energyBarBg);

    float energyPercent = energy / maxEnergy;
    // Color based on energy level
    if (energyPercent > 0.5f)
    {
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gold (sugar rush!)
    }
    else if (energyPercent > 0.25f)
    {
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange (fading)
    }
    else if (energyPercent > 0)
    {
        SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255); // Red (crashing)
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Gray (0)
    }
    SDL_Rect energyMeter = {barX, energyBarY, (int)(barWidth * energyPercent), barHeight};
    SDL_RenderFillRect(renderer, &energyMeter);
}

void Player::setPosition(float newX, float newY)
{
    x = newX;
    y = newY;
    velocityX = 0;
    velocityY = 0;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;
}

SDL_Rect Player::getRect() const
{
    return {(int)x, (int)y, (int)width, (int)height};
}

SDL_Rect Player::getFeetRect() const
{
    return {(int)x, (int)(y + height * 0.66f), (int)width, (int)(height * 0.34f)};
}