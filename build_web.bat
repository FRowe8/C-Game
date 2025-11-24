@echo off
REM Build script for Web (Emscripten) version - Windows

echo =========================================
echo Quantum Idle - Web Build (Emscripten)
echo =========================================
echo.

REM Check if Emscripten is installed
where emcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Emscripten not found!
    echo.
    echo Please install Emscripten SDK:
    echo   git clone https://github.com/emscripten-core/emsdk.git
    echo   cd emsdk
    echo   emsdk install latest
    echo   emsdk activate latest
    echo   emsdk_env.bat
    echo.
    pause
    exit /b 1
)

echo Emscripten found
echo.

REM Create build directory
echo [1/4] Creating build directory...
if exist build-web rmdir /s /q build-web
mkdir build-web
cd build-web

REM Configure with Emscripten
echo [2/4] Configuring CMake with Emscripten...
call emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DPLATFORM_WEB=ON -G "MinGW Makefiles"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM Build
echo [3/4] Building...
call emmake make -j4

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

REM Copy to web directory
echo [4/4] Copying files to web directory...
cd ..
if not exist web\dist mkdir web\dist
copy /Y build-web\QuantumIdle.js web\dist\
copy /Y build-web\QuantumIdle.wasm web\dist\
copy /Y build-web\QuantumIdle.data web\dist\ 2>nul
copy /Y web\index.html web\dist\

echo.
echo =========================================
echo Build Complete!
echo =========================================
echo.
echo Output directory: web\dist\
echo.
echo To test locally:
echo   cd web\dist
echo   python -m http.server 8080
echo   Then open: http://localhost:8080
echo.
echo To deploy:
echo   Upload the contents of web\dist\ to your web server
echo.
pause
