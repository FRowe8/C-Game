@echo off
REM Setup verification script
echo ========================================
echo Quantum Idle - Setup Checker
echo ========================================
echo.
echo This script checks if you have all the prerequisites installed.
echo.

set ALL_OK=1

REM Check CMake
echo [1/3] Checking CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo    ❌ CMake NOT FOUND
    echo    Install from: https://cmake.org/download/
    echo    See INSTALL_CMAKE.md for instructions
    set ALL_OK=0
) else (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VER=%%i
    echo    ✅ CMake found: version %CMAKE_VER%
)
echo.

REM Check C++ Compiler (try to find cl.exe for MSVC)
echo [2/3] Checking C++ Compiler...
where cl.exe >nul 2>&1
if errorlevel 1 (
    REM Try to find it in common VS locations
    if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC" (
        echo    ✅ Visual Studio Build Tools found (not in PATH)
        echo    Note: CMake will find it automatically
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC" (
        echo    ✅ Visual Studio 2022 found (not in PATH)
        echo    Note: CMake will find it automatically
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC" (
        echo    ✅ Visual Studio 2019 Build Tools found (not in PATH)
        echo    Note: CMake will find it automatically
    ) else (
        echo    ❌ Visual Studio Build Tools NOT FOUND
        echo    Install from: https://visualstudio.microsoft.com/downloads/
        echo    Choose "Build Tools for Visual Studio 2022"
        echo    Select "Desktop development with C++"
        set ALL_OK=0
    )
) else (
    for /f "tokens=*" %%i in ('cl.exe 2^>^&1 ^| findstr /C:"Version"') do set MSVC_VER=%%i
    echo    ✅ MSVC Compiler found: %MSVC_VER%
)
echo.

REM Check SDL2
echo [3/3] Checking SDL2...
if exist "C:\SDL2\include\SDL.h" (
    echo    ✅ SDL2 found at C:\SDL2
) else if defined SDL2_DIR (
    if exist "%SDL2_DIR%\include\SDL.h" (
        echo    ✅ SDL2 found at %SDL2_DIR%
    ) else (
        echo    ⚠️  SDL2_DIR is set but SDL2 not found there
        echo    Run: build.bat (it will offer to download SDL2)
        set ALL_OK=0
    )
) else (
    echo    ⚠️  SDL2 not found (will be auto-downloaded)
    echo    Run: build.bat (it will offer to download SDL2)
    REM Don't set ALL_OK=0 here since build.bat handles this
)
echo.

echo ========================================
if %ALL_OK%==1 (
    echo ✅ All prerequisites found!
    echo.
    echo You're ready to build. Run: build.bat
) else (
    echo ❌ Some prerequisites are missing
    echo.
    echo Please install the missing items above, then run this script again.
    echo.
    echo Quick links:
    echo - CMake: https://cmake.org/download/
    echo - Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/
    echo - See INSTALL_CMAKE.md for detailed instructions
)
echo ========================================
echo.
pause
