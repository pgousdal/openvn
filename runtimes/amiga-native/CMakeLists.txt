cmake_minimum_required(VERSION 3.16)
project(openvn_amiga_native C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(
    OPENVN_GENERATED_STORY_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/story.generated.c"
    CACHE FILEPATH
    "Generated OpenVN story source"
)

get_filename_component(
    OPENVN_GENERATED_STORY_DIR
    "${OPENVN_GENERATED_STORY_SOURCE}"
    DIRECTORY
)

add_library(openvn_dispatch STATIC
    src/dispatch.c
    src/player_state.c
    src/story.c
    "${OPENVN_GENERATED_STORY_SOURCE}"
)

target_include_directories(
    openvn_dispatch
    PUBLIC
        include
        "${OPENVN_GENERATED_STORY_DIR}"
)

add_executable(openvn-player-host src/main_host.c)
target_link_libraries(openvn-player-host PRIVATE openvn_dispatch)

add_executable(test-openvn-dispatch tests/test_dispatch.c)
target_link_libraries(test-openvn-dispatch PRIVATE openvn_dispatch)

add_executable(test-openvn-story tests/test_story.c)
target_include_directories(test-openvn-story PRIVATE tests)
target_link_libraries(test-openvn-story PRIVATE openvn_dispatch)

enable_testing()
add_test(NAME dispatch COMMAND test-openvn-dispatch)
add_test(NAME story COMMAND test-openvn-story)

if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    add_executable(openvn-player src/main_amiga.c)
    target_link_libraries(openvn-player PRIVATE openvn_dispatch)
endif()
