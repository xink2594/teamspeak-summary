@echo off
REM Voice Monitor Plugin - Windows Build Script

echo ========================================
echo  TeamSpeak 3 Voice Monitor Plugin Build
echo ========================================

REM Create build directory
if not exist "build" mkdir build

REM Configure with CMake
echo.
echo [1/3] Configuring with CMake...
cmake -B build -G "MinGW Makefiles" .
if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

REM Build
echo.
echo [2/3] Building plugin...
cmake --build build --config Release
if errorlevel 1 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo [3/3] Build complete!
echo.
echo Output: build\VoiceMonitorPlugin_win64.dll
echo.
echo To install:
echo   1. Copy build\VoiceMonitorPlugin_win64.dll to your TeamSpeak 3 plugins folder
echo   2. Restart TeamSpeak 3 client
echo   3. Enable the plugin in Settings ^> Plugins
echo.
pause
