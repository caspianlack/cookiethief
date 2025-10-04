#include "enemy.h"

Enemy::Enemy(float startX, float startY) {
    x = startX;
    y = startY;
    width = 32;
    height = 48;
    speed = 2.0f;
}

void Enemy::update(Player& player) {
    // Simple chase AI
    if (player.x > x) {
        x += speed;
    } else if (player.x < x) {
        x -= speed;
    }
}

void Enemy::render(SDL_Renderer* renderer) {
    // Draw baker body (grey)
    SDL_SetRenderDrawColor(renderer,  200, 200, 200, 255);
    SDL_Rect rect = getRect();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw chef hat (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect hat = {(int)x + 5, (int)y - 10, 22, 10};
    SDL_RenderFillRect(renderer, &hat);
}

bool Enemy::checkCollision(Player& player) {
    SDL_Rect playerRect = player.getRect();
    SDL_Rect enemyRect = getRect();
    
    return SDL_HasIntersection(&playerRect, &enemyRect);
}

SDL_Rect Enemy::getRect() {
    return {(int)x, (int)y, (int)width, (int)height};
}