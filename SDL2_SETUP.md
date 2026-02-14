# SDL2 Setup Guide for CookieThief

## Problem
The compiler can't find SDL2 headers because you're using UNIX-style paths (`/mingw64/include`) in a Windows command prompt.

## Solution

### Step 1: Open MSYS2 MinGW 64-bit Terminal
1. Press Windows key
2. Search for "MSYS2 MinGW 64-bit" 
3. Open it (NOT "MSYS2 MSYS" or regular cmd)

### Step 2: Install SDL2 Libraries
Run these commands in the MSYS2 MinGW 64-bit terminal:

```bash
pacman -S mingw-w64-x86_64-SDL2
pacman -S mingw-w64-x86_64-SDL2_image
pacman -S mingw-w64-x86_64-SDL2_ttf
```

If prompted, press Y to confirm installation.

### Step 3: Navigate to Your Project
```bash
cd /c/Users/caspi/Desktop/Projects/cookiethief
```

### Step 4: Build Using the Updated Script
Run one of these:

```bash
# Option 1: Use the shell script
bash build.sh

# Option 2: Use the improved build script with pkg-config
bash build_improved.sh
```

## Why This Works
- MSYS2 MinGW terminal understands `/mingw64/include` paths
- The terminal has the correct environment variables set
- pkg-config automatically finds the right include and library paths

## Verification
After installation, verify SDL2 is installed:
```bash
pkg-config --cflags --libs sdl2
```

You should see output like:
```
-IC:/msys64/mingw64/include/SDL2 -Dmain=SDL_main -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2
```
