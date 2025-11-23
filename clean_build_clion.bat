@echo off
REM Clean build script for CLion/MinGW

echo ========================================
echo Clean Build for CLion/MinGW
echo ========================================
echo.

echo This will:
echo 1. Delete cmake-build-debug directory
echo 2. Force CMake to reconfigure from scratch
echo 3. Build the project
echo.
echo Press Ctrl+C to cancel, or
pause

echo.
echo [1/4] Deleting old build directory...
if exist cmake-build-debug (
    rmdir /s /q cmake-build-debug
    echo Old build deleted.
) else (
    echo No old build found.
)

echo.
echo [2/4] Creating fresh build directory...
mkdir cmake-build-debug
cd cmake-build-debug

echo.
echo [3/4] Running CMake configuration...
echo.
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo Check the output above for errors.
    pause
    exit /b 1
)

echo.
echo [4/4] Building project...
echo.
cmake --build .

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    echo Check the output above for errors.
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Copying SDL2.dll to build directory...
copy C:\SDL2\bin\SDL2.dll . >nul 2>&1

echo.
echo Executable location: cmake-build-debug\QuantumIdle.exe
echo.
echo To run the game:
echo   cd cmake-build-debug
echo   QuantumIdle.exe
echo.
cd ..
pause
