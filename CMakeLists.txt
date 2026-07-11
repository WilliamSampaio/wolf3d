cmake_minimum_required(VERSION 3.16)
project(wolf3d C)

find_package(SDL2 REQUIRED)

add_library(wolf3d_core
    src/data_files.c src/vswap.c src/map.c src/raycast.c src/player.c src/game.c
    src/render.c)
target_include_directories(wolf3d_core PUBLIC src)
target_link_libraries(wolf3d_core PUBLIC m)
target_compile_options(wolf3d_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(wolf3d src/main.c)
target_link_libraries(wolf3d PRIVATE wolf3d_core SDL2::SDL2)
target_compile_options(wolf3d PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(test_vswap tests/test_vswap.c)
target_link_libraries(test_vswap PRIVATE wolf3d_core)
target_compile_definitions(test_vswap PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/shareware-v1.4")
target_compile_options(test_vswap PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME vswap COMMAND test_vswap)

add_executable(test_map tests/test_map.c)
target_link_libraries(test_map PRIVATE wolf3d_core)
target_compile_definitions(test_map PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/shareware-v1.4")
target_compile_options(test_map PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME map COMMAND test_map)

add_executable(test_raycast tests/test_raycast.c)
target_link_libraries(test_raycast PRIVATE wolf3d_core)
target_compile_options(test_raycast PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME raycast COMMAND test_raycast)

add_executable(test_player tests/test_player.c)
target_link_libraries(test_player PRIVATE wolf3d_core)
target_compile_options(test_player PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME player COMMAND test_player)

add_executable(test_game tests/test_game.c)
target_link_libraries(test_game PRIVATE wolf3d_core)
target_compile_options(test_game PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME game COMMAND test_game)

add_executable(test_render tests/test_render.c)
target_link_libraries(test_render PRIVATE wolf3d_core)
target_compile_options(test_render PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME render COMMAND test_render)
