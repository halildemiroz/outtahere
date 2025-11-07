cmake_minimum_required(VERSION 3.10)
project(outtahere C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(./include)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})

find_package(tmx REQUIRED)
include_directories(${TMX_INCLUDE_DIRS})

add_executable(outtahere
	src/anim.c
	src/camera.c
	src/game.c
	src/main.c
	src/menu.c
	src/player.c
	src/polygon_collision.c
	src/tilemap.c
	include/anim.h
	include/camera.h
	include/game.h
	include/menu.h
	include/player.h
	include/polygon_collision.h
	include/tilemap.h
	)

target_link_libraries(outtahere sdl2 sdl2_image sdl2_ttf tmx m)
