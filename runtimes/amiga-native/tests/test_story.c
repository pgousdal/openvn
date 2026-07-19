#include "openvn_story.h"
#include "story.generated.h"

#include "test_check.h"
#include <string.h>

int main(void) {
    OpenVNStoryState state;
    const OpenVNGeneratedNode *node;

    OPENVN_TEST_CHECK(openvn_story_attach(&state, &OPENVN_GENERATED_STORY));
    OPENVN_TEST_CHECK(strcmp(state.story->version, "0.4") == 0);
    OPENVN_TEST_CHECK(openvn_story_start(&state));

    node = openvn_story_current(&state);
    OPENVN_TEST_CHECK(node != 0);
    OPENVN_TEST_CHECK(node->type == OPENVN_NODE_TEXT);
    OPENVN_TEST_CHECK(strcmp(node->text, "Hello from OpenVN.") == 0);

    OPENVN_TEST_CHECK(openvn_story_step(&state));
    node = openvn_story_current(&state);
    OPENVN_TEST_CHECK(node->type == OPENVN_NODE_CHOICE);
    OPENVN_TEST_CHECK(node->option_count == 2U);

    OPENVN_TEST_CHECK(openvn_story_choose(&state, 0U));
    node = openvn_story_current(&state);
    OPENVN_TEST_CHECK(node->type == OPENVN_NODE_SCENE);

    OPENVN_TEST_CHECK(openvn_story_step(&state));
    node = openvn_story_current(&state);
    OPENVN_TEST_CHECK(node->type == OPENVN_NODE_END);

    OPENVN_TEST_CHECK(openvn_story_step(&state));
    OPENVN_TEST_CHECK(state.ended);

    return 0;
}
