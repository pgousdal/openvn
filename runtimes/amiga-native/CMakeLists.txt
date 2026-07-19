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
    src/bitmap_font.c
    src/condition.c
    src/audio.c
    src/audio_8svx.c
    src/audio_mod.c
    src/audio_mod_player.c
    src/audio_paula.c
    src/dialogue.c
    src/dispatch.c
    src/graphics.c
    src/image_ilbm.c
    src/palette.c
    src/planar.c
    src/player.c
    src/player_state.c
    src/scene_layout.c
    src/story.c
    src/variables.c
    "${OPENVN_GENERATED_STORY_SOURCE}"
    "${OPENVN_GENERATED_ASSETS_SOURCE}"
)

if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    target_sources(
        openvn_dispatch
        PRIVATE
            src/audio_amiga.c
            src/bitmap_amiga.c
            src/bitmap_font_amiga.c
            src/display_amiga.c
            src/graphics_amiga.c
    )
else()
    target_sources(
        openvn_dispatch
        PRIVATE
            src/audio_host.c
            src/graphics_host.c
    )
endif()

target_include_directories(
    openvn_dispatch
    PUBLIC
        include
        "${OPENVN_GENERATED_STORY_DIR}"
        "${OPENVN_GENERATED_ASSETS_DIR}"
)

add_executable(openvn-player-host src/main_host.c)
target_link_libraries(openvn-player-host PRIVATE openvn_dispatch)

add_executable(test-openvn-dialogue tests/test_dialogue.c)
target_link_libraries(test-openvn-dialogue PRIVATE openvn_dispatch)

add_executable(test-openvn-dispatch tests/test_dispatch.c)
target_link_libraries(test-openvn-dispatch PRIVATE openvn_dispatch)

add_executable(test-openvn-variables tests/test_variables.c)
target_link_libraries(test-openvn-variables PRIVATE openvn_dispatch)

add_executable(test-openvn-condition tests/test_condition.c)
target_link_libraries(test-openvn-condition PRIVATE openvn_dispatch)

add_executable(test-openvn-story tests/test_story.c)
target_include_directories(test-openvn-story PRIVATE tests)
target_link_libraries(test-openvn-story PRIVATE openvn_dispatch)

add_executable(test-openvn-graphics tests/test_graphics.c)
target_link_libraries(test-openvn-graphics PRIVATE openvn_dispatch)

add_executable(test-openvn-scene-layout tests/test_scene_layout.c)
target_link_libraries(test-openvn-scene-layout PRIVATE openvn_dispatch)

add_executable(test-openvn-bitmap-font tests/test_bitmap_font.c)
target_link_libraries(test-openvn-bitmap-font PRIVATE openvn_dispatch)

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

add_executable(test-openvn-player-audio tests/test_player_audio.c)
target_link_libraries(test-openvn-player-audio PRIVATE openvn_dispatch)

add_executable(test-openvn-8svx tests/test_8svx.c)
target_link_libraries(test-openvn-8svx PRIVATE openvn_dispatch)

add_executable(test-openvn-mod tests/test_mod.c)
target_link_libraries(test-openvn-mod PRIVATE openvn_dispatch)

add_executable(test-openvn-mod-player tests/test_mod_player.c)
target_link_libraries(test-openvn-mod-player PRIVATE openvn_dispatch)

add_executable(test-openvn-paula tests/test_paula.c)
add_executable(test-openvn-mod-effects tests/test_mod_effects.c)
target_link_libraries(test-openvn-mod-effects PRIVATE openvn_dispatch)
target_link_libraries(test-openvn-paula PRIVATE openvn_dispatch)

enable_testing()
add_test(NAME dialogue COMMAND test-openvn-dialogue)
add_test(NAME dispatch COMMAND test-openvn-dispatch)
add_test(NAME variables COMMAND test-openvn-variables)
add_test(NAME condition COMMAND test-openvn-condition)
add_test(NAME story COMMAND test-openvn-story)
add_test(NAME graphics COMMAND test-openvn-graphics)
add_test(NAME scene-layout COMMAND test-openvn-scene-layout)
add_test(NAME bitmap-font COMMAND test-openvn-bitmap-font)
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
add_test(NAME player-audio COMMAND test-openvn-player-audio)
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
add_test(NAME mod-effects COMMAND test-openvn-mod-effects)

if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    add_executable(openvn-player src/main_amiga.c)
    target_link_libraries(openvn-player PRIVATE openvn_dispatch)
endif()

# BEGIN OPENVN M6 PR1I AMIGA SDK LINK
if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    # SDK stubs used by DrawDTObject(), CreatePort(), and related Amiga calls.
    target_link_libraries(openvn-player PRIVATE amiga)
endif()
# END OPENVN M6 PR1I AMIGA SDK LINK

# BEGIN OPENVN M6 PR1M GCC INLINE BINDINGS
if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    # Select GCC inline library-vector bindings from <proto/*.h>.
    # Without this, OS calls are emitted as unresolved external C symbols.
    target_compile_definitions(openvn_dispatch PRIVATE __USE_INLINE__)
    target_compile_definitions(openvn-player PRIVATE __USE_INLINE__)
endif()
# END OPENVN M6 PR1M GCC INLINE BINDINGS
