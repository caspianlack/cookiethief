#include "gameover_overlay.h"
#include "game.h"
#include "constants.h"
#include <SDL2/SDL.h>

// ============================================================================
// GameOverOverlay implementation
// ============================================================================

void GameOverOverlay::onEnter(Game* /*game*/)
{
    fadeAlpha_ = 0.f;
    accepted_  = false;
}

void GameOverOverlay::handleEvent(SDL_Event& e, Game* game)
{
    if (e.type == SDL_KEYDOWN && !accepted_)
    {
        if (e.key.keysym.sym == SDLK_r && fadeAlpha_ >= 100.f)
        {
            accepted_ = true;
            done_ = true;
            game->loadLobby();
        }
    }
}

void GameOverOverlay::update(float dt, Game* /*game*/)
{
    if (fadeAlpha_ < 255.f)
    {
        fadeAlpha_ += FADE_SPEED * dt;
        if (fadeAlpha_ > 255.f) fadeAlpha_ = 255.f;
    }
}

void GameOverOverlay::render(SDL_Renderer* renderer, Game* game)
{
    Uint8 alpha = (Uint8)fadeAlpha_;

    // --- Black veil ---
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)(alpha * 0.78f));
    SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Only draw text once the veil is mostly opaque
    if (alpha < 100) return;

    Uint8 textAlpha = alpha > 200 ? 255 : (Uint8)((alpha / 200.f) * 255.f);

    SDL_Color red   = {255,  50,  50, textAlpha};
    SDL_Color white = {255, 255, 255, textAlpha};
    SDL_Color hint  = {200, 200, 200, textAlpha};

    TextManager*  tm  = game->getTextManager();
    GameRun*      run = game->getCurrentRun();

    tm->renderText(renderer, "CAUGHT!", "title", SCREEN_WIDTH / 2, 80, red, true);

    int y = 180;
    char text[64];

    sprintf(text, "Made it to Floor: %d", run->getCurrentFloor());
    tm->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 44;

    sprintf(text, "Cookies Stolen: %d", run->getStats().cookiesThisRun);
    tm->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);
    y += 44;

    sprintf(text, "Distance Fell: %d", run->getStats().distanceFell);
    tm->renderText(renderer, text, "normal", SCREEN_WIDTH / 2, y, white, true);

    tm->renderText(renderer, "Press R to return to lobby", "small",
                   SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60, hint, true);
}
