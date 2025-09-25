cmake_minimum_required(VERSION 3.10)
project(outtahere C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(/opt/homebrew/Cellar/sdl2/2.32.10/include)
link_directories(/opt/homebrew/Cellar/sdl2/2.32.10/lib)

include_directories(/opt/homebrew/Cellar/sdl2_image/2.8.8/include)
link_directories(/opt/homebrew/Cellar/sdl2_image/2.8.8/lib)

include_directories(/opt/homebrew/Cellar/tmx/1.10.0/include)
link_directories(/opt/homebrew/Cellar/tmx/1.10.0/lib)

link_libraries(SDL2main SDL2 SDL2_image tmx)

add_executable(outtahere
	src/main.c
  src/game.c
	)

