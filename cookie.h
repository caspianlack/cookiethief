#ifndef COOKIE_H
#define COOKIE_H

#include <SDL2/SDL.h>
#include "player.h"

class Cookie {
public:
    float x, y;
    float size;
    bool collected;
    
    Cookie(float startX, float startY);
    
    void render(SDL_Renderer* renderer);
    bool checkCollision(Player& player);
    
    SDL_Rect getRect();
};

#endif