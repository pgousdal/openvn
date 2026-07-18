#ifndef OPENVN_DIALOGUE_H
#define OPENVN_DIALOGUE_H

#include <stddef.h>

#define OPENVN_DIALOGUE_MAX_TEXT 512U
#define OPENVN_DIALOGUE_MAX_WRAPPED 640U

typedef struct OpenVNDialogueLayout {
    int box_x;
    int box_y;
    int box_width;
    int box_height;
    int text_x;
    int text_y;
    int text_width;
    int text_height;
} OpenVNDialogueLayout;

void openvn_dialogue_layout(
    unsigned int screen_width,
    unsigned int screen_height,
    OpenVNDialogueLayout *layout
);

int openvn_dialogue_wrap(
    const char *text,
    unsigned int max_columns,
    char *output,
    size_t output_size
);

#endif
