cmake_minimum_required(VERSION 3.16)
project(outtahere VERSION 1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Find libtmx (tmx library) - optional
find_library(TMX_LIBRARY tmx)
find_path(TMX_INCLUDE_DIR tmx.h)

if(TMX_LIBRARY AND TMX_INCLUDE_DIR)
    set(HAVE_TMX TRUE)
    message(STATUS "Found libtmx: ${TMX_LIBRARY}")
else()
    set(HAVE_TMX FALSE)
    message(WARNING "libtmx not found. Tilemap functionality will be disabled.")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

if(HAVE_TMX)
    include_directories(${TMX_INCLUDE_DIR})
    add_definitions(-DHAVE_TMX=1)
endif()

# Source files
set(SOURCES
    src/main.c
    src/game.c
    src/player.c
    src/tilemap.c
    src/physics.c
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    m  # Math library
)

if(HAVE_TMX)
    target_link_libraries(${PROJECT_NAME} ${TMX_LIBRARY})
endif()

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})