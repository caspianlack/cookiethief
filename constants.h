#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Downwell pit dimensions
const int PIT_WIDTH = 400;  // The playable vertical shaft
const int PIT_LEFT = (SCREEN_WIDTH - PIT_WIDTH) / 2;  // 200
const int PIT_RIGHT = PIT_LEFT + PIT_WIDTH;  // 600

// Bomb Jack arena dimensions
const int ARENA_WIDTH = 500;
const int ARENA_HEIGHT = 550;
const int ARENA_LEFT = (SCREEN_WIDTH - ARENA_WIDTH) / 2;  // 150
const int ARENA_TOP = 25;

// Physics constants
const float GRAVITY = 0.5f;
const float GLIDE_FALL_SPEED = 2.0f;
const float JUMP_FORCE = -12.0f;
const float MOVE_SPEED = 5.0f;
const float MAX_FALL_SPEED = 15.0f;

// Gliding settings
const float MAX_GLIDE_TIME = 2.0f;

// Energy/Health system (Sugar Rush)
const float MAX_ENERGY = 100.0f;
const float ENERGY_DRAIN_RATE = 5.0f;
const int STARTING_HEARTS = 3;
const int MAX_HEARTS = 5;
const float COOKIE_ENERGY_RESTORE = 25.0f;
const float INVINCIBILITY_TIME = 1.0f;

// Sluggish state (low energy penalties)
const float SLUGGISH_MOVE_SPEED = 3.5f;
const float SLUGGISH_JUMP_FORCE = -10.0f;

// Enemy dimensions
const float ENEMY_WIDTH = 32.0f;
const float ENEMY_HEIGHT = 48.0f;

// Enemy: Patrol (Frying Pan)
const float PATROL_SPEED = 2.0f;
const float PATROL_PAUSE_MIN = 1.0f;
const float PATROL_PAUSE_MAX = 3.0f;
const int PATROL_PAUSE_CHANCE = 100;  // 1/100 chance per frame

// Enemy: Jumper (Rolling Pin)
const float JUMPER_SPEED = 3.5f;
const float JUMPER_JUMP_FORCE = -11.0f;
const float JUMPER_JUMP_COOLDOWN = 1.5f;
const float JUMPER_CHASE_COOLDOWN = 2.0f;
const float JUMPER_CHASE_RANGE = 400.0f;
const float JUMPER_AGGRESSIVE_RANGE = 200.0f;
const float JUMPER_AGGRESSIVE_SPEED_MULT = 1.3f;
const float JUMPER_AGGRESSIVE_JUMP_FORCE = -12.0f;
const float JUMPER_AGGRESSIVE_JUMP_COOLDOWN = 0.8f;

// Enemy: Shooter (Wooden Spoon)
const float SHOOTER_SPEED = 1.5f;
const float SHOOTER_SHOOT_INTERVAL = 3.0f;
const float SHOOTER_AIM_TIME = 0.8f;
const float SHOOTER_INITIAL_COOLDOWN = 2.0f;
const float SHOOTER_MIN_SHOOT_INTERVAL = 1.5f;  // With difficulty scaling

// Projectile settings
const float PROJECTILE_SPEED = 6.0f;
const float PROJECTILE_WIDTH = 12.0f;
const float PROJECTILE_HEIGHT = 12.0f;
const float PROJECTILE_GRAVITY = 0.2f;
const float PROJECTILE_PREDICTION_TIME = 0.5f;  // Predictive aiming

// Enemy AI
const float ENEMY_EDGE_CHECK_DISTANCE = 10.0f;
const float ENEMY_PLATFORM_MARGIN = 5.0f;
const float ENEMY_ALERT_INCREASE_RATE = 0.02f;
const float ENEMY_ALERT_DECREASE_RATE = 0.01f;
const float ENEMY_ALERT_RANGE = 300.0f;

const float ENEMY_ACTIVATION_RANGE = 400.0f;
const float ENEMY_DEACTIVATION_RANGE = 600.0f;


// Difficulty scaling per floor
const float ENEMY_SPEED_SCALE_PER_FLOOR = 0.1f;
const float SHOOTER_COOLDOWN_REDUCE_PER_FLOOR = 0.2f;
const float JUMPER_COOLDOWN_REDUCE_PER_FLOOR = 0.1f;

const float JUMPER_CHASE_TIME_MIN = 3.0f;   // Minimum chase duration
const float JUMPER_CHASE_TIME_MAX = 5.0f;   // Maximum chase duration
const float JUMPER_RETREAT_TIME_MIN = 1.5f; // How long to retreat after hit
const float JUMPER_RETREAT_TIME_MAX = 2.5f;
const float JUMPER_LOST_PLAYER_TIME = 2.0f; // Time before giving up when player is far

// Game settings
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

#endif