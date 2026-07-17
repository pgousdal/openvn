#ifndef OPENVN_SCENE_LAYOUT_H
#define OPENVN_SCENE_LAYOUT_H

typedef enum OpenVNCharacterAnchor {
    OPENVN_ANCHOR_LEFT = 0,
    OPENVN_ANCHOR_CENTER = 1,
    OPENVN_ANCHOR_RIGHT = 2
} OpenVNCharacterAnchor;

typedef struct OpenVNScenePosition {
    int x;
    int y;
} OpenVNScenePosition;

OpenVNScenePosition openvn_scene_character_position(
    unsigned int screen_width,
    unsigned int screen_height,
    unsigned int character_width,
    unsigned int character_height,
    unsigned int origin_x,
    unsigned int origin_y,
    OpenVNCharacterAnchor anchor
);

#endif
