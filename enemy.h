#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include <vector>
#include "player.h"

struct Platform;
class Projectile;

enum EnemyType {
    ENEMY_PATROL,   // Frying pan - patrols platform edges
    ENEMY_JUMPER,   // Rolling pin - chases and jumps at player
    ENEMY_SHOOTER   // Wooden spoon - shoots projectiles from platform edges
};


class Enemy {
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
    bool isPaused;
    bool isSleeping;
    float wakeUpRange;
    
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
    
    void update(Player& player, const std::vector<Platform>& platforms = {}, std::vector<Projectile*>* projectiles = nullptr);
    
    void render(SDL_Renderer* renderer, float cameraY = 0);
    
    bool checkCollision(Player& player);
    SDL_Rect getRect();

private:
    void updatePatrol(Player& player, const std::vector<Platform>& platforms);
    void updateJumper(Player& player, const std::vector<Platform>& platforms);
    void updateShooter(Player& player, std::vector<Projectile*>& projectiles, const std::vector<Platform>& platforms);
    void shootAtPlayer(Player& player, std::vector<Projectile*>& projectiles);
    void applyGravity(const std::vector<Platform>& platforms);
    void findPlatformEdges(const std::vector<Platform>& platforms);
    bool isOnPlatformEdge(const std::vector<Platform>& platforms, bool checkLeft);
    
    bool hasLineOfSight(Player& player, const std::vector<Platform>& platforms);
    
    bool shouldActivate(const Player& player) const;
    bool shouldDeactivate(const Player& player) const;
    
    // Jumper AI helpers
    bool hasLandingPlatformAhead(const std::vector<Platform>& platforms, bool checkRight);
    Platform* findPlatformBetween(const std::vector<Platform>& platforms, float startY, float targetY, bool preferRight);
    bool isBlockedHorizontally(const std::vector<Platform>& platforms, bool checkRight);
};

#endif