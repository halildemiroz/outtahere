# Outta Here - Mario-style Platformer Game

A Mario-style platformer game written in C using SDL2 for graphics and libtmx for tilemap support.

## Features

- SDL2-based game engine with hardware-accelerated rendering
- Mario-style physics with gravity and jumping
- Tilemap support using libtmx for level design
- Keyboard controls (Arrow keys/WASD for movement, Space for jump)
- Basic collision detection
- Extensible architecture for adding more game features

## Dependencies

Before building, make sure you have the following packages installed:

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libtmx-dev
```

### Fedora/RHEL:
```bash
sudo dnf install gcc cmake SDL2-devel SDL2_image-devel libtmx-devel
```

### macOS (with Homebrew):
```bash
brew install cmake sdl2 sdl2_image tmx
```

## Building

1. Clone the repository:
```bash
git clone https://github.com/halildemiroz/outtahere.git
cd outtahere
```

2. Create a build directory and compile:
```bash
mkdir build
cd build
cmake ..
make
```

3. Run the game:
```bash
./outtahere
```

## Controls

- **Arrow Keys** or **WASD**: Move left/right
- **Space**, **Up Arrow**, or **W**: Jump
- **ESC**: Quit game

## Project Structure

```
outtahere/
├── src/           # Source code files
├── include/       # Header files
├── assets/        # Game assets (maps, textures)
├── CMakeLists.txt # Build configuration
└── README.md      # This file
```

## Development

The game is structured with modular components:

- `game.c/h`: Main game loop and SDL initialization
- `player.c/h`: Player character logic and physics
- `physics.c/h`: Physics system (gravity, collisions)
- `tilemap.c/h`: Tilemap loading and rendering using libtmx

## Contributing

Feel free to submit issues and enhancement requests!