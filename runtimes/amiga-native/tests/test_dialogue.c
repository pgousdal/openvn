#include "openvn_dialogue.h"

#include <assert.h>
#include <string.h>

int main(void) {
    OpenVNDialogueLayout layout;
    char output[128];

    openvn_dialogue_layout(320U, 256U, &layout);
    assert(layout.box_x == 8);
    assert(layout.box_y == 176);
    assert(layout.box_width == 304);
    assert(layout.box_height == 72);
    assert(layout.text_width == 288);

    assert(openvn_dialogue_wrap(
        "The old terminal is running one shared story.",
        20U,
        output,
        sizeof(output)
    ));
    assert(strcmp(
        output,
        "The old terminal is\nrunning one shared\nstory."
    ) == 0);

    assert(openvn_dialogue_wrap("abcdefgh", 4U, output, sizeof(output)));
    assert(strcmp(output, "abcd\nefgh") == 0);

    return 0;
}
