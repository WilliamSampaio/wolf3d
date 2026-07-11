cmake_minimum_required(VERSION 3.16)
project(wolf3d C)

find_package(SDL2 REQUIRED)

add_executable(wolf3d src/main.c src/data_files.c src/vswap.c)
target_link_libraries(wolf3d PRIVATE SDL2::SDL2)
target_compile_options(wolf3d PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(test_vswap tests/test_vswap.c src/vswap.c)
target_include_directories(test_vswap PRIVATE src)
target_compile_options(test_vswap PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME vswap COMMAND test_vswap)
