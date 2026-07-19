#ifndef OPENVN_STORY_H
#define OPENVN_STORY_H

#include <stddef.h>

#include "openvn_condition.h"

typedef enum OpenVNNodeType {
    OPENVN_NODE_INVALID = 0,
    OPENVN_NODE_TEXT,
    OPENVN_NODE_CHOICE,
    OPENVN_NODE_JUMP,
    OPENVN_NODE_END,
    OPENVN_NODE_SCENE,
    OPENVN_NODE_SHOW,
    OPENVN_NODE_HIDE,
    OPENVN_NODE_MUSIC,
    OPENVN_NODE_SOUND,
    OPENVN_NODE_SET_BOOL,
    OPENVN_NODE_SET_INT,
    OPENVN_NODE_SET_STRING,
    OPENVN_NODE_CONDITION
} OpenVNNodeType;

typedef struct OpenVNGeneratedChoice {
    const char *text;
    const char *target;
} OpenVNGeneratedChoice;

typedef struct OpenVNGeneratedNode {
    const char *id;
    OpenVNNodeType type;
    const char *text;
    const char *next;
    const char *target;
    const char *argument1;
    const char *argument2;
    const OpenVNGeneratedChoice *options;
    size_t option_count;
    OpenVNCondition condition;
    const char *true_target;
    const char *false_target;
} OpenVNGeneratedNode;

typedef struct OpenVNGeneratedStory {
    const char *version;
    const char *entry;
    const OpenVNGeneratedNode *nodes;
    size_t node_count;
} OpenVNGeneratedStory;

typedef struct OpenVNStoryState {
    const OpenVNGeneratedStory *story;
    size_t current_index;
    int started;
    int ended;
} OpenVNStoryState;

void openvn_story_reset(OpenVNStoryState *state);
int openvn_story_attach(
    OpenVNStoryState *state,
    const OpenVNGeneratedStory *story
);
int openvn_story_find_node(
    const OpenVNGeneratedStory *story,
    const char *id
);
const OpenVNGeneratedNode *openvn_story_current(
    const OpenVNStoryState *state
);
int openvn_story_start(OpenVNStoryState *state);
int openvn_story_step(OpenVNStoryState *state);
int openvn_story_choose(OpenVNStoryState *state, size_t index);
int openvn_story_branch(OpenVNStoryState *state, int result);
const char *openvn_story_status(const OpenVNStoryState *state);

#endif
