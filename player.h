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
    // Sprite sheet layout: 
    // Row 0: Idle (2 frames)
    // Row 1: Walk (4 frames)
    // Row 2: Die/Faint (4 frames)
    // Row 3: Jump (2 frames)
    enum AnimationState { IDLE, WALK, JUMP, FALL, GLIDE, DIE };
    AnimationState animState;
    AnimationState previousAnimState; // Track state changes to reset frame
    float animTimer;
    int currentFrame;
    int currentRow;      // Current sprite sheet row (set in updateAnimation)
    bool facingLeft;     // Track direction for flipping
    
    // Death animation timing
    float deathTimer;    // Time since death occurred
    float deathFadeAlpha; // Alpha for death screen overlay (0-255)

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