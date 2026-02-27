#include "textManager.h"
#include <cstdio>

TextManager::TextManager()
{
}

TextManager::~TextManager()
{
    clean();
}

bool TextManager::init()
{
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return false;
    }
    return true;
}

void TextManager::clean()
{
    for (auto &pair : fonts)
    {
        if (pair.second)
        {
            TTF_CloseFont(pair.second);
        }
    }
    fonts.clear();
    TTF_Quit();
}

bool TextManager::loadFont(const std::string &name, const std::string &filepath, int size)
{
    TTF_Font *font = TTF_OpenFont(filepath.c_str(), size);

    if (!font)
    {
        printf("Failed to load font '%s'! TTF_Error: %s\n", filepath.c_str(), TTF_GetError());
        return false;
    }

    fonts[name] = font;
    printf("Font '%s' loaded successfully at size %d\n", name.c_str(), size);
    return true;
}

void TextManager::renderText(SDL_Renderer *renderer, const std::string &text,
                             const std::string &fontName, int x, int y,
                             SDL_Color color, bool centered)
{
    // Check if font exists
    if (fonts.find(fontName) == fonts.end())
    {
        printf("Font '%s' not loaded!\n", fontName.c_str());
        return;
    }

    TTF_Font *font = fonts[fontName];

    // Render text to surface
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!textSurface)
    {
        printf("Unable to render text! TTF_Error: %s\n", TTF_GetError());
        return;
    }

    // Create texture from surface
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture)
    {
        printf("Unable to create texture from text! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    // Get text dimensions
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    // Adjust position if centered
    int renderX = x;
    int renderY = y;
    if (centered)
    {
        renderX = x - textWidth / 2;
        renderY = y - textHeight / 2;
    }

    // Render texture
    SDL_Rect renderQuad = {renderX, renderY, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

    // Clean up
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void TextManager::getTextSize(const std::string &text, const std::string &fontName,
                              int *width, int *height)
{
    if (fonts.find(fontName) == fonts.end())
    {
        *width = 0;
        *height = 0;
        return;
    }

    TTF_Font *font = fonts[fontName];
    TTF_SizeText(font, text.c_str(), width, height);
}