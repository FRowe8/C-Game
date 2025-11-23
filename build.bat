@echo off
REM Build script for Windows

echo Quantum Idle - Build Script
echo ============================
echo.

REM Check if SDL2 exists
if not exist "C:\SDL2" (
    if not defined SDL2_DIR (
        echo WARNING: SDL2 not found at C:\SDL2
        echo.
        echo Would you like to automatically download and install SDL2? (Y/N)
        set /p INSTALL_SDL2=
        if /i "%INSTALL_SDL2%"=="Y" (
            call setup_sdl2_windows.bat
            echo.
            echo SDL2 installed! Please run build.bat again in a NEW terminal.
            pause
            exit /b 0
        ) else (
            echo.
            echo Please install SDL2 manually. See WINDOWS_BUILD.md for instructions.
            pause
            exit /b 1
        )
    )
)

echo SDL2 found!
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    echo See WINDOWS_BUILD.md for troubleshooting.
    cd ..
    pause
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

REM Copy SDL2.dll to output directory
echo.
echo Copying SDL2.dll...
if exist "C:\SDL2\lib\x64\SDL2.dll" (
    copy "C:\SDL2\lib\x64\SDL2.dll" "Release\" >nul
    echo SDL2.dll copied successfully
) else if exist "C:\SDL2\lib\x86\SDL2.dll" (
    copy "C:\SDL2\lib\x86\SDL2.dll" "Release\" >nul
    echo SDL2.dll copied successfully
) else if defined SDL2_DIR (
    if exist "%SDL2_DIR%\lib\x64\SDL2.dll" (
        copy "%SDL2_DIR%\lib\x64\SDL2.dll" "Release\" >nul
        echo SDL2.dll copied successfully
    )
)

cd ..

echo.
echo ========================================
echo Build complete!
echo ========================================
echo.
echo Run the game with:
echo   cd build\Release
echo   QuantumIdle.exe
echo.
echo Or double-click: build\Release\QuantumIdle.exe
echo.
pause
