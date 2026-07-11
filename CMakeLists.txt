cmake_minimum_required(VERSION 3.16)
project(wolf3d C)

find_package(SDL2 REQUIRED)

add_executable(wolf3d src/main.c)
target_link_libraries(wolf3d PRIVATE SDL2::SDL2)
target_compile_options(wolf3d PRIVATE -Wall -Wextra -Wpedantic)

