#ifndef PLATFORM_H
#define PLATFORM_H

#include <SDL2/SDL.h>

struct Platform
{
    float x, y, width, height;
    SDL_Color color;

    SDL_Rect getRect() const
    {
        return {(int)x, (int)y, (int)width, (int)height};
    }

    void render(SDL_Renderer *renderer) const
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = getRect();
        SDL_RenderFillRect(renderer, &rect);
    }
};

#endif