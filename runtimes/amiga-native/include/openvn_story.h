#ifndef OPENVN_STORY_H
#define OPENVN_STORY_H

#include <stddef.h>

#define OPENVN_MAX_NODES 512
#define OPENVN_MAX_OPTIONS 16
#define OPENVN_MAX_TEXT 1024
#define OPENVN_MAX_ID 128

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
    OPENVN_NODE_SOUND
} OpenVNNodeType;

typedef struct OpenVNChoiceOption {
    char text[OPENVN_MAX_TEXT];
    char target[OPENVN_MAX_ID];
} OpenVNChoiceOption;

typedef struct OpenVNStoryNode {
    char id[OPENVN_MAX_ID];
    OpenVNNodeType type;
    char text[OPENVN_MAX_TEXT];
    char next[OPENVN_MAX_ID];
    char target[OPENVN_MAX_ID];
    char argument1[OPENVN_MAX_ID];
    char argument2[OPENVN_MAX_ID];
    OpenVNChoiceOption options[OPENVN_MAX_OPTIONS];
    size_t option_count;
} OpenVNStoryNode;

typedef struct OpenVNStory {
    char version[16];
    char entry[OPENVN_MAX_ID];
    OpenVNStoryNode nodes[OPENVN_MAX_NODES];
    size_t node_count;
    size_t current_index;
    int loaded;
    int ended;
} OpenVNStory;

void openvn_story_reset(OpenVNStory *story);
int openvn_story_load_file(OpenVNStory *story, const char *path);
int openvn_story_find_node(const OpenVNStory *story, const char *id);
const OpenVNStoryNode *openvn_story_current(const OpenVNStory *story);
int openvn_story_start(OpenVNStory *story);
int openvn_story_step(OpenVNStory *story);
int openvn_story_choose(OpenVNStory *story, size_t index);
const char *openvn_story_status(const OpenVNStory *story);

#endif
