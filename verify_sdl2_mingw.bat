@echo off
REM Verify SDL2 MinGW installation

echo ========================================
echo SDL2 MinGW Installation Verification
echo ========================================
echo.

echo Checking for required files...
echo.

set ERRORS=0

REM Check include directory
if exist "C:\SDL2\include\SDL2\SDL.h" (
    echo [OK] SDL2 headers found at C:\SDL2\include\SDL2\SDL.h
) else (
    echo [ERROR] SDL2 headers NOT found at C:\SDL2\include\SDL2\SDL.h
    set ERRORS=1
)

REM Check MinGW library files (.a files, NOT .lib)
if exist "C:\SDL2\lib\libSDL2.dll.a" (
    echo [OK] libSDL2.dll.a found
) else (
    echo [ERROR] libSDL2.dll.a NOT found - you may have MSVC version!
    set ERRORS=1
)

if exist "C:\SDL2\lib\libSDL2main.a" (
    echo [OK] libSDL2main.a found
) else (
    echo [ERROR] libSDL2main.a NOT found - you may have MSVC version!
    set ERRORS=1
)

if exist "C:\SDL2\lib\libSDL2.a" (
    echo [OK] libSDL2.a found
) else (
    echo [WARNING] libSDL2.a not found (optional for dynamic linking)
)

REM Check for MSVC files that shouldn't be there
if exist "C:\SDL2\lib\SDL2.lib" (
    echo [WARNING] SDL2.lib found - this is MSVC format, incompatible with MinGW!
    echo [WARNING] You have the MSVC version, not MinGW version!
    set ERRORS=1
)

if exist "C:\SDL2\lib\SDL2main.lib" (
    echo [WARNING] SDL2main.lib found - this is MSVC format, incompatible with MinGW!
    echo [WARNING] You have the MSVC version, not MinGW version!
    set ERRORS=1
)

REM Check DLL
if exist "C:\SDL2\bin\SDL2.dll" (
    echo [OK] SDL2.dll found
) else (
    echo [ERROR] SDL2.dll NOT found
    set ERRORS=1
)

echo.
echo ========================================
if %ERRORS%==0 (
    echo Result: ALL CHECKS PASSED!
    echo Your SDL2 installation appears correct for MinGW.
) else (
    echo Result: ERRORS FOUND!
    echo.
    echo You may have the MSVC version of SDL2 instead of MinGW version.
    echo Please see MINGW_BUILD.md for correct installation instructions.
    echo.
    echo Quick fix:
    echo 1. Delete C:\SDL2
    echo 2. Run: setup_sdl2_windows.bat
)
echo ========================================
echo.

pause
