# Building with MinGW / CLion

If you're using **CLion** or **MinGW** directly (not Visual Studio), you need the **MinGW version** of SDL2, not the MSVC version!

## The Problem

The automatic `setup_sdl2_windows.bat` script downloads the **MSVC** version of SDL2, which has `.lib` files that MinGW cannot link against. MinGW needs `.a` files.

## Solution: Download MinGW SDL2

### Step 1: Download SDL2 for MinGW

Go to: https://github.com/libsdl-org/SDL/releases/latest

Download: **`SDL2-devel-2.x.x-mingw.tar.gz`** (NOT the VC version!)

### Step 2: Extract

Extract the `.tar.gz` file:
- Use 7-Zip, WinRAR, or another tool that can handle `.tar.gz`
- Or use Windows 10/11 built-in tar: `tar -xzf SDL2-devel-2.30.0-mingw.tar.gz`

### Step 3: Copy to C:\SDL2

After extracting, you'll have a folder like `SDL2-2.30.0`.

Inside, you'll find:
```
SDL2-2.30.0/
  ├── x86_64-w64-mingw32/
  │   ├── include/
  │   ├── lib/
  │   └── bin/
  └── i686-w64-mingw32/
```

Copy the **x86_64-w64-mingw32** folder contents to `C:\SDL2`:

```
C:\SDL2\
  ├── include\
  │   └── SDL2\
  │       └── SDL.h (and other headers)
  ├── lib\
  │   ├── libSDL2.a
  │   ├── libSDL2.dll.a
  │   ├── libSDL2main.a
  │   └── libSDL2_test.a
  └── bin\
      └── SDL2.dll
```

**Important**: The directory structure should have `include/SDL2/` NOT just `include/`!

### Step 4: Update CMakeLists.txt (if needed)

The CMake file should find SDL2 automatically. If not, you may need to adjust paths.

### Step 5: Build

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

Or in CLion, just click Build.

---

## Alternative: Quick Manual Download

If the above is too complex, here's a direct approach:

1. Download: https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-mingw.tar.gz
2. Extract with: `tar -xzf SDL2-devel-2.30.0-mingw.tar.gz`
3. Copy `SDL2-2.30.0/x86_64-w64-mingw32/*` to `C:/SDL2/`
4. Make sure you have:
   - `C:/SDL2/include/SDL2/SDL.h`
   - `C:/SDL2/lib/libSDL2.a`
   - `C:/SDL2/bin/SDL2.dll`

---

## Verifying Your Setup

Check these files exist:
```cmd
dir C:\SDL2\include\SDL2\SDL.h
dir C:\SDL2\lib\libSDL2.a
dir C:\SDL2\bin\SDL2.dll
```

If you see "File Not Found", the structure is wrong.

---

## Why This Happens

- **MSVC** (Visual Studio) uses `.lib` format
- **MinGW** (GCC for Windows) uses `.a` format
- They are binary incompatible
- You must use matching compiler and library format

---

## Still Having Issues?

If you keep getting linking errors, check:
1. Make sure you downloaded the **mingw** version (not VC)
2. Make sure the directory structure is correct
3. Delete your `build/` folder and rebuild from scratch
4. In CLion: File → Invalidate Caches and Restart

---

## Alternative: Use MSVC Instead

If this is too complicated, you can use Visual Studio instead:
1. Install Visual Studio 2022 (Community Edition - Free)
2. Use the VC version of SDL2 (which the auto-installer downloads)
3. Open the project in Visual Studio instead of CLion
4. Build with MSVC

This would work with the existing `C:\SDL2` you already have!
