@echo off
REM AstroDynamics Engine Build Script for Windows

echo === AstroDynamics Engine Build Script ===

REM Check if cmake is available
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake is not installed or not in PATH
    exit /b 1
)

REM Create build directory
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure with CMake
echo Configuring project...
cmake .. -G "Visual Studio 16 2019" -A x64

if %errorlevel% neq 0 (
    echo CMake configuration failed
    exit /b 1
)

REM Build
echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed
    exit /b 1
)

echo Build successful!
echo Run with: build\Release\AstroDynamicsEngine.exe

REM Create directories if they don't exist
if not exist "assets\fonts" (
    mkdir assets\fonts
    echo Note: Please add a font file to assets\fonts\arial.ttf for HUD display
)

if not exist "scenarios" (
    echo Creating scenarios directory...
    mkdir scenarios
)

cd ..
pause
