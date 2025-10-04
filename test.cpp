#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    std::cout << "SDL2 is working!" << std::endl;
    std::cout << "SDL Version: " << SDL_MAJOR_VERSION << "." 
              << SDL_MINOR_VERSION << "." << SDL_PATCHLEVEL << std::endl;
    
    SDL_Quit();
    return 0;
}