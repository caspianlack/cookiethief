#include "screen.h"
#include <cassert>

// ============================================================================
// ScreenManager implementation
// ============================================================================

Screen* ScreenManager::top() const
{
    if (stack_.empty()) return nullptr;
    return stack_.back().get();
}

void ScreenManager::push(std::unique_ptr<Screen> screen, Game* game)
{
    screen->onEnter(game);
    stack_.push_back(std::move(screen));
}

void ScreenManager::pop(Game* game)
{
    if (stack_.empty()) return;
    stack_.back()->onExit(game);
    stack_.pop_back();
}

void ScreenManager::replace(std::unique_ptr<Screen> screen, Game* game)
{
    if (!stack_.empty())
    {
        stack_.back()->onExit(game);
        stack_.pop_back();
    }
    screen->onEnter(game);
    stack_.push_back(std::move(screen));
}

void ScreenManager::clear(Game* game)
{
    while (!stack_.empty())
    {
        stack_.back()->onExit(game);
        stack_.pop_back();
    }
}

// ----

void ScreenManager::handleEvent(SDL_Event& e, Game* game)
{
    if (stack_.empty()) return;
    // Only the top screen handles events
    stack_.back()->handleEvent(e, game);
}

void ScreenManager::update(float dt, Game* game)
{
    if (stack_.empty()) return;

    // Auto-pop screens that signal they are done
    if (stack_.back()->isDone())
    {
        pop(game);
        return;
    }

    // Find the lowest screen that should update
    int startIdx = (int)stack_.size() - 1;
    for (int i = startIdx; i > 0; i--)
    {
        if (stack_[i]->blocksUpdate())
        {
            startIdx = i;
            break;
        }
        startIdx = i - 1;
    }

    for (int i = startIdx; i < (int)stack_.size(); i++)
    {
        stack_[i]->update(dt, game);
    }
}

void ScreenManager::render(SDL_Renderer* renderer, Game* game)
{
    if (stack_.empty()) return;

    // Find the lowest screen that should be rendered
    int startIdx = (int)stack_.size() - 1;
    for (int i = startIdx; i > 0; i--)
    {
        if (stack_[i]->blocksRender())
        {
            startIdx = i;
            break;
        }
        startIdx = i - 1;
    }

    for (int i = startIdx; i < (int)stack_.size(); i++)
    {
        stack_[i]->render(renderer, game);
    }
}
