#ifndef GAMEOVER_OVERLAY_H
#define GAMEOVER_OVERLAY_H

#include "screen.h"
#include <SDL2/SDL.h>

// ============================================================================
// GameOverOverlay
//
// Rendered on top of the game world (blocksRender=false).
// Fades in a black veil, shows "CAUGHT!" stats, waits for R to restart.
// On dismissal it pops itself and calls game->loadLobby().
// ============================================================================
class GameOverOverlay : public Screen
{
public:
    GameOverOverlay() = default;

    bool blocksRender() const override { return false; } // world shows through
    bool blocksUpdate() const override { return false; } // world keeps ticking (death anim, gravity)

    void onEnter(Game* game) override;
    void handleEvent(SDL_Event& e, Game* game) override;
    void update(float dt, Game* game) override;
    void render(SDL_Renderer* renderer, Game* game) override;

private:
    float fadeAlpha_  = 0.f;   // 0-255 black overlay alpha
    bool  accepted_   = false;

    static constexpr float FADE_SPEED = 120.f; // alpha units per second
};

#endif // GAMEOVER_OVERLAY_H
