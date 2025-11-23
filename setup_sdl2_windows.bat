@echo off
REM Automatic SDL2 setup for Windows
REM This script downloads and extracts SDL2 automatically

echo ========================================
echo Quantum Idle - SDL2 Setup for Windows
echo ========================================
echo.

REM Check if SDL2 already exists
if exist "C:\SDL2" (
    echo SDL2 already exists at C:\SDL2
    echo If you want to re-download, delete C:\SDL2 first
    pause
    exit /b 0
)

echo This script will:
echo 1. Download SDL2 development libraries
echo 2. Extract to C:\SDL2
echo 3. Set SDL2_DIR environment variable
echo.
echo Press Ctrl+C to cancel, or
pause

REM Create temp directory
mkdir temp_sdl2 2>nul
cd temp_sdl2

echo.
echo Downloading SDL2 (MinGW version)...
echo.

REM Download SDL2 MinGW version using PowerShell
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-mingw.tar.gz' -OutFile 'SDL2-mingw.tar.gz'}"

if not exist SDL2.zip (
    echo ERROR: Failed to download SDL2
    echo Please download manually from: https://github.com/libsdl-org/SDL/releases
    echo See WINDOWS_BUILD.md for instructions
    pause
    exit /b 1
)

echo.
echo Extracting SDL2...
echo.

REM Extract using PowerShell
powershell -Command "& {Expand-Archive -Path 'SDL2.zip' -DestinationPath '.' -Force}"

REM Move to C:\SDL2
echo Moving to C:\SDL2...
move SDL2-2.30.0 C:\SDL2

REM Set environment variable
echo.
echo Setting SDL2_DIR environment variable...
setx SDL2_DIR "C:\SDL2"

REM Cleanup
cd ..
rmdir /s /q temp_sdl2

echo.
echo ========================================
echo SDL2 Setup Complete!
echo ========================================
echo.
echo SDL2 installed to: C:\SDL2
echo Environment variable SDL2_DIR set
echo.
echo IMPORTANT: Close and reopen your terminal for changes to take effect!
echo.
echo Next steps:
echo 1. Close this window
echo 2. Open a NEW terminal
echo 3. Run: build.bat
echo.
pause
