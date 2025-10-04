#include "cookie.h"

Cookie::Cookie(float startX, float startY) {
    x = startX;
    y = startY;
    size = 20;
    collected = false;
}

void Cookie::render(SDL_Renderer* renderer) {
    if (!collected) {
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
        SDL_Rect rect = getRect();
        SDL_RenderFillRect(renderer, &rect);
    }
}

bool Cookie::checkCollision(Player& player) {
    SDL_Rect playerRect = player.getRect();
    SDL_Rect cookieRect = getRect();
    
    return SDL_HasIntersection(&playerRect, &cookieRect);
}

SDL_Rect Cookie::getRect() {
    return {(int)x, (int)y, (int)size, (int)size};
}