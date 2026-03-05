#ifndef HEART_LOSS_OVERLAY_H
#define HEART_LOSS_OVERLAY_H

#include "screen.h"
#include <SDL2/SDL.h>

// ============================================================================
// HeartLossOverlay
//
// Plays when the player takes damage but survives (hearts > 0 after the hit).
//
// Animation sequence:
//   FLASH   (0.12s) — full-screen red flash
//   FREEZE  (0.35s) — game world frozen, player flickers
//   FLY     (1.0s)  — a heart icon arcs from the player position to the HUD slot
//   LOCK    (0.25s) — heart "pops" into the HUD slot with a scale bounce
//   DONE           — overlay removes itself, invincibility already running
//
// The HUD heart slots are rendered top-right. We calculate the exact pixel
// position of the slot that was just lost and fly the heart there.
// ============================================================================
class HeartLossOverlay : public Screen
{
public:
    // Call this constructor right after loseHeart() has been applied.
    // playerScreenX/Y — current screen-space centre of the player
    // heartsAfterHit   — value of player->hearts after the loss (for HUD slot targeting)
    // maxHearts        — player->maxHearts (used to compute HUD position)
    HeartLossOverlay(float playerScreenX, float playerScreenY,
                     int heartsAfterHit, int maxHearts);

    bool blocksRender() const override { return false; } // world shows through
    bool blocksUpdate() const override { return true;  } // game paused

    void handleEvent(SDL_Event& e, Game* game) override {}
    void update(float dt, Game* game) override;
    void render(SDL_Renderer* renderer, Game* game) override;

private:
    enum class Phase { FLASH, FREEZE, FLY, LOCK, DONE };
    Phase phase_  = Phase::FLASH;
    float timer_  = 0.f;

    // Animation constants
    static constexpr float FLASH_DUR  = 0.12f;
    static constexpr float FREEZE_DUR = 0.35f;
    static constexpr float FLY_DUR    = 1.00f;
    static constexpr float LOCK_DUR   = 0.25f;

    // Heart origin (player position at moment of hit)
    float originX_, originY_;

    // Heart destination (the HUD slot that just went grey)
    float targetX_, targetY_;

    // Current animated heart position
    float heartX_, heartY_;

    // Lock phase scale factor (bounces from 1.5 → 1.0)
    float lockScale_ = 1.5f;

    // Flash alpha
    Uint8 flashAlpha_ = 180;

    // Cubic Bezier: 0 <= t <= 1
    static float bezier(float t, float p0, float cp1, float cp2, float p3);
};

#endif // HEART_LOSS_OVERLAY_H
