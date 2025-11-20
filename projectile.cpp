#include "projectile.h"
#include "player.h"

Projectile::Projectile(float startX, float startY, float vx, float vy)
{
    x = startX;
    y = startY;
    velocityX = vx;
    velocityY = vy;
    width = 12;
    height = 12;
    active = true;
    color = {255, 200, 150, 255}; // Cookie dough color
}

void Projectile::update()
{
    x += velocityX;
    y += velocityY;

    // Slight gravity
    velocityY += 0.2f;

    // Deactivate if off screen
    if (y > 10000 || y < -1000 || x < -200 || x > 1000)
    {
        active = false;
    }
}

void Projectile::render(SDL_Renderer *renderer, float cameraY)
{
    SDL_Rect rect = {
        (int)x,
        (int)(y - cameraY),
        (int)width,
        (int)height};

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

bool Projectile::checkCollision(const Player &player)
{
    if (!active)
        return false;

    SDL_Rect projectileRect = getRect();
    SDL_Rect playerRect = player.getRect();

    return SDL_HasIntersection(&projectileRect, &playerRect);
}

SDL_Rect Projectile::getRect() const
{
    return {(int)x, (int)y, (int)width, (int)height};
}