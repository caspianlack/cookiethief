#ifndef PLATFORM_H
#define PLATFORM_H

#include <SDL2/SDL.h>
#include "constants.h"

#include <vector>

enum PlatformType
{
    PLATFORM_DARK = 0,
    PLATFORM_MILK = 1,
    PLATFORM_WHITE = 2,
    PLATFORM_NONE = 3
};

enum TileState { TILE_HEALTHY, TILE_CRACKED, TILE_DESTROYED };

struct Tile
{
    TileState state = TILE_HEALTHY;
    float breakTimer = 0.0f;
};

struct Platform
{
    float x, y, width, height;
    SDL_Color color;
    PlatformType type;
    std::vector<Tile> tiles;

    Platform(float x = 0, float y = 0, float w = 0, float h = 0, SDL_Color c = {255, 255, 255, 255}, PlatformType t = PLATFORM_DARK)
        : x(x), y(y), width(w), height(h), color(c), type(t) 
    {
        // Initialize tiles based on width
        int numTilesX = (int)(w / 24); // PLATFORM_TILE_SIZE is 24
        if (numTilesX <= 0) numTilesX = 1;
        tiles.resize(numTilesX);
    }

    SDL_Rect getRect() const
    {
        return {(int)x, (int)y, (int)width, (int)height};
    }

    void render(SDL_Renderer *renderer) const
    {
        // Fallback to color if no texture is used or for simple cases
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = getRect();
        SDL_RenderFillRect(renderer, &rect);
    }
};

#endif