#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>
#include <vector>
#include <memory>

// Forward declare Game to avoid circular includes.
// Screens receive a Game* for callbacks and context.
class Game;

// ============================================================================
// Screen — base class for all game screens and overlays
// ============================================================================
class Screen
{
public:
    virtual ~Screen() = default;

    // Lifecycle
    virtual void onEnter(Game* game) {}
    virtual void onExit(Game* game) {}

    // If false, the screen beneath this one in the stack is also updated.
    // Full screens should return true; overlays that pause gameplay return true too.
    virtual bool blocksUpdate() const { return true; }

    // If false, the screen beneath this one in the stack is also rendered first.
    // Overlay screens (game-over fade, heart-loss) return false so the world shows through.
    virtual bool blocksRender() const { return true; }

    // Return true if the screen wants to be popped (self-removing)
    virtual bool isDone() const { return done_; }

    virtual void handleEvent(SDL_Event& e, Game* game) {}
    virtual void update(float dt, Game* game) {}
    virtual void render(SDL_Renderer* renderer, Game* game) {}

protected:
    bool done_ = false;
};

// ============================================================================
// ScreenManager — stack-based screen manager
//
// Usage:
//   screenManager.push(std::make_unique<MyScreen>(), game);
//   screenManager.pop(game);      // removes top screen
//   screenManager.replace(...);   // pop + push atomically
// ============================================================================
class ScreenManager
{
public:
    ScreenManager() = default;
    ~ScreenManager() = default;

    // Push a new screen on top of the stack
    void push(std::unique_ptr<Screen> screen, Game* game);

    // Pop the top screen
    void pop(Game* game);

    // Replace the top screen (pop + push without intermediate state)
    void replace(std::unique_ptr<Screen> screen, Game* game);

    // Remove all screens
    void clear(Game* game);

    // True if there are no screens
    bool empty() const { return stack_.empty(); }

    // Number of screens on the stack
    int size() const { return (int)stack_.size(); }

    // Returns a raw pointer to the top screen (nullptr if empty)
    Screen* top() const;

    // --- Main loop integration ---

    void handleEvent(SDL_Event& e, Game* game);

    // dt = delta time in seconds
    void update(float dt, Game* game);

    // Renders all screens that are visible (respects blocksRender)
    void render(SDL_Renderer* renderer, Game* game);

private:
    std::vector<std::unique_ptr<Screen>> stack_;

    // Deferred actions to avoid mutating stack during iteration
    bool pendingPop_  = false;
    std::unique_ptr<Screen> pendingPush_;
    bool pendingReplace_ = false;

    void processPending(Game* game);
};

#endif // SCREEN_H
