#include "story.generated.h"

static const OpenVNGeneratedChoice openvn_options_1[] = {
    {"Continue", "scene"},
    {"Finish", "end"}
};

static const OpenVNGeneratedNode openvn_nodes[] = {
    {
        "start",
        OPENVN_NODE_TEXT,
        "Hello from OpenVN.",
        "choice",
        "",
        "",
        "",
        0,
        0U
    },
    {
        "choice",
        OPENVN_NODE_CHOICE,
        "",
        "",
        "",
        "",
        "",
        openvn_options_1,
        2U
    },
    {
        "scene",
        OPENVN_NODE_SCENE,
        "",
        "end",
        "",
        "lighthouse_storm",
        "",
        0,
        0U
    },
    {
        "end",
        OPENVN_NODE_END,
        "",
        "",
        "",
        "",
        "",
        0,
        0U
    }
};

const OpenVNGeneratedStory OPENVN_GENERATED_STORY = {
    "0.4",
    "start",
    openvn_nodes,
    4U
};
