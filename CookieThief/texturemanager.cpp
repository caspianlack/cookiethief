#include "texturemanager.h"

// ============================================================================
// IMPLEMENTATION - texturemanager.cpp
// ============================================================================

// #include "texturemanager.h"
#include <cstdio>

TextureManager::TextureManager()
{
    renderer = nullptr;
}

TextureManager::~TextureManager()
{
    clean();
}

bool TextureManager::init(SDL_Renderer *ren)
{
    renderer = ren;

    // Initialize SDL_image with PNG support
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        return false;
    }

    printf("TextureManager initialized successfully\n");
    return true;
}

void TextureManager::clean()
{
    // Free all loaded textures
    for (auto const &pair : textures)
    {
        SDL_DestroyTexture(pair.second);
    }
    textures.clear();

    IMG_Quit();
}

bool TextureManager::loadTexture(const std::string &name, const std::string &filepath)
{
    // Load image as surface
    SDL_Surface *tempSurface = IMG_Load(filepath.c_str());

    if (!tempSurface)
    {
        printf("Failed to load image '%s'! IMG_Error: %s\n",
               filepath.c_str(), IMG_GetError());
        return false;
    }

    // Create texture from surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    if (!texture)
    {
        printf("Failed to create texture from '%s'! SDL_Error: %s\n",
               filepath.c_str(), SDL_GetError());
        return false;
    }

    // Store texture
    textures[name] = texture;
    printf("Loaded texture '%s' from '%s'\n", name.c_str(), filepath.c_str());

    return true;
}

SDL_Texture *TextureManager::getTexture(const std::string &name)
{
    if (textures.find(name) != textures.end())
    {
        return textures[name];
    }

    printf("Texture '%s' not found!\n", name.c_str());
    return nullptr;
}

void TextureManager::renderTexture(const std::string &name, int x, int y,
                                   int width, int height, SDL_RendererFlip flip)
{
    SDL_Texture *texture = getTexture(name);
    if (!texture)
        return;

    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;

    // If width/height not specified, use texture dimensions
    if (width == -1 || height == -1)
    {
        SDL_QueryTexture(texture, NULL, NULL, &destRect.w, &destRect.h);
    }
    else
    {
        destRect.w = width;
        destRect.h = height;
    }

    SDL_RenderCopyEx(renderer, texture, NULL, &destRect, 0, NULL, flip);
}

void TextureManager::renderFrame(const std::string &name, int x, int y,
                                 int frameX, int frameY, int frameWidth, int frameHeight,
                                 int destWidth, int destHeight, SDL_RendererFlip flip)
{
    SDL_Texture *texture = getTexture(name);
    if (!texture)
        return;

    SDL_Rect srcRect = {frameX, frameY, frameWidth, frameHeight};
    SDL_Rect destRect = {x, y,
                         destWidth == -1 ? frameWidth : destWidth,
                         destHeight == -1 ? frameHeight : destHeight};

    SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0, NULL, flip);
}

void TextureManager::renderTextureEx(const std::string &name, int x, int y,
                                     int width, int height, double angle,
                                     SDL_RendererFlip flip)
{
    SDL_Texture *texture = getTexture(name);
    if (!texture)
        return;

    SDL_Rect destRect = {x, y, width, height};
    SDL_RenderCopyEx(renderer, texture, NULL, &destRect, angle, NULL, flip);
}

bool TextureManager::createTextureFromMap(const std::string &name, 
                             const std::vector<std::string> &pixelMap, 
                             const std::map<char, SDL_Color> &palette,
                             int scale)
{
    if (pixelMap.empty()) return false;
    
    int rows = pixelMap.size();
    int cols = pixelMap[0].length();
    
    // Create a surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, cols * scale, rows * scale, 32, 
                                        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    
    if (!surface) {
        printf("Failed to create surface for %s! SDL_Error: %s\n", name.c_str(), SDL_GetError());
        return false;
    }

    // Lock surface for manipulation
    SDL_LockSurface(surface);
    Uint32* pixels = (Uint32*)surface->pixels;
    
    // Fill with transparent initially
    SDL_memset(pixels, 0, surface->h * surface->pitch);

    // Draw pixels
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            char pixelChar = pixelMap[y][x];
            if (pixelChar != ' ' && palette.find(pixelChar) != palette.end()) {
                SDL_Color color = palette.at(pixelChar);
                Uint32 mappedColor = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
                
                // Draw scaled pixel
                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        int destX = x * scale + sx;
                        int destY = y * scale + sy;
                        if (destX < surface->w && destY < surface->h) {
                            pixels[destY * (surface->pitch / 4) + destX] = mappedColor;
                        }
                    }
                }
            }
        }
    }

    SDL_UnlockSurface(surface);

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        printf("Failed to create texture from surface for %s! SDL_Error: %s\n", name.c_str(), SDL_GetError());
        return false;
    }

    textures[name] = texture;
    return true;
}