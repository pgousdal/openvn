cmake_minimum_required(VERSION 3.16)
project(openvn_amiga_native C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(openvn_dispatch STATIC
    src/dispatch.c
    src/player_state.c
    src/story.c
)

target_include_directories(openvn_dispatch PUBLIC include)

add_executable(openvn-player-host src/main_host.c)
target_link_libraries(openvn-player-host PRIVATE openvn_dispatch)

add_executable(test-openvn-dispatch tests/test_dispatch.c)
target_link_libraries(test-openvn-dispatch PRIVATE openvn_dispatch)

add_executable(test-openvn-story tests/test_story.c)
target_link_libraries(test-openvn-story PRIVATE openvn_dispatch)

enable_testing()
add_test(NAME dispatch COMMAND test-openvn-dispatch)
add_test(
    NAME story
    COMMAND test-openvn-story ${CMAKE_CURRENT_SOURCE_DIR}/tests/minimal-story.json
)

if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    add_executable(openvn-player src/main_amiga.c)
    target_link_libraries(openvn-player PRIVATE openvn_dispatch)
endif()
