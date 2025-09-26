cmake_minimum_required(VERSION 3.16)
project(outtahere C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set SDL2 options before any FetchContent operations
if(WIN32)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries on Windows" FORCE)
    set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Disable SDL2main" FORCE)
endif()

include_directories(./include)

include(FetchContent)

find_package(LibXml2 QUIET)
if(NOT LibXml2_FOUND)
    message(STATUS "LibXml2 not found, fetching...")
    
    # On Windows, we need to handle dependencies
    if(WIN32)
        # Disable features that require external dependencies on Windows
        set(LIBXML2_WITH_ICONV OFF CACHE BOOL "Build with Iconv support")
        set(LIBXML2_WITH_ICU OFF CACHE BOOL "Build with ICU support") 
        set(LIBXML2_WITH_LZMA OFF CACHE BOOL "Build with LZMA support")
        set(LIBXML2_WITH_ZLIB OFF CACHE BOOL "Build with ZLIB support")
        set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "Build Python bindings")
        set(LIBXML2_WITH_PROGRAMS OFF CACHE BOOL "Build programs")
        set(LIBXML2_WITH_TESTS OFF CACHE BOOL "Build tests")
    endif()
    
    FetchContent_Declare(
        libxml2
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
        GIT_TAG v2.10.3   # stable tag
    )
    FetchContent_MakeAvailable(libxml2)
    
    # Set variables for libtmx to find libxml2
    set(LIBXML2_INCLUDE_DIR ${libxml2_SOURCE_DIR}/include CACHE PATH "LibXml2 include directory")
    if(WIN32)
        set(LIBXML2_LIBRARY xml2 CACHE STRING "LibXml2 library")  # Use target name
    else()
        set(LIBXML2_LIBRARY ${libxml2_BINARY_DIR}/libxml2.a CACHE PATH "LibXml2 library")
    endif()
    
    # Also set the variables that FindLibXml2.cmake looks for
    set(LibXml2_FOUND TRUE CACHE BOOL "LibXml2 found")
    set(LIBXML2_FOUND TRUE CACHE BOOL "LibXml2 found")
    set(LibXml2_INCLUDE_DIRS ${libxml2_SOURCE_DIR}/include CACHE PATH "LibXml2 include directory")
    set(LibXml2_LIBRARIES xml2 CACHE STRING "LibXml2 libraries")
endif()

find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
    message(STATUS "ZLIB not found, fetching...")
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.1  # Updated to newer version with better CMake support
    )
    FetchContent_MakeAvailable(zlib)
endif()

# -------------------------
# SDL2 (cross-platform)
# -------------------------
if(WIN32)
    # On Windows, try to find SDL2 using find_package first
    find_package(SDL2 QUIET)
    if(NOT SDL2_FOUND)
        # Fallback to FetchContent for Windows
        message(STATUS "SDL2 not found on Windows, fetching...")
        

        
        FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.28.5
        )
        FetchContent_MakeAvailable(SDL2)
        set(SDL2_LIBRARIES SDL2::SDL2)
        set(SDL2_INCLUDE_DIRS "")
        set(SDL2_CFLAGS_OTHER "")
    endif()
else()
    # On Unix-like systems, use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
endif()

# -------------------------
# SDL2_image
# -------------------------
if(WIN32)
    # On Windows, try to find SDL2_image using find_package first
    find_package(SDL2_image QUIET)
    if(NOT SDL2_image_FOUND)
        # Fallback to FetchContent for Windows
        message(STATUS "SDL2_image not found on Windows, fetching...")
        
        FetchContent_Declare(
            SDL2_image
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
            GIT_TAG release-2.8.2
        )
        FetchContent_MakeAvailable(SDL2_image)
        set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
        set(SDL2_IMAGE_INCLUDE_DIRS "")
    endif()
else()
    # On Unix-like systems, use pkg-config
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    if(NOT SDL2_IMAGE_FOUND)
        message(STATUS "SDL2_image not found, fetching...")
        
        FetchContent_Declare(
            SDL2_image
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
            GIT_TAG release-2.8.2
        )
        FetchContent_MakeAvailable(SDL2_image)
        set(SDL2_IMAGE_LIBRARIES SDL2_image)
        set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_image_SOURCE_DIR}/include)
    endif()
endif()

# -------------------------
# libtmx (optional for now on Windows due to libxml2 complexity)
# -------------------------
if(NOT WIN32)
    find_package(tmx QUIET)
    if(NOT tmx_FOUND)
        message(STATUS "libtmx not found, fetching with FetchContent...")
        FetchContent_Declare(
            libtmx
            GIT_REPOSITORY https://github.com/baylej/tmx.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(libtmx)
    endif()
else()
    message(STATUS "Skipping libtmx on Windows - will build without TMX support for now")
    set(tmx_FOUND FALSE)
endif()

# -------------------------
# Your game executable
# -------------------------
add_executable(outtahere
  src/main.c
	src/game.c
	src/player.c
  src/tilemap.c
  include/tilemap.h
  include/game.h
	include/player.h
)

# Link SDL2
if(WIN32)
    if(TARGET SDL2::SDL2)
        # Modern CMake target-based linking for Windows (when using find_package)
        target_link_libraries(outtahere PRIVATE SDL2::SDL2)
    else()
        # When using FetchContent on Windows - link with static SDL2
        # We disabled SDL2main, so only link SDL2
        target_link_libraries(outtahere PRIVATE SDL2-static)
    endif()
else()
    # Traditional linking for Unix-like systems
    target_include_directories(outtahere PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(outtahere PRIVATE ${SDL2_LIBRARIES})
    target_compile_options(outtahere PRIVATE ${SDL2_CFLAGS_OTHER})
endif()

# Link SDL2_image
if(WIN32)
    if(TARGET SDL2_image::SDL2_image)
        # Modern CMake target-based linking for Windows (when using find_package)
        target_link_libraries(outtahere PRIVATE SDL2_image::SDL2_image)
    else()
        # When using FetchContent on Windows
        target_link_libraries(outtahere PRIVATE SDL2_image)
    endif()
else()
    # Traditional linking for Unix-like systems
    target_include_directories(outtahere PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(outtahere PRIVATE ${SDL2_IMAGE_LIBRARIES})
    target_compile_options(outtahere PRIVATE ${SDL2_IMAGE_CFLAGS_OTHER})
endif()

# Link libtmx (if available)
if(TARGET tmx)
    target_link_libraries(outtahere PRIVATE tmx)
    target_compile_definitions(outtahere PRIVATE HAS_TMX=1)
else()
    message(STATUS "Building without TMX support")
    target_compile_definitions(outtahere PRIVATE HAS_TMX=0)
endif()


