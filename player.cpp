#include "player.h"
#include "constants.h"
#include <cstdio>

Player::Player(float startX, float startY) {
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
}

void Player::jump() {
    if (onGround) {
        // Weaker jump when sluggish (no energy)
        if (isSluggish()) {
            velocityY = SLUGGISH_JUMP_FORCE;
            printf("    Player::jump() called - SLUGGISH jump, velocityY set to %f\n", velocityY);
        } else {
            velocityY = JUMP_FORCE;
            printf("    Player::jump() called - velocityY set to %f\n", velocityY);
        }
        onGround = false;
        isGliding = false;
    } else {
        printf("    Player::jump() BLOCKED - not on ground\n");
    }
}

void Player::startGliding() {
    // Can't glide when sluggish (no energy)
    if (isSluggish()) {
        printf("    Can't glide - no energy! Need cookies!\n");
        return;
    }
    
    // Can only glide if not on ground, has glide time
    if (!onGround && glideTime > 0) {
        isGliding = true;
    }
}

void Player::stopGliding() {
    isGliding = false;
}

void Player::moveLeft() {
    // Slower movement when sluggish
    if (isSluggish()) {
        velocityX = -SLUGGISH_MOVE_SPEED;
    } else {
        velocityX = -MOVE_SPEED;
    }
}

void Player::moveRight() {
    // Slower movement when sluggish
    if (isSluggish()) {
        velocityX = SLUGGISH_MOVE_SPEED;
    } else {
        velocityX = MOVE_SPEED;
    }
}

void Player::stopMoving() {
    velocityX = 0;
}

void Player::restoreEnergy(float amount) {
    energy += amount;
    if (energy > maxEnergy) {
        energy = maxEnergy;
    }
    printf("Energy restored! Current energy: %.1f\n", energy);
}

bool Player::isSluggish() {
    return energy <= 0;
}

void Player::loseHeart() {
    if (isInvincible) {
        return; // Don't lose hearts while invincible
    }
    
    hearts--;
    isInvincible = true;
    invincibilityTimer = INVINCIBILITY_TIME;
    
    if (hearts <= 0) {
        hearts = 0;
        isDead = true;
        printf("Player died! Game Over\n");
    } else {
        printf("Hit by enemy! Hearts remaining: %d (Invincible for %.1fs)\n", hearts, INVINCIBILITY_TIME);
    }
}

void Player::reset(float startX, float startY) {
    x = startX;
    y = startY;
    velocityX = 0;
    velocityY = 0;
    onGround = false;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;
    energy = MAX_ENERGY;
    hearts = STARTING_HEARTS;
    isDead = false;
    isInvincible = false;
    invincibilityTimer = 0.0f;
}

void Player::update() {
    if (isDead) {
        return; // Don't update if dead
    }
    
    printf("  Player::update() START - velocityY: %f, isGliding: %d, onGround: %d\n", 
           velocityY, isGliding, onGround);
    
    // Update invincibility timer
    if (isInvincible) {
        invincibilityTimer -= 1.0f / FPS;
        if (invincibilityTimer <= 0) {
            isInvincible = false;
            invincibilityTimer = 0;
            printf("Invincibility ended\n");
        }
    }
    
    // Drain energy over time (sugar crash)
    energy -= ENERGY_DRAIN_RATE / FPS;
    if (energy < 0) {
        energy = 0;
    }
    
    // Stop gliding if energy runs out mid-glide
    if (isSluggish() && isGliding) {
        isGliding = false;
        printf("Sugar crash! Can't glide anymore - need cookies!\n");
    }
    
    // Gliding: constant fall speed
    if (isGliding && glideTime > 0 && !onGround) {
        velocityY = GLIDE_FALL_SPEED;
        glideTime -= 1.0f / FPS;
        printf("    GLIDING - velocityY forced to %f\n", GLIDE_FALL_SPEED);
        
        if (glideTime <= 0) {
            glideTime = 0;
            isGliding = false;
        }
    } else {
        // Gravity when not gliding
        velocityY += GRAVITY;
        if (!onGround) {
            isGliding = false;
        }
        
        // Cap fall speed
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }
    
    // Reset glide when on ground
    if (onGround) {
        glideTime = MAX_GLIDE_TIME;
    }
    
    x += velocityX;
    y += velocityY;
    
    if (x < 0) x = 0;
    if (x + width > SCREEN_WIDTH) x = SCREEN_WIDTH - width;
    
    if (y + height > SCREEN_HEIGHT) {
        y = SCREEN_HEIGHT - height;
        velocityY = 0;
        onGround = true;
        isGliding = false;
    }
    
    printf("  Player::update() END - velocityY: %f, energy: %.1f, hearts: %d\n", 
           velocityY, energy, hearts);
}

void Player::render(SDL_Renderer* renderer) {
    if (isDead) {
        return; // Don't render if dead
    }

    // Flash player when invincible (blink effect)
    if (isInvincible) {
        // Blink every 0.1 seconds
        int blinkInterval = (int)(invincibilityTimer * 10) % 2;
        if (blinkInterval == 0) {
            return; // Skip rendering this frame to create blink effect
        }
    }

    if (isGliding) {
        SDL_SetRenderDrawColor(renderer, 191, 64, 191, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 112, 41, 99, 255);
    }
    SDL_Rect playerRect = getRect();
    SDL_RenderFillRect(renderer, &playerRect);
    
    // Draw Neesa more human for test
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect hair = {(int)x, (int)y, (int)width, (int)height / 3};  // Full width, 1/3 height, at top
    SDL_RenderFillRect(renderer, &hair);

    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    SDL_Rect face = {(int)x + 5, (int)y + 5, 22, 15};
    SDL_RenderFillRect(renderer, &face);

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
    if (energyPercent > 0.5f) {
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gold (sugar rush!)
    } else if (energyPercent > 0.25f) {
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange (fading)
    } else if (energyPercent > 0) {
        SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255); // Red (crashing)
    } else {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Gray (0)
    }
    SDL_Rect energyMeter = {barX, energyBarY, (int)(barWidth * energyPercent), barHeight};
    SDL_RenderFillRect(renderer, &energyMeter);
}

SDL_Rect Player::getRect() {
    return {(int)x, (int)y, (int)width, (int)height};
}