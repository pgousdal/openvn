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

set(
    OPENVN_GENERATED_ASSETS_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/assets.generated.c"
    CACHE FILEPATH
    "Generated OpenVN asset table source"
)

get_filename_component(
    OPENVN_GENERATED_STORY_DIR
    "${OPENVN_GENERATED_STORY_SOURCE}"
    DIRECTORY
)

get_filename_component(
    OPENVN_GENERATED_ASSETS_DIR
    "${OPENVN_GENERATED_ASSETS_SOURCE}"
    DIRECTORY
)

add_library(openvn_dispatch STATIC
    src/assets_runtime.c
    src/audio.c
    src/audio_8svx.c
    src/audio_host.c
    src/audio_mod.c
    src/audio_mod_player.c
    src/audio_paula.c
    src/dispatch.c
    src/graphics.c
    src/graphics_host.c
    src/image_ilbm.c
    src/palette.c
    src/planar.c
    src/player.c
    src/player_state.c
    src/story.c
    "${OPENVN_GENERATED_STORY_SOURCE}"
    "${OPENVN_GENERATED_ASSETS_SOURCE}"
)

target_include_directories(
    openvn_dispatch
    PUBLIC
        include
        "${OPENVN_GENERATED_STORY_DIR}"
        "${OPENVN_GENERATED_ASSETS_DIR}"
)

add_executable(openvn-player-host src/main_host.c)
target_link_libraries(openvn-player-host PRIVATE openvn_dispatch)

add_executable(test-openvn-dispatch tests/test_dispatch.c)
target_link_libraries(test-openvn-dispatch PRIVATE openvn_dispatch)

add_executable(test-openvn-story tests/test_story.c)
target_include_directories(test-openvn-story PRIVATE tests)
target_link_libraries(test-openvn-story PRIVATE openvn_dispatch)

add_executable(test-openvn-graphics tests/test_graphics.c)
target_link_libraries(test-openvn-graphics PRIVATE openvn_dispatch)

add_executable(test-openvn-image tests/test_image.c)
target_link_libraries(test-openvn-image PRIVATE openvn_dispatch)

add_executable(
    test-openvn-assets-runtime
    tests/test_assets_runtime.c
)
target_link_libraries(test-openvn-assets-runtime PRIVATE openvn_dispatch)

add_executable(test-openvn-planar tests/test_planar.c)
target_link_libraries(test-openvn-planar PRIVATE openvn_dispatch)

add_executable(test-openvn-palette tests/test_palette.c)
target_link_libraries(test-openvn-palette PRIVATE openvn_dispatch)

add_executable(test-openvn-audio tests/test_audio.c)
target_link_libraries(test-openvn-audio PRIVATE openvn_dispatch)

add_executable(test-openvn-8svx tests/test_8svx.c)
target_link_libraries(test-openvn-8svx PRIVATE openvn_dispatch)

add_executable(test-openvn-mod tests/test_mod.c)
target_link_libraries(test-openvn-mod PRIVATE openvn_dispatch)

add_executable(test-openvn-mod-player tests/test_mod_player.c)
target_link_libraries(test-openvn-mod-player PRIVATE openvn_dispatch)

add_executable(test-openvn-paula tests/test_paula.c)
target_link_libraries(test-openvn-paula PRIVATE openvn_dispatch)

enable_testing()
add_test(NAME dispatch COMMAND test-openvn-dispatch)
add_test(NAME story COMMAND test-openvn-story)
add_test(NAME graphics COMMAND test-openvn-graphics)
set_tests_properties(
    graphics
    PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
add_test(
    NAME image
    COMMAND test-openvn-image
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/lighthouse_storm.iff
)
add_test(NAME assets-runtime COMMAND test-openvn-assets-runtime)
add_test(NAME planar COMMAND test-openvn-planar)
add_test(NAME palette COMMAND test-openvn-palette)
add_test(NAME audio COMMAND test-openvn-audio)
set_tests_properties(
    audio
    PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
add_test(
    NAME 8svx
    COMMAND test-openvn-8svx
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/thunder.8svx
)
add_test(
    NAME mod
    COMMAND test-openvn-mod
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/storm.mod
)
add_test(
    NAME mod-player
    COMMAND test-openvn-mod-player
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/storm.mod
)
add_test(NAME paula COMMAND test-openvn-paula)

if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    target_sources(
        openvn_dispatch
        PRIVATE
            src/audio_amiga.c
            src/bitmap_amiga.c
            src/display_amiga.c
            src/graphics_amiga.c
    )

    add_executable(openvn-player src/main_amiga.c)
    target_link_libraries(openvn-player PRIVATE openvn_dispatch)
endif()
