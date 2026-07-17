#include "openvn_story.h"

#include <string.h>

void openvn_story_reset(OpenVNStoryState *state) {
    if (state != 0) {
        state->story = 0;
        state->current_index = 0U;
        state->started = 0;
        state->ended = 0;
    }
}

int openvn_story_attach(
    OpenVNStoryState *state,
    const OpenVNGeneratedStory *story
) {
    if (state == 0 || story == 0 || story->nodes == 0 ||
        story->node_count == 0U || story->entry == 0) {
        return 0;
    }

    openvn_story_reset(state);
    state->story = story;
    return 1;
}

int openvn_story_find_node(
    const OpenVNGeneratedStory *story,
    const char *id
) {
    size_t index;

    if (story == 0 || id == 0) {
        return -1;
    }

    for (index = 0U; index < story->node_count; index++) {
        if (strcmp(story->nodes[index].id, id) == 0) {
            return (int)index;
        }
    }

    return -1;
}

const OpenVNGeneratedNode *openvn_story_current(
    const OpenVNStoryState *state
) {
    if (state == 0 || state->story == 0 ||
        state->current_index >= state->story->node_count) {
        return 0;
    }

    return &state->story->nodes[state->current_index];
}

int openvn_story_start(OpenVNStoryState *state) {
    int index;

    if (state == 0 || state->story == 0) {
        return 0;
    }

    index = openvn_story_find_node(state->story, state->story->entry);
    if (index < 0) {
        return 0;
    }

    state->current_index = (size_t)index;
    state->started = 1;
    state->ended = 0;
    return 1;
}

static int move_to(OpenVNStoryState *state, const char *id) {
    int index;

    if (id == 0 || id[0] == '\0') {
        state->ended = 1;
        return 1;
    }

    index = openvn_story_find_node(state->story, id);
    if (index < 0) {
        return 0;
    }

    state->current_index = (size_t)index;
    return 1;
}

int openvn_story_step(OpenVNStoryState *state) {
    const OpenVNGeneratedNode *node;

    node = openvn_story_current(state);
    if (node == 0 || state->ended) {
        return 0;
    }

    switch (node->type) {
        case OPENVN_NODE_TEXT:
        case OPENVN_NODE_SCENE:
        case OPENVN_NODE_SHOW:
        case OPENVN_NODE_HIDE:
        case OPENVN_NODE_MUSIC:
        case OPENVN_NODE_SOUND:
            return move_to(state, node->next);
        case OPENVN_NODE_JUMP:
            return move_to(state, node->target);
        case OPENVN_NODE_END:
            state->ended = 1;
            return 1;
        case OPENVN_NODE_CHOICE:
            return 0;
        default:
            return 0;
    }
}

int openvn_story_choose(OpenVNStoryState *state, size_t index) {
    const OpenVNGeneratedNode *node;

    node = openvn_story_current(state);
    if (node == 0 || node->type != OPENVN_NODE_CHOICE) {
        return 0;
    }

    if (index >= node->option_count) {
        return 0;
    }

    return move_to(state, node->options[index].target);
}

const char *openvn_story_status(const OpenVNStoryState *state) {
    const OpenVNGeneratedNode *node;

    if (state == 0 || state->story == 0) {
        return "UNLOADED";
    }

    if (state->ended) {
        return "ENDED";
    }

    if (!state->started) {
        return "READY";
    }

    node = openvn_story_current(state);
    if (node == 0) {
        return "INVALID";
    }

    switch (node->type) {
        case OPENVN_NODE_TEXT: return "TEXT";
        case OPENVN_NODE_CHOICE: return "CHOICE";
        case OPENVN_NODE_JUMP: return "JUMP";
        case OPENVN_NODE_END: return "END";
        case OPENVN_NODE_SCENE: return "SCENE";
        case OPENVN_NODE_SHOW: return "SHOW";
        case OPENVN_NODE_HIDE: return "HIDE";
        case OPENVN_NODE_MUSIC: return "MUSIC";
        case OPENVN_NODE_SOUND: return "SOUND";
        default: return "INVALID";
    }
}
