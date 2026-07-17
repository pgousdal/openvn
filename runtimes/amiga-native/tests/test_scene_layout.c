#include "openvn_scene_layout.h"

#include <assert.h>

int main(void) {
    OpenVNScenePosition position;

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 62U, 220U, OPENVN_ANCHOR_CENTER
    );
    assert(position.x == 98);
    assert(position.y == 36);

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 62U, 220U, OPENVN_ANCHOR_LEFT
    );
    assert(position.x == 18);
    assert(position.y == 36);

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 62U, 220U, OPENVN_ANCHOR_RIGHT
    );
    assert(position.x == 178);
    assert(position.y == 36);

    position = openvn_scene_character_position(
        320U, 256U, 124U, 220U, 999U, 999U, OPENVN_ANCHOR_CENTER
    );
    assert(position.x == 98);
    assert(position.y == 36);

    return 0;
}
