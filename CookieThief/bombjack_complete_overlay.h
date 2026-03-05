#ifndef BOMBJACK_COMPLETE_OVERLAY_H
#define BOMBJACK_COMPLETE_OVERLAY_H

#include "screen.h"
#include <SDL2/SDL.h>

// ============================================================================
// BombJackCompleteOverlay
//
// Shown when the player clears all cookies in a Bomb Jack side room.
// Displays "LEVEL COMPLETE!" + cookie count ticker, then auto-dismisses
// and calls game->exitBombJackRoom() to restore the Downwell world.
//
// Phases:
//   SLIDE_IN  (0.3s) — panel slides up from bottom
//   TICKER    (1.0s) — cookie count ticks up
//   HOLD      (0.8s) — static display
//   SLIDE_OUT (0.3s) — panel slides back down
//   DONE             — calls exitSideRoom, pops itself
// ============================================================================
class BombJackCompleteOverlay : public Screen
{
public:
    // cookiesEarned: total cookies collected in this BJ level
    explicit BombJackCompleteOverlay(int cookiesEarned);

    // Full blocking overlay — game world pauses during fanfare
    bool blocksRender() const override { return true; }
    bool blocksUpdate() const override { return true; }

    void onEnter(Game* game) override;
    void update(float dt, Game* game) override;
    void render(SDL_Renderer* renderer, Game* game) override;

private:
    enum class Phase { SLIDE_IN, TICKER, HOLD, SLIDE_OUT, DONE };
    Phase  phase_      = Phase::SLIDE_IN;
    float  timer_      = 0.f;

    int    cookiesEarned_  = 0;  // actual total
    int    displayCookies_ = 0;  // animated counter
    float  tickerAccum_    = 0.f;

    // Panel Y offset for slide animation (0 = fully on screen)
    float  panelOffsetY_ = 0.f;

    static constexpr float SLIDE_IN_DUR  = 0.3f;
    static constexpr float TICKER_DUR    = 1.0f;
    static constexpr float HOLD_DUR      = 0.8f;
    static constexpr float SLIDE_OUT_DUR = 0.3f;

    static constexpr int PANEL_H = 220;
};

#endif // BOMBJACK_COMPLETE_OVERLAY_H
