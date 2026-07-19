#include "openvn_scene_layout.h"

#include "test_check.h"

int main(void) {
    OpenVNScenePosition position;

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 62U, 220U, OPENVN_ANCHOR_CENTER
    );
    OPENVN_TEST_CHECK(position.x == 98);
    OPENVN_TEST_CHECK(position.y == 36);

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 62U, 220U, OPENVN_ANCHOR_LEFT
    );
    OPENVN_TEST_CHECK(position.x == 18);
    OPENVN_TEST_CHECK(position.y == 36);

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 62U, 220U, OPENVN_ANCHOR_RIGHT
    );
    OPENVN_TEST_CHECK(position.x == 178);
    OPENVN_TEST_CHECK(position.y == 36);

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 999U, 999U, OPENVN_ANCHOR_CENTER
    );
    OPENVN_TEST_CHECK(position.x == 98);
    OPENVN_TEST_CHECK(position.y == 36);

    return 0;
}
