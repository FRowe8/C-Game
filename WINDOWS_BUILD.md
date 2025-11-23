# Windows Build Guide (No vcpkg required!)

## Option 1: Download SDL2 Pre-built Binaries (Recommended)

### Step 1: Download SDL2
1. Go to https://github.com/libsdl-org/SDL/releases/latest
2. Download `SDL2-devel-2.x.x-VC.zip` (Visual Studio version)
3. Extract to `C:\SDL2` (or anywhere you like)

### Step 2: Set Environment Variable
```cmd
setx SDL2_DIR "C:\SDL2"
```
(Restart your terminal after this)

### Step 3: Build
```cmd
mkdir build
cd build
cmake .. -DSDL2_DIR="C:\SDL2"
cmake --build . --config Release
```

### Step 4: Copy SDL2.dll
After building, copy `C:\SDL2\lib\x64\SDL2.dll` to your `build\Release\` folder

### Step 5: Run
```cmd
cd Release
QuantumIdle.exe
```

---

## Option 2: MSYS2 (Alternative - Unix-like on Windows)

### Step 1: Install MSYS2
Download from https://www.msys2.org/ and install

### Step 2: Install packages
Open "MSYS2 MinGW 64-bit" terminal:
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 make
```

### Step 3: Build
```bash
cd /c/Users/YourName/path/to/C-Game
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./QuantumIdle.exe
```

---

## Option 3: Visual Studio (No CMake)

If you have Visual Studio installed, I can create a .sln project file instead!

---

## Troubleshooting

### "SDL2.dll not found"
- Copy SDL2.dll from SDL2/lib/x64/ to the same folder as QuantumIdle.exe

### "Cannot find SDL2"
- Make sure SDL2_DIR environment variable is set
- Or pass it directly: `cmake .. -DSDL2_DIR="C:\SDL2"`

### "OpenGL errors"
- Make sure your graphics drivers are up to date
- Try running as administrator

---

## Quick Test (No Build)
Want to test if SDL2 works first? I can create a minimal test program!
