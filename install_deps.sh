#!/bin/bash
# Installation script for Outta Here dependencies

echo "Installing dependencies for Outta Here Mario-style game..."

if command -v apt-get &> /dev/null; then
    echo "Detected Debian/Ubuntu system"
    sudo apt update
    sudo apt install -y build-essential cmake libsdl2-dev libsdl2-image-dev
    echo "✓ Dependencies installed successfully on Debian/Ubuntu"
    
elif command -v dnf &> /dev/null; then
    echo "Detected Fedora/RHEL system"
    sudo dnf install -y gcc cmake SDL2-devel SDL2_image-devel
    echo "✓ Dependencies installed successfully on Fedora/RHEL"
    
elif command -v pacman &> /dev/null; then
    echo "Detected Arch Linux system"
    sudo pacman -S --needed base-devel cmake sdl2 sdl2_image
    echo "✓ Dependencies installed successfully on Arch Linux"
    
elif command -v brew &> /dev/null; then
    echo "Detected macOS with Homebrew"
    brew install cmake sdl2 sdl2_image
    echo "✓ Dependencies installed successfully on macOS"
    
else
    echo "❌ Unsupported system. Please install manually:"
    echo "   - build-essential/gcc"
    echo "   - cmake"
    echo "   - SDL2 development libraries"
    echo "   - SDL2_image development libraries"
    exit 1
fi

echo ""
echo "Dependencies installed! You can now build the game:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"
echo "  ./outtahere"