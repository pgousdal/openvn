#include "openvn_story.h"

#include <assert.h>
#include <string.h>

int main(int argc, char **argv) {
    OpenVNStory story;
    const OpenVNStoryNode *node;

    assert(argc == 2);
    assert(openvn_story_load_file(&story, argv[1]));
    assert(strcmp(story.version, "0.4") == 0);
    assert(openvn_story_start(&story));

    node = openvn_story_current(&story);
    assert(node != 0);
    assert(node->type == OPENVN_NODE_TEXT);
    assert(strcmp(node->text, "Hello from OpenVN.") == 0);

    assert(openvn_story_step(&story));
    node = openvn_story_current(&story);
    assert(node->type == OPENVN_NODE_CHOICE);
    assert(node->option_count == 2U);

    assert(openvn_story_choose(&story, 0U));
    node = openvn_story_current(&story);
    assert(node->type == OPENVN_NODE_SCENE);

    assert(openvn_story_step(&story));
    node = openvn_story_current(&story);
    assert(node->type == OPENVN_NODE_END);

    assert(openvn_story_step(&story));
    assert(story.ended);

    return 0;
}
