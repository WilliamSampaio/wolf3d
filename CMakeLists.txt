cmake_minimum_required(VERSION 3.16)
project(wolf3d C)

find_package(SDL2 REQUIRED)

function(wolf3d_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(wolf3d_enable_test_assertions target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
        target_link_options(${target} PRIVATE /STACK:8388608)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endfunction()

add_library(wolf3d_core
    src/data_files.c src/vswap.c src/map.c src/raycast.c src/player.c src/game.c
    src/render.c src/door.c)
target_include_directories(wolf3d_core PUBLIC src)
target_compile_features(wolf3d_core PUBLIC c_std_99)
if(NOT MSVC)
    target_link_libraries(wolf3d_core PUBLIC m)
endif()
wolf3d_enable_warnings(wolf3d_core)

add_executable(wolf3d src/main.c)
if(TARGET SDL2::SDL2main)
    target_link_libraries(wolf3d PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(wolf3d PRIVATE wolf3d_core SDL2::SDL2)
wolf3d_enable_warnings(wolf3d)

enable_testing()
add_executable(test_vswap tests/test_vswap.c)
target_link_libraries(test_vswap PRIVATE wolf3d_core)
target_compile_definitions(test_vswap PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/shareware-v1.4")
wolf3d_enable_warnings(test_vswap)
wolf3d_enable_test_assertions(test_vswap)
add_test(NAME vswap COMMAND test_vswap)

add_executable(test_map tests/test_map.c)
target_link_libraries(test_map PRIVATE wolf3d_core)
target_compile_definitions(test_map PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/shareware-v1.4")
wolf3d_enable_warnings(test_map)
wolf3d_enable_test_assertions(test_map)
add_test(NAME map COMMAND test_map)

add_executable(test_raycast tests/test_raycast.c)
target_link_libraries(test_raycast PRIVATE wolf3d_core)
wolf3d_enable_warnings(test_raycast)
wolf3d_enable_test_assertions(test_raycast)
add_test(NAME raycast COMMAND test_raycast)

add_executable(test_player tests/test_player.c)
target_link_libraries(test_player PRIVATE wolf3d_core)
wolf3d_enable_warnings(test_player)
wolf3d_enable_test_assertions(test_player)
add_test(NAME player COMMAND test_player)

add_executable(test_game tests/test_game.c)
target_link_libraries(test_game PRIVATE wolf3d_core)
wolf3d_enable_warnings(test_game)
wolf3d_enable_test_assertions(test_game)
add_test(NAME game COMMAND test_game)

add_executable(test_render tests/test_render.c)
target_link_libraries(test_render PRIVATE wolf3d_core)
wolf3d_enable_warnings(test_render)
wolf3d_enable_test_assertions(test_render)
add_test(NAME render COMMAND test_render)
