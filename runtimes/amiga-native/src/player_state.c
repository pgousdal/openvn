#include "openvn_dispatch.h"
#include "openvn_story.h"
#include "story.generated.h"

#include <stdlib.h>

static OpenVNStoryState g_story;

void openvn_state_reset(void) {
    openvn_story_attach(&g_story, &OPENVN_GENERATED_STORY);
}

const OpenVNStoryState *openvn_state_story(void) {
    return &g_story;
}

int openvn_dispatch_request(const OpenVNRequest *request) {
    char *end;
    long index;

    if (request == 0) {
        return 0;
    }

    switch (request->command) {
        case OPENVN_CMD_LOAD:
            return openvn_story_attach(&g_story, &OPENVN_GENERATED_STORY);
        case OPENVN_CMD_RUN:
            return openvn_story_start(&g_story);
        case OPENVN_CMD_STEP:
            return openvn_story_step(&g_story);
        case OPENVN_CMD_CHOOSE:
            index = strtol(request->argument1, &end, 10);
            if (*request->argument1 == '\0' || *end != '\0' || index < 0) {
                return 0;
            }
            return openvn_story_choose(&g_story, (size_t)index);
        case OPENVN_CMD_STATUS:
            return g_story.story != 0;
        case OPENVN_CMD_SCENE:
        case OPENVN_CMD_SHOW:
        case OPENVN_CMD_HIDE:
        case OPENVN_CMD_MUSIC:
        case OPENVN_CMD_SOUND:
            return 1;
        case OPENVN_CMD_QUIT:
            g_story.ended = 1;
            return 1;
        default:
            return 0;
    }
}
