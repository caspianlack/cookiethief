#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <map>
#include <vector>

class TextureManager
{
private:
    std::map<std::string, SDL_Texture *> textures;
    SDL_Renderer *renderer;

public:
    TextureManager();
    ~TextureManager();

    bool init(SDL_Renderer *ren);
    void clean();

    // Load a texture from file and store it with a name
    bool loadTexture(const std::string &name, const std::string &filepath);

    // Create a texture from a "Pixel Map" (Array of strings)

    // Create a texture from a "Pixel Map" (Array of strings)
    // charMap: vector of strings representing rows. chars map to colors.
    // palette: map defining which char corresponds to which RGBA color.
    bool createTextureFromMap(const std::string &name, 
                             const std::vector<std::string> &pixelMap, 
                             const std::map<char, SDL_Color> &palette,
                             int scale = 1);

    // Get a texture by name
    SDL_Texture *getTexture(const std::string &name);

    // Render a texture at position with optional scaling
    void renderTexture(const std::string &name, int x, int y,
                       int width = -1, int height = -1,
                       SDL_RendererFlip flip = SDL_FLIP_NONE);

    // Render a specific frame from a spritesheet
    void renderFrame(const std::string &name, int x, int y,
                     int frameX, int frameY, int frameWidth, int frameHeight,
                     int destWidth = -1, int destHeight = -1,
                     SDL_RendererFlip flip = SDL_FLIP_NONE);

    // Render with rotation
    void renderTextureEx(const std::string &name, int x, int y,
                         int width, int height, double angle,
                         SDL_RendererFlip flip = SDL_FLIP_NONE);
};

#endif
