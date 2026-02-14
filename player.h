#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>

class Player
{
public:
    float x, y;
    float velocityX, velocityY;
    float width, height;
    
    // Sprite dimensions (32x32 tiles from Aseprite)
    static const int SPRITE_SIZE = 32;
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

    // Animation
    // Aseprite sheet layout: 4 columns × 12 rows, 32x32 tiles
    // Row 2 (index 1): Idle (2 frames)
    // Row 5 (index 4): Walk (2 frames)
    // Row 11 (index 10): Death/Faint (4 frames)
    enum AnimationState { IDLE, WALK, JUMP, FALL, GLIDE, DIE };
    AnimationState animState;
    float animTimer;
    int currentFrame;
    bool facingLeft; // Track direction for flipping

    void updateAnimation();
    SDL_Rect getSpriteSrcRect();

    bool isSluggish();

    Player(float startX, float startY);

    void jump();
    void startGliding();
    void stopGliding();
    void moveLeft();
    void moveRight();
    void stopMoving();
    void update();
    void render(SDL_Renderer *renderer, bool showBars = true);

    void setPosition(float newX, float newY);
    void restoreEnergy(float amount);
    void loseHeart();
    void reset(float startX, float startY);

    SDL_Rect getRect() const;
    SDL_Rect getFeetRect() const;
};

#endif