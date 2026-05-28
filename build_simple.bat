@echo off
REM Voice Monitor Plugin - Simple Build Script (No CMake Required)

echo ========================================
echo  TeamSpeak 3 Voice Monitor Plugin Build
echo ========================================

REM Check if gcc is available
where gcc >nul 2>&1
if errorlevel 1 (
    echo ERROR: gcc not found! Please install MinGW-w64 or add it to PATH.
    echo.
    echo Download MinGW-w64 from: https://www.mingw-w64.org/
    echo Or use: winget install mingw
    pause
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build

REM Set SDK path
set SDK_DIR=..\ts3client-pluginsdk-master

echo.
echo [1/2] Compiling plugin...
echo.

REM Compile plugin.c to object file
gcc -c -O2 -Wall -fPIC ^
    -I"%SDK_DIR%\include" ^
    -I"src" ^
    -DWIN32 -D_WIN32 ^
    -o build\plugin.o ^
    src\plugin.c

if errorlevel 1 (
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)

echo [2/2] Linking plugin...
echo.

REM Link to create DLL
gcc -shared -o build\VoiceMonitorPlugin_win64.dll build\plugin.o

if errorlevel 1 (
    echo ERROR: Linking failed!
    pause
    exit /b 1
)

echo ========================================
echo  Build Complete!
echo ========================================
echo.
echo Output: build\VoiceMonitorPlugin_win64.dll
echo.
echo To install:
echo   1. Copy build\VoiceMonitorPlugin_win64.dll to:
echo      %%APPDATA%%\TS3Client\plugins\
echo   2. Restart TeamSpeak 3 client
echo   3. Enable the plugin in Settings ^> Plugins
echo.
pause
