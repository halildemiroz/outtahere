cmake_minimum_required(VERSION 3.16)
project(outtahere C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(./include)

include(FetchContent)

find_package(LibXml2 QUIET)
if(NOT LibXml2_FOUND)
    message(STATUS "LibXml2 not found, fetching...")
    FetchContent_Declare(
        libxml2
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
        GIT_TAG v2.10.3   # stable tag
    )
    FetchContent_MakeAvailable(libxml2)
endif()

find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
    message(STATUS "ZLIB not found, fetching...")
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.2.13
    )
    FetchContent_MakeAvailable(zlib)
endif()

# -------------------------
# SDL2 (system only)
# -------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# -------------------------
# libtmx (system or fallback)
# -------------------------
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

# -------------------------
# Your game executable
# -------------------------
add_executable(outtahere
    src/main.c
	src/game.c
	src/player.c
	include/game.h
	include/player.h
)

# Link SDL2
target_include_directories(outtahere PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(outtahere PRIVATE ${SDL2_LIBRARIES})
target_compile_options(outtahere PRIVATE ${SDL2_CFLAGS_OTHER})

# Link libtmx
if(TARGET tmx)
    target_link_libraries(outtahere PRIVATE tmx)
endif()
# Tell libtmx where LibXml2 is
set(LIBXML2_INCLUDE_DIR ${libxml2_SOURCE_DIR}/include)
set(LIBXML2_LIBRARY ${libxml2_BINARY_DIR}/libxml2.a)

