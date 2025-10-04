#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Physics constants
const float GRAVITY = 0.5f;
const float GLIDE_FALL_SPEED = 2.0f;
const float JUMP_FORCE = -12.0f;
const float MOVE_SPEED = 5.0f;
const float MAX_FALL_SPEED = 15.0f;

// Gliding settings
const float MAX_GLIDE_TIME = 2.0f;

// Game settings
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

#endif