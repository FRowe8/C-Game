# 🎮 Quick Start Guide for Windows

## Super Easy Method (Recommended)

### Step 0: Check What You Need (Optional)
Run this to see what's missing:
```cmd
check_setup.bat
```

### Step 1: Install CMake
**Download**: https://cmake.org/download/
- Choose **"Windows x64 Installer"** (cmake-3.x.x-windows-x86_64.msi)
- Run the installer
- **⚠️ CRITICAL**: Select **"Add CMake to system PATH"** during installation

**Detailed instructions**: See [INSTALL_CMAKE.md](INSTALL_CMAKE.md)

### Step 2: Install Visual Studio Build Tools
**Download**: https://visualstudio.microsoft.com/downloads/
- Scroll to "Tools for Visual Studio"
- Download **"Build Tools for Visual Studio 2022"** (FREE)
- During installation, select **"Desktop development with C++"**

### Step 3: Verify Installation (Recommended)
Close and reopen Command Prompt, then test:
```cmd
cmake --version
```
Should show: `cmake version 3.x.x`

If it says "not recognized", see [INSTALL_CMAKE.md](INSTALL_CMAKE.md) for troubleshooting.

### Step 4: Build the Game
1. Open Command Prompt (cmd)
2. Navigate to the game folder:
   ```cmd
   cd path\to\C-Game
   ```
3. Run the build script:
   ```cmd
   build.bat
   ```
4. The script will automatically offer to download SDL2 for you!
5. Answer "Y" when prompted
6. After SDL2 installs, open a NEW terminal and run `build.bat` again

### Step 5: Play!
```cmd
cd build\Release
QuantumIdle.exe
```
Or just double-click: `build\Release\QuantumIdle.exe`

---

## Alternative: Manual SDL2 Installation

If the automatic download doesn't work:

### Download SDL2
1. Go to: https://github.com/libsdl-org/SDL/releases/latest
2. Download: `SDL2-devel-2.x.x-VC.zip`
3. Extract the ZIP file
4. Rename the extracted folder to `SDL2`
5. Move it to `C:\SDL2`

Your folder structure should look like:
```
C:\SDL2\
  ├── include\
  │   └── SDL.h (and other headers)
  └── lib\
      ├── x64\
      │   ├── SDL2.lib
      │   └── SDL2.dll
      └── x86\
```

### Build
```cmd
build.bat
```

---

## Troubleshooting

### "cmake is not recognized"
- CMake not in PATH. Reinstall CMake and check "Add to PATH" option
- Or add manually: Settings → System → Environment Variables

### "MSVC not found" or "No C++ compiler"
- Install Visual Studio Build Tools (see Step 2 above)
- Make sure "Desktop development with C++" is selected

### "SDL2 not found"
- Run `setup_sdl2_windows.bat` manually
- Or follow manual installation steps above
- Make sure SDL2 is at `C:\SDL2`

### "SDL2.dll not found" when running game
- The build.bat script should copy it automatically
- If not, manually copy `C:\SDL2\lib\x64\SDL2.dll` to `build\Release\`

### "OpenGL errors" when running
- Update your graphics drivers
- Try running as administrator

### Still having issues?
See detailed instructions in: `WINDOWS_BUILD.md`

---

## What You Need (Checklist)

- ✅ CMake (version 3.15+)
- ✅ Visual Studio Build Tools OR Visual Studio 2019/2022
- ✅ SDL2 (automatically downloaded by build.bat)
- ✅ Windows 10 or later (should work on Windows 7+ though)

---

## Too Complicated? Try These Alternatives:

### Option A: Use MSYS2 (Unix-like environment)
1. Install MSYS2: https://www.msys2.org/
2. Open "MSYS2 MinGW 64-bit" terminal
3. Install tools:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 make
   ```
4. Build:
   ```bash
   mkdir build && cd build
   cmake .. -G "MinGW Makefiles"
   make
   ./QuantumIdle.exe
   ```

### Option B: Use WSL (Windows Subsystem for Linux)
1. Install WSL: https://docs.microsoft.com/en-us/windows/wsl/install
2. Open Ubuntu terminal
3. Follow the Linux instructions in README.md

---

## Quick Commands Reference

```cmd
REM First time setup (one-time only)
setup_sdl2_windows.bat

REM Build the game
build.bat

REM Run the game
cd build\Release
QuantumIdle.exe

REM Clean build (if needed)
rmdir /s build
build.bat
```

---

## Expected Build Time

- First build: 2-5 minutes
- Rebuild after changes: 10-30 seconds

---

## Need Help?

1. Check WINDOWS_BUILD.md for detailed troubleshooting
2. Make sure all prerequisites are installed
3. Try running build.bat as Administrator
4. Check that antivirus isn't blocking downloads/builds

Happy quantum computing! 🔬✨
