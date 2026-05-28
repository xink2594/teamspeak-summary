#!/bin/bash
# Voice Monitor Plugin - Linux/macOS Build Script

echo "========================================"
echo " TeamSpeak 3 Voice Monitor Plugin Build"
echo "========================================"

# Create build directory
mkdir -p build

# Configure with CMake
echo ""
echo "[1/3] Configuring with CMake..."
cmake -B build .
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    exit 1
fi

# Build
echo ""
echo "[2/3] Building plugin..."
cmake --build build --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "[3/3] Build complete!"
echo ""

# Detect platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Output: build/VoiceMonitorPlugin_mac.dylib"
else
    echo "Output: build/VoiceMonitorPlugin_linux_amd64.so"
fi

echo ""
echo "To install:"
echo "  1. Copy the built library to your TeamSpeak 3 plugins folder"
echo "  2. Restart TeamSpeak 3 client"
echo "  3. Enable the plugin in Settings > Plugins"
echo ""
