cmake_minimum_required(VERSION 3.10)
project(outtahere)

set(CMAKE_C_STANDARD 99)

# Find packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Check for TMX library - Set to true for testing
set(HAVE_TMX TRUE)
add_definitions(-DHAVE_TMX)
message(STATUS "TMX library found (simulated)")

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.c
    src/tilemap.c
)

# Create executable
add_executable(outtahere ${SOURCES})

# Link libraries
target_link_libraries(outtahere ${SDL2_LIBRARIES})

# Compiler flags
target_compile_options(outtahere PRIVATE ${SDL2_CFLAGS_OTHER})