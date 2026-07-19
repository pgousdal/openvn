#include "openvn_dialogue.h"

#include "test_check.h"
#include <string.h>

int main(void) {
    OpenVNDialogueLayout layout;
    char output[128];

    openvn_dialogue_layout(320U, 256U, &layout);
    OPENVN_TEST_CHECK(layout.box_x == 8);
    OPENVN_TEST_CHECK(layout.box_y == 176);
    OPENVN_TEST_CHECK(layout.box_width == 304);
    OPENVN_TEST_CHECK(layout.box_height == 72);
    OPENVN_TEST_CHECK(layout.text_width == 288);

    OPENVN_TEST_CHECK(openvn_dialogue_wrap(
        "The old terminal is running one shared story.",
        20U,
        output,
        sizeof(output)
    ));
    OPENVN_TEST_CHECK(strcmp(
        output,
        "The old terminal is\nrunning one shared\nstory."
    ) == 0);

    OPENVN_TEST_CHECK(openvn_dialogue_wrap("abcdefgh", 4U, output, sizeof(output)));
    OPENVN_TEST_CHECK(strcmp(output, "abcd\nefgh") == 0);

    return 0;
}
