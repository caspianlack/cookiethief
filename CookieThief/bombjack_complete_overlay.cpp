#include "bombjack_complete_overlay.h"
#include "game.h"
#include "constants.h"
#include "textmanager.h"
#include <cmath>
#include <cstdio>
#include <SDL2/SDL.h>

// ============================================================================
// BombJackCompleteOverlay implementation
// ============================================================================

BombJackCompleteOverlay::BombJackCompleteOverlay(int cookiesEarned)
    : cookiesEarned_(cookiesEarned)
    , displayCookies_(0)
{
    // Panel starts fully below the screen
    panelOffsetY_ = (float)PANEL_H;
}

void BombJackCompleteOverlay::onEnter(Game* /*game*/)
{
    timer_         = 0.f;
    phase_         = Phase::SLIDE_IN;
    panelOffsetY_  = (float)PANEL_H;
    displayCookies_ = 0;
    tickerAccum_   = 0.f;
}

// ============================================================================
// Update
// ============================================================================
void BombJackCompleteOverlay::update(float dt, Game* game)
{
    timer_ += dt;

    switch (phase_)
    {
        case Phase::SLIDE_IN:
        {
            float t = timer_ / SLIDE_IN_DUR;
            if (t > 1.f) t = 1.f;
            // Ease-out: decelerate coming in
            panelOffsetY_ = (float)PANEL_H * (1.f - t * t);
            if (timer_ >= SLIDE_IN_DUR)
            {
                panelOffsetY_ = 0.f;
                phase_ = Phase::TICKER;
                timer_ = 0.f;
            }
            break;
        }

        case Phase::TICKER:
        {
            // Tick the cookie counter up from 0 to cookiesEarned_
            tickerAccum_ += dt;
            if (cookiesEarned_ > 0)
            {
                float tickInterval = TICKER_DUR / (float)cookiesEarned_;
                while (tickerAccum_ >= tickInterval && displayCookies_ < cookiesEarned_)
                {
                    displayCookies_++;
                    tickerAccum_ -= tickInterval;
                }
            }
            if (timer_ >= TICKER_DUR)
            {
                displayCookies_ = cookiesEarned_; // snap to final
                phase_ = Phase::HOLD;
                timer_ = 0.f;
            }
            break;
        }

        case Phase::HOLD:
            if (timer_ >= HOLD_DUR)
            {
                phase_ = Phase::SLIDE_OUT;
                timer_ = 0.f;
            }
            break;

        case Phase::SLIDE_OUT:
        {
            float t = timer_ / SLIDE_OUT_DUR;
            if (t > 1.f) t = 1.f;
            // Ease-in: accelerate going out
            panelOffsetY_ = (float)PANEL_H * (t * t);
            if (timer_ >= SLIDE_OUT_DUR)
            {
                phase_ = Phase::DONE;
                done_  = true;
                // Exit the side room once the animation finishes
                game->exitBombJackRoom();
            }
            break;
        }

        case Phase::DONE:
            done_ = true;
            break;
    }
}

// ============================================================================
// Render
// ============================================================================
void BombJackCompleteOverlay::render(SDL_Renderer* renderer, Game* game)
{
    TextManager* tm = game->getTextManager();

    // --- Dim the background slightly ---
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_Rect backdrop = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &backdrop);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // --- Panel (slides up from bottom) ---
    int panelY = SCREEN_HEIGHT / 2 - PANEL_H / 2 + (int)panelOffsetY_;
    int panelX = SCREEN_WIDTH  / 2 - 280;
    int panelW = 560;

    // Panel shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect shadow = {panelX + 6, panelY + 6, panelW, PANEL_H};
    SDL_RenderFillRect(renderer, &shadow);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Panel background — dark chocolate tone
    SDL_SetRenderDrawColor(renderer, 38, 20, 8, 255);
    SDL_Rect panel = {panelX, panelY, panelW, PANEL_H};
    SDL_RenderFillRect(renderer, &panel);

    // Panel border — gold
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &panel);
    // Double border (inner)
    SDL_Rect innerBorder = {panelX + 4, panelY + 4, panelW - 8, PANEL_H - 8};
    SDL_RenderDrawRect(renderer, &innerBorder);

    // --- "LEVEL COMPLETE!" ---
    SDL_Color gold   = {255, 215,   0, 255};
    SDL_Color green  = {100, 255, 100, 255};
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color hint   = {180, 180, 180, 255};

    tm->renderText(renderer, "LEVEL COMPLETE!", "title",
                   SCREEN_WIDTH / 2, panelY + 32, gold, true);

    // Divider line
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180);
    SDL_RenderDrawLine(renderer,
                       panelX + 20,  panelY + 75,
                       panelX + panelW - 20, panelY + 75);

    // --- Cookie count ticker ---
    char cookieText[48];
    sprintf(cookieText, "COOKIES  +%d", displayCookies_);
    SDL_Color counterColor = (displayCookies_ == cookiesEarned_) ? green : white;
    tm->renderText(renderer, cookieText, "normal",
                   SCREEN_WIDTH / 2, panelY + 105, counterColor, true);

    // Small row of cookie squares as visual pip indicators
    const int PIP_SIZE    = 14;
    const int PIP_GAP     = 6;
    const int maxPips     = (cookiesEarned_ > 20) ? 20 : cookiesEarned_;
    if (maxPips > 0)
    {
        int totalW  = maxPips * (PIP_SIZE + PIP_GAP) - PIP_GAP;
        int pipX    = SCREEN_WIDTH / 2 - totalW / 2;
        int pipY    = panelY + 140;
        int filled  = (cookiesEarned_ > 0)
                      ? (displayCookies_ * maxPips / cookiesEarned_)
                      : 0;

        for (int i = 0; i < maxPips; i++)
        {
            if (i < filled)
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // gold filled
            else
                SDL_SetRenderDrawColor(renderer, 80, 50, 20, 255);   // dark empty

            SDL_Rect pip = {pipX + i * (PIP_SIZE + PIP_GAP), pipY, PIP_SIZE, PIP_SIZE};
            SDL_RenderFillRect(renderer, &pip);

            SDL_SetRenderDrawColor(renderer, 200, 160, 50, 255);
            SDL_RenderDrawRect(renderer, &pip);
        }
    }

    // --- Flavour text ---
    if (phase_ == Phase::HOLD || phase_ == Phase::SLIDE_OUT)
    {
        tm->renderText(renderer, "Head back to the heist...", "small",
                       SCREEN_WIDTH / 2, panelY + PANEL_H - 30, hint, true);
    }
}
