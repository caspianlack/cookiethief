#!/bin/bash
# Improved build script using pkg-config for automatic SDL2 detection

# Get SDL2 flags automatically
SDL2_CFLAGS=$(pkg-config --cflags sdl2 SDL2_image SDL2_ttf)
SDL2_LIBS=$(pkg-config --libs sdl2 SDL2_image SDL2_ttf)

# Compile
g++ main.cpp game.cpp player.cpp enemy.cpp cookie.cpp projectile.cpp \
textmanager.cpp levelmanager.cpp downwellgenerator.cpp gamerun.cpp texturemanager.cpp \
$SDL2_CFLAGS \
$SDL2_LIBS \
-o CookieThief.exe

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./CookieThief.exe"
else
    echo "Build failed!"
    exit 1
fi
