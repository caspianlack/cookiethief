#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>

class Player {
public:
    float x, y;
    float velocityX, velocityY;
    float width, height;
    bool onGround;
    bool isGliding;
    float glideTime;
    
    Player(float startX, float startY);
    
    void jump();
    void startGliding();
    void stopGliding();
    void moveLeft();
    void moveRight();
    void stopMoving();
    void update();
    void render(SDL_Renderer* renderer);
    
    SDL_Rect getRect();
};

#endif