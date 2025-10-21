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
    
    // Energy/Health system (Sugar Rush)
    float energy;
    float maxEnergy;
    int hearts;
    int maxHearts;
    bool isDead;
    bool isInvincible;
    float invincibilityTimer;
    
    // Stomp mechanics
    bool isStomping;
    float stompBounce;
    
    bool isSluggish();
    
    Player(float startX, float startY);
    
    void jump();
    void startGliding();
    void stopGliding();
    void moveLeft();
    void moveRight();
    void stopMoving();
    void update();
    void render(SDL_Renderer* renderer, bool showBars = true);
    
    void setPosition(float newX, float newY);
    void restoreEnergy(float amount);
    void loseHeart();
    void reset(float startX, float startY);
    
    SDL_Rect getRect() const;
    SDL_Rect getFeetRect() const;
};

#endif