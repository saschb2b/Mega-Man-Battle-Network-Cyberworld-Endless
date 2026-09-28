# The game's C for Android: SDL2 from its source (docker/sdl2.sh android) as
# the libSDL2.so SDL's activity loads, and every source of the desktop
# builds (Makefile) as libmain.so, with the GBA core of docker/mgba.sh.
cmake_minimum_required(VERSION 3.18)
project(cyberworld C)

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL2_SRC} sdl2)

set(GAME ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(MGBA /opt/mgba/android/${ANDROID_ABI})
file(GLOB SRCS ${GAME}/src/*/*.c)
file(GLOB DIRS LIST_DIRECTORIES true ${GAME}/src/*)
file(WRITE ${CMAKE_BINARY_DIR}/gen/version.h "#define CW_VERSION \"${CW_VERSION}\"\n")

add_library(main SHARED ${SRCS})
target_include_directories(main PRIVATE ${DIRS} ${CMAKE_BINARY_DIR}/gen ${MGBA}/include)
target_compile_options(main PRIVATE -std=c11 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)
if(CW_WERROR)
	target_compile_options(main PRIVATE -Werror)
endif()
target_compile_definitions(main PRIVATE _DEFAULT_SOURCE)
target_link_libraries(main PRIVATE SDL2 ${MGBA}/lib/libmgba.a log android m)
