@echo off
REM Build script for Windows

echo Quantum Idle - Build Script
echo ============================
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

REM Build
echo Building...
cmake --build . --config Release

echo.
echo Build complete! Run with: build\Release\QuantumIdle.exe
pause
