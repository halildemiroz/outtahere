cmake_minimum_required(VERSION 3.16)

# Project name and language
project(OuttaHere LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable debugging symbols
set(CMAKE_BUILD_TYPE Debug)

# Include directories
include_directories(include)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find SDL2_image
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

# Find SDL2_ttf
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})

# Find libtmx
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBTMX REQUIRED libtmx)
include_directories(${LIBTMX_INCLUDE_DIRS})

# Source files
file(GLOB SOURCES "src/*.c")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link SDL2, SDL2_image, SDL2_ttf, and libtmx
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${LIBTMX_LIBRARIES})

# macOS-specific settings
if(APPLE)
    message(STATUS "Building for macOS")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework OpenGL" "-framework IOKit" "-framework CoreVideo")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Print a message when done
message(STATUS "CMake configuration complete. Build files are in ${CMAKE_BINARY_DIR}")
