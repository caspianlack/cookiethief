#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include "player.h"

class Enemy {
public:
    float x, y;
    float width, height;
    float speed;
    
    Enemy(float startX, float startY);
    
    void update(Player& player);
    void render(SDL_Renderer* renderer);
    bool checkCollision(Player& player);
    
    SDL_Rect getRect();
};

#endif