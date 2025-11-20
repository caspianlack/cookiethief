#ifndef TEXT_MANAGER_H
#define TEXT_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <map>

class TextManager
{
private:
    std::map<std::string, TTF_Font *> fonts;

public:
    TextManager();
    ~TextManager();

    bool init();
    void clean();

    // Load font prefab to be reused
    bool loadFont(const std::string &name, const std::string &filepath, int size);

    // render text
    void renderText(SDL_Renderer *renderer, const std::string &text,
                    const std::string &fontName, int x, int y,
                    SDL_Color color, bool centered = false);

    // get text dimensions (for positioning)
    void getTextSize(const std::string &text, const std::string &fontName,
                     int *width, int *height);
};

#endif