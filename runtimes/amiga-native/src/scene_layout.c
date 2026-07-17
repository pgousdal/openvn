#include "openvn_scene_layout.h"

static int anchor_x(
    unsigned int screen_width,
    OpenVNCharacterAnchor anchor
) {
    if (anchor == OPENVN_ANCHOR_LEFT) {
        return (int)(screen_width / 4U);
    }
    if (anchor == OPENVN_ANCHOR_RIGHT) {
        return (int)((screen_width * 3U) / 4U);
    }
    return (int)(screen_width / 2U);
}

OpenVNScenePosition openvn_scene_character_position(
    unsigned int screen_width,
    unsigned int screen_height,
    unsigned int character_width,
    unsigned int character_height,
    unsigned int origin_x,
    unsigned int origin_y,
    OpenVNCharacterAnchor anchor
) {
    OpenVNScenePosition position;
    unsigned int effective_origin_x;
    unsigned int effective_origin_y;

    effective_origin_x = origin_x;
    if (effective_origin_x > character_width) {
        effective_origin_x = character_width / 2U;
    }

    effective_origin_y = origin_y;
    if (effective_origin_y > character_height) {
        effective_origin_y = character_height;
    }

    position.x = anchor_x(screen_width, anchor) - (int)effective_origin_x;
    position.y = (int)screen_height - (int)effective_origin_y;
    return position;
}
