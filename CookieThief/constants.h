#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Player sprite scaling and hitbox configuration
const int PLAYER_SPRITE_SIZE = 32;        // Source sprite size from sprite sheet
const int PLAYER_RENDER_SCALE = 3;        // Scale factor (3 = 96x96 rendered)
const int PLAYER_RENDER_SIZE = PLAYER_SPRITE_SIZE * PLAYER_RENDER_SCALE; // 96x96

// Hitbox offsets within the 32x32 sprite space (before scaling)
const int HITBOX_LEFT_OFFSET = 10;   // Pixels from left edge
const int HITBOX_RIGHT_OFFSET = 13;  // Pixels from right edge
const int HITBOX_TOP_OFFSET = 10;    // Pixels from top edge
const int HITBOX_BOTTOM_OFFSET = 9;  // Pixels from bottom edge (where feet are)

// Calculated hitbox dimensions (scaled up by PLAYER_RENDER_SCALE)
const int HITBOX_WIDTH = (PLAYER_SPRITE_SIZE - HITBOX_LEFT_OFFSET - HITBOX_RIGHT_OFFSET) * PLAYER_RENDER_SCALE;   // 9 * 3 = 27px
const int HITBOX_HEIGHT = (PLAYER_SPRITE_SIZE - HITBOX_TOP_OFFSET - HITBOX_BOTTOM_OFFSET) * PLAYER_RENDER_SCALE; // 13 * 3 = 39px

// Downwell pit dimensions
const int PIT_WIDTH = 400;                           // The playable vertical shaft
const int PIT_LEFT = (SCREEN_WIDTH - PIT_WIDTH) / 2; // 200
const int PIT_RIGHT = PIT_LEFT + PIT_WIDTH;          // 600

// Bomb Jack arena dimensions
const int ARENA_WIDTH = 500;
const int ARENA_HEIGHT = 550;
const int ARENA_LEFT = (SCREEN_WIDTH - ARENA_WIDTH) / 2; // 150
const int ARENA_TOP = 25;

// Physics constants
const float JUMP_GRAVITY = 0.45f;
const float FALL_GRAVITY = 0.25f; // Slower acceleration when falling
const float GLIDE_FALL_SPEED = 2.0f;
const float JUMP_FORCE = -12.0f;
const float MOVE_SPEED = 5.0f;
const float MAX_FALL_SPEED = 22.0f;
const float PLATFORM_BREAK_SPEED = 14.0f; // Smashing is easier to trigger now
const float PLATFORM_BREAK_RESISTANCE = 0.1f; // Multiply velocity by this when smashing (30% loss)

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
const int PATROL_PAUSE_CHANCE = 100; // 1/100 chance per frame

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
const float SHOOTER_MIN_SHOOT_INTERVAL = 1.5f; // With difficulty scaling

// Projectile settings
const float PROJECTILE_SPEED = 6.0f;
const float PROJECTILE_WIDTH = 12.0f;
const float PROJECTILE_HEIGHT = 12.0f;
const float PROJECTILE_GRAVITY = 0.2f;
const float PROJECTILE_PREDICTION_TIME = 0.5f; // Predictive aiming

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

// Tileset constants
const int TILE_SIZE = 8;
const int PLATFORM_RENDER_SCALE = PLAYER_RENDER_SCALE; // Match player scale (3x)
const int PLATFORM_TILE_SIZE = TILE_SIZE * PLATFORM_RENDER_SCALE; // 24px

const int PLATFORM_TILE_DARK = 0;
const int PLATFORM_TILE_MILK = 1;
const int PLATFORM_TILE_WHITE = 2;
const int PLATFORM_TILE_EMPTY = 3;

// ============================================================
// Oven Enemy sprite sheet configuration
// Sprite sheet: 32x32 tiles, 3x scale -> 96x96 rendered
// The actual oven art is ~16x16 centered in the 32x32 tile.
// Bottom of oven sits 9px from the bottom of the 32x32 tile.
// ============================================================
const int OVEN_SPRITE_SIZE   = 32;   // Source tile size
const int OVEN_RENDER_SCALE  = 3;    // Same 3x scale as player
const int OVEN_RENDER_SIZE   = OVEN_SPRITE_SIZE * OVEN_RENDER_SCALE; // 96px

// Hitbox offsets within the 32x32 tile (before scaling)
// 9px in on each side horizontally, 5px from top, 9px from bottom
const int OVEN_HITBOX_LEFT   =  9;
const int OVEN_HITBOX_RIGHT  =  9;
const int OVEN_HITBOX_TOP    =  5;
const int OVEN_HITBOX_BOTTOM =  9;

// Calculated hitbox dimensions (scaled)
const int OVEN_HITBOX_W = (OVEN_SPRITE_SIZE - OVEN_HITBOX_LEFT - OVEN_HITBOX_RIGHT)  * OVEN_RENDER_SCALE; // 14*3 = 42
const int OVEN_HITBOX_H = (OVEN_SPRITE_SIZE - OVEN_HITBOX_TOP  - OVEN_HITBOX_BOTTOM) * OVEN_RENDER_SCALE; // 18*3 = 54

// Oven animation rows (0-indexed)
const int OVEN_ROW_IDLE   = 0; // 5 frames  (unused for now)
const int OVEN_ROW_WALK   = 1; // 4 frames
const int OVEN_ROW_BAKE   = 2; // baking / oven heating  (frame count TBD)
const int OVEN_ROW_OPEN   = 3; // oven door opening      (5 frames)
const int OVEN_ROW_SHOOT  = 4; // cookies leaving oven   (6 frames synced to shots)
const int OVEN_ROW_DEATH  = 5; // death (1 frame placeholder)

const int OVEN_FRAMES_IDLE  = 5;
const int OVEN_FRAMES_WALK  = 4;
const int OVEN_FRAMES_BAKE  = 4; // adjust to actual frame count in sheet
const int OVEN_FRAMES_OPEN  = 5;
const int OVEN_FRAMES_SHOOT = 6;
const int OVEN_FRAMES_DEATH = 1;

// Oven behaviour timings
const float OVEN_WALK_SPEED          = 1.2f;
const float OVEN_WALK_MIN_TIME       = 2.0f;  // How long it walks before stopping
const float OVEN_WALK_MAX_TIME       = 4.0f;
const float OVEN_BAKE_FRAME_DUR      = 0.18f; // seconds per bake frame
const float OVEN_OPEN_FRAME_DUR      = 0.14f; // seconds per door-open frame
// shoot row: each cookie firing occupies 2 frames (1 cookie gone + 1 transition)
// With 3 cookies => 6 frames total. Fire cookie on the "gone" frames: 0, 2, 4.
const float OVEN_SHOOT_FRAME_DUR     = 0.15f; // seconds per shoot-row frame
const float OVEN_WALK_FRAME_DUR      = 0.18f; // seconds per walk frame

#endif