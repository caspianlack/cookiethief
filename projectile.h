#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL2/SDL.h>

class Player;

class Projectile {
public:
    float x, y;
    float velocityX, velocityY;
    float width, height;
    bool active;
    SDL_Color color;
    
    Projectile(float startX, float startY, float vx, float vy);
    
    void update();
    void render(SDL_Renderer* renderer, float cameraY = 0);
    bool checkCollision(const Player& player);
    SDL_Rect getRect() const;
};

#endif