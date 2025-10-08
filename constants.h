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

// Game settings
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

#endif