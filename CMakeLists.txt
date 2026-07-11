cmake_minimum_required(VERSION 3.16)
project(wolf3d C)

find_package(SDL2 REQUIRED)

add_executable(wolf3d src/main.c src/data_files.c src/vswap.c src/map.c src/raycast.c)
target_link_libraries(wolf3d PRIVATE SDL2::SDL2 m)
target_compile_options(wolf3d PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(test_vswap tests/test_vswap.c src/vswap.c)
target_include_directories(test_vswap PRIVATE src)
target_compile_options(test_vswap PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME vswap COMMAND test_vswap)

add_executable(test_map tests/test_map.c src/map.c)
target_include_directories(test_map PRIVATE src)
target_compile_options(test_map PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME map COMMAND test_map)

add_executable(test_raycast tests/test_raycast.c src/raycast.c)
target_include_directories(test_raycast PRIVATE src)
target_link_libraries(test_raycast PRIVATE m)
target_compile_options(test_raycast PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME raycast COMMAND test_raycast)
