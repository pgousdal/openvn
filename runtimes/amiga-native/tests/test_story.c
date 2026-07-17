#include "openvn_story.h"
#include "story.generated.h"

#include <assert.h>
#include <string.h>

int main(void) {
    OpenVNStoryState state;
    const OpenVNGeneratedNode *node;

    assert(openvn_story_attach(&state, &OPENVN_GENERATED_STORY));
    assert(strcmp(state.story->version, "0.4") == 0);
    assert(openvn_story_start(&state));

    node = openvn_story_current(&state);
    assert(node != 0);
    assert(node->type == OPENVN_NODE_TEXT);
    assert(strcmp(node->text, "Hello from OpenVN.") == 0);

    assert(openvn_story_step(&state));
    node = openvn_story_current(&state);
    assert(node->type == OPENVN_NODE_CHOICE);
    assert(node->option_count == 2U);

    assert(openvn_story_choose(&state, 0U));
    node = openvn_story_current(&state);
    assert(node->type == OPENVN_NODE_SCENE);

    assert(openvn_story_step(&state));
    node = openvn_story_current(&state);
    assert(node->type == OPENVN_NODE_END);

    assert(openvn_story_step(&state));
    assert(state.ended);

    return 0;
}
