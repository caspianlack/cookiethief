#include "heart_loss_overlay.h"
#include "constants.h"
#include "game.h"
#include <cmath>
#include <SDL2/SDL.h>

// ============================================================================
// HeartLossOverlay implementation
// ============================================================================

HeartLossOverlay::HeartLossOverlay(float playerScreenX, float playerScreenY,
                                   int heartsAfterHit, int maxHearts)
    : originX_(playerScreenX)
    , originY_(playerScreenY)
{
    // Mirror the renderHearts() layout to find the exact target slot.
    // renderHearts() puts slots at top-right, left-to-right:
    //   startX = SCREEN_WIDTH - 10 - (maxHearts * 25)
    //   heartX[i] = startX + i * 25   (heart size = 20, spacing = 25)
    //   startY = 10
    const int heartSize    = 20;
    const int heartSpacing = 25;
    const int startX = SCREEN_WIDTH - 10 - (maxHearts * heartSpacing);
    const int startY = 10;

    // The slot that was just lost is the one that will be grey:
    // it was the (heartsAfterHit)-th slot (0-indexed), i.e. the first empty slot.
    int slotIdx = heartsAfterHit; // 0-based index of the newly-empty slot
    targetX_ = (float)(startX + slotIdx * heartSpacing + heartSize / 2);
    targetY_ = (float)(startY + heartSize / 2);

    // Start heart at player centre
    heartX_ = originX_;
    heartY_ = originY_;
}

// ----------------------------------------------------------------------------
// Cubic Bezier interpolation for one axis
// ----------------------------------------------------------------------------
float HeartLossOverlay::bezier(float t, float p0, float cp1, float cp2, float p3)
{
    float u = 1.f - t;
    return u*u*u*p0
         + 3.f*u*u*t*cp1
         + 3.f*u*t*t*cp2
         + t*t*t*p3;
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void HeartLossOverlay::update(float dt, Game* /*game*/)
{
    timer_ += dt;

    switch (phase_)
    {
        case Phase::FLASH:
            // Fade the flash from 180 → 0 over FLASH_DUR
            flashAlpha_ = (Uint8)(180 * (1.f - timer_ / FLASH_DUR));
            if (timer_ >= FLASH_DUR)
            {
                phase_ = Phase::FREEZE;
                timer_ = 0.f;
            }
            break;

        case Phase::FREEZE:
            // Just wait; world is visible but frozen
            if (timer_ >= FREEZE_DUR)
            {
                phase_ = Phase::FLY;
                timer_ = 0.f;
            }
            break;

        case Phase::FLY:
        {
            float t = timer_ / FLY_DUR;
            if (t > 1.f) t = 1.f;

            // Control points create a satisfying arc:
            // CP1 shoots up-right from the origin; CP2 approaches from the left of target.
            float cp1x = originX_ + 150.f;
            float cp1y = originY_ - 250.f;
            float cp2x = targetX_ - 80.f;
            float cp2y = targetY_ + 80.f;

            heartX_ = bezier(t, originX_, cp1x, cp2x, targetX_);
            heartY_ = bezier(t, originY_, cp1y, cp2y, targetY_);

            if (timer_ >= FLY_DUR)
            {
                heartX_ = targetX_;
                heartY_ = targetY_;
                phase_  = Phase::LOCK;
                timer_  = 0.f;
                lockScale_ = 1.6f;
            }
            break;
        }

        case Phase::LOCK:
        {
            // Scale bounces from 1.6 → 1.0 with a small overshoot
            float t = timer_ / LOCK_DUR;
            if (t > 1.f) t = 1.f;

            // Simple spring-ish decay: overshoot at t=0.4, settle at t=1.0
            lockScale_ = 1.f + 0.6f * std::cos(t * 3.14159f * 1.5f) * (1.f - t);

            if (timer_ >= LOCK_DUR)
            {
                phase_ = Phase::DONE;
                done_ = true;
            }
            break;
        }

        case Phase::DONE:
            done_ = true;
            break;
    }
}

// ----------------------------------------------------------------------------
// Render — only draws the overlay elements; world is rendered by the stack.
// ----------------------------------------------------------------------------
void HeartLossOverlay::render(SDL_Renderer* renderer, Game* /*game*/)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // --- FLASH phase: full-screen red tint ---
    if (phase_ == Phase::FLASH && flashAlpha_ > 0)
    {
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, flashAlpha_);
        SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &full);
    }

    // --- FLY / LOCK phases: draw the flying heart ---
    if (phase_ == Phase::FLY || phase_ == Phase::LOCK)
    {
        const int BASE_SIZE = 20;
        int drawSize = (int)(BASE_SIZE * lockScale_);

        // Draw a simple red heart rectangle (matches the HUD heart style)
        int hx = (int)(heartX_ - drawSize / 2);
        int hy = (int)(heartY_ - drawSize / 2);

        // Shadow / trail
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 120);
        SDL_Rect shadow = {hx + 3, hy + 3, drawSize, drawSize};
        SDL_RenderFillRect(renderer, &shadow);

        // Main filled heart (bright red)
        SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
        SDL_Rect heart = {hx, hy, drawSize, drawSize};
        SDL_RenderFillRect(renderer, &heart);

        // Inner highlight (top-left corner, like pixel-art shine)
        SDL_SetRenderDrawColor(renderer, 255, 180, 180, 200);
        SDL_Rect shine = {hx + 3, hy + 3, drawSize / 3, drawSize / 4};
        SDL_RenderFillRect(renderer, &shine);

        // Outline
        SDL_SetRenderDrawColor(renderer, 120, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &heart);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
