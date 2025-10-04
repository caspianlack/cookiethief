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
}

void Player::jump() {
    if (onGround) {
        velocityY = JUMP_FORCE;
        onGround = false;
        isGliding = false;
        printf("    Player::jump() called - velocityY set to %f\n", velocityY);
    } else {
        printf("    Player::jump() BLOCKED - not on ground\n");
    }
}

void Player::startGliding() {
    // Can only glide if not on ground, has glide time
    if (!onGround && glideTime > 0) { // && velocityY > -5.0f for no gliding while jumping up
        isGliding = true;
    }
}

void Player::stopGliding() {
    isGliding = false;
}

void Player::moveLeft() {
    velocityX = -MOVE_SPEED;
}

void Player::moveRight() {
    velocityX = MOVE_SPEED;
}

void Player::stopMoving() {
    velocityX = 0;
}

void Player::update() {
    printf("  Player::update() START - velocityY: %f, isGliding: %d, onGround: %d\n", 
           velocityY, isGliding, onGround);
    
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
        
        // Cap fall speed (important more for downwell inspired levels)
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
    
    printf("  Player::update() END - velocityY: %f\n", velocityY);
}

void Player::render(SDL_Renderer* renderer) {

    if (isGliding) {
        SDL_SetRenderDrawColor(renderer, 191, 64, 191, 255);   // gliding sprite - todo replace with sprite and make directional
    } else {
        SDL_SetRenderDrawColor(renderer, 112, 41, 99, 255);   // not gliding sprite - todo replace with sprites and make direction as well as switch case for running, idle, start of jump, in jump, landing, hit and death
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
}

SDL_Rect Player::getRect() {
    return {(int)x, (int)y, (int)width, (int)height};
}