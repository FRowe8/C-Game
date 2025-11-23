@echo off
REM Quick setup script for installing SDL2 via vcpkg (CLion integration)

echo ========================================
echo SDL2 Setup via vcpkg for CLion
echo ========================================
echo.

REM Detect vcpkg location
set VCPKG_PATH=C:\Users\Computer\.vcpkg-clion\vcpkg

if not exist "%VCPKG_PATH%\vcpkg.exe" (
    echo ERROR: vcpkg not found at %VCPKG_PATH%
    echo.
    echo This script is for CLion with vcpkg integration.
    echo If you don't have vcpkg set up in CLion, use setup_sdl2_windows.bat instead.
    pause
    exit /b 1
)

echo Found vcpkg at: %VCPKG_PATH%
echo.
echo This will install SDL2 using vcpkg (recommended method for CLion).
echo.
echo Press Ctrl+C to cancel, or
pause

echo.
echo Installing SDL2 via vcpkg...
echo This may take several minutes on first install (vcpkg compiles from source).
echo.

cd /d "%VCPKG_PATH%"
vcpkg install sdl2:x64-windows

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: vcpkg install failed!
    echo Try running this manually:
    echo   cd %VCPKG_PATH%
    echo   vcpkg install sdl2:x64-windows
    pause
    exit /b 1
)

echo.
echo ========================================
echo SDL2 Installation Complete!
echo ========================================
echo.
echo vcpkg has installed SDL2 for x64-windows.
echo.
echo Next steps:
echo 1. In CLion: Tools ^> CMake ^> Reset Cache and Reload Project
echo 2. Build ^> Rebuild Project
echo.
echo The build should now succeed!
echo.
pause
