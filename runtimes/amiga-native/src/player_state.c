#include "openvn_dispatch.h"

#include <stdio.h>
#include <string.h>

typedef struct OpenVNPlayerState {
    int running;
    char story_path[256];
    char scene[256];
    char character[256];
    char pose[256];
    char music[256];
    char sound[256];
} OpenVNPlayerState;

static OpenVNPlayerState g_state;

void openvn_state_reset(void) {
    memset(&g_state, 0, sizeof(g_state));
}

int openvn_dispatch_request(const OpenVNRequest *request) {
    if (request == NULL) {
        return 0;
    }

    switch (request->command) {
        case OPENVN_CMD_LOAD:
            snprintf(g_state.story_path, sizeof(g_state.story_path), "%s", request->argument1);
            return 1;
        case OPENVN_CMD_RUN:
            g_state.running = 1;
            return 1;
        case OPENVN_CMD_SCENE:
            snprintf(g_state.scene, sizeof(g_state.scene), "%s", request->argument1);
            return 1;
        case OPENVN_CMD_SHOW:
            snprintf(g_state.character, sizeof(g_state.character), "%s", request->argument1);
            snprintf(g_state.pose, sizeof(g_state.pose), "%s", request->argument2);
            return 1;
        case OPENVN_CMD_HIDE:
            if (strcmp(g_state.character, request->argument1) == 0) {
                g_state.character[0] = '\0';
                g_state.pose[0] = '\0';
            }
            return 1;
        case OPENVN_CMD_MUSIC:
            snprintf(g_state.music, sizeof(g_state.music), "%s", request->argument1);
            return 1;
        case OPENVN_CMD_SOUND:
            snprintf(g_state.sound, sizeof(g_state.sound), "%s", request->argument1);
            return 1;
        case OPENVN_CMD_QUIT:
            g_state.running = 0;
            return 1;
        default:
            return 0;
    }
}
