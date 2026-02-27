#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include <vector>
#include "player.h"

struct Platform;
class Projectile;

enum EnemyType
{
    ENEMY_PATROL,  // Frying pan  - patrols platform edges
    ENEMY_JUMPER,  // Rolling pin - chases and jumps at player
    ENEMY_BAKER,   // The Arch-Baker - chases player down the hole
    ENEMY_OVEN     // Oven - walks, stops, bakes, opens door, shoots cookies (replaces ENEMY_SHOOTER)
};

// State machine for the oven enemy
enum OvenState
{
    OVEN_WALKING,   // Row 1 - patrolling
    OVEN_BAKING,    // Row 2 - oven heating up (plays once then transitions)
    OVEN_OPENING,   // Row 3 - door opening   (plays once then transitions)
    OVEN_SHOOTING,  // Row 4 - cookies being fired (synced to projectiles)
    OVEN_DEAD       // Row 5 - placeholder death
};

class Enemy
{
public:
    float x, y;
    float width, height;
    float speed;
    float velocityY;
    bool onGround;
    EnemyType type;
    bool facingLeft;
    float retreatTimer;
    bool isRetreating;

    float attackCooldown;
    bool isAttacking;
    float attackRange;

    // Patrol/Shooter data
    float patrolLeft, patrolRight;
    int patrolDirection;
    bool hasFoundEdges;
    float pauseTimer;
    
    // Animation
    float animTimer;
    int currentFrame;
    int currentRow;        // Which sprite-sheet row is active
    bool isPaused;
    bool isSleeping;
    float wakeUpRange;

    // Oven-specific state machine
    OvenState ovenState;
    float ovenWalkTimer;    // How long left to walk before stopping
    int   ovenShotsFired;   // Cookies fired so far in current volley
    float ovenFrameDur;     // Current frame duration (set per-state)

    // Jumper data
    float jumpCooldown;
    float chaseCooldown;
    float chaseTimer;
    float maxChaseTime;
    bool hasLostPlayer;
    float lostPlayerTimer;

    // Shooter data
    float shootCooldown;
    float shootInterval;
    float aimTime;
    bool isAiming;

    // Alert system
    float alertLevel; // 0 - 1

    bool isActive;

    Enemy(float startX, float startY, EnemyType enemyType = ENEMY_PATROL, int difficulty = 0);

    void update(Player &player, const std::vector<Platform> &platforms = {}, std::vector<Projectile *> *projectiles = nullptr);

    void render(SDL_Renderer *renderer, float cameraY = 0);

    bool checkCollision(Player &player);
    SDL_Rect getRect();

private:
    void updatePatrol(Player &player, const std::vector<Platform> &platforms);
    void updateJumper(Player &player, const std::vector<Platform> &platforms);
    void updateShooter(Player &player, std::vector<Projectile *> &projectiles, const std::vector<Platform> &platforms);
    void shootAtPlayer(Player &player, std::vector<Projectile *> &projectiles);
    void applyGravity(const std::vector<Platform> &platforms);
    void findPlatformEdges(const std::vector<Platform> &platforms);
    bool isOnPlatformEdge(const std::vector<Platform> &platforms, bool checkLeft);

    bool hasLineOfSight(Player &player, const std::vector<Platform> &platforms);

    bool shouldActivate(const Player &player) const;
    bool shouldDeactivate(const Player &player) const;

    // Jumper AI helpers
    bool hasLandingPlatformAhead(const std::vector<Platform> &platforms, bool checkRight);
    Platform *findPlatformBetween(const std::vector<Platform> &platforms, float startY, float targetY, bool preferRight);
    bool isBlockedHorizontally(const std::vector<Platform> &platforms, bool checkRight);

    void updateBaker(Player &player);
    void updateOven(Player &player, const std::vector<Platform> &platforms, std::vector<Projectile *> &projectiles);
    void ovenFireCookie(Player &player, std::vector<Projectile *> &projectiles);
};

#endif