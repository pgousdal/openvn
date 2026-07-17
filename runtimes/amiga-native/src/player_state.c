#include "assets.generated.h"
#include "openvn_dispatch.h"
#include "openvn_graphics_host.h"
#include "openvn_player.h"

#include <stdlib.h>

static OpenVNPlayer g_player;
static OpenVNGraphicsService g_graphics;
static OpenVNHostGraphicsContext g_graphics_context;
static int g_initialized;

static int ensure_initialized(void) {
    OpenVNGraphicsConfig config;

    if (g_initialized) {
        return 1;
    }

    openvn_graphics_host_init(&g_graphics, &g_graphics_context);

    config.width = 640U;
    config.height = 256U;
    config.depth = 8U;
    config.use_datatypes = 1;
    config.fullscreen = 0;
    config.assets = &OPENVN_GENERATED_ASSETS;

    if (!openvn_graphics_open(&g_graphics, &config)) {
        return 0;
    }

    openvn_player_init(&g_player, &g_graphics);
    g_initialized = 1;
    return 1;
}

void openvn_state_reset(void) {
    if (g_initialized) {
        openvn_graphics_close(&g_graphics);
        g_initialized = 0;
    }

    ensure_initialized();
}

const OpenVNStoryState *openvn_state_story(void) {
    return &g_player.story;
}

int openvn_dispatch_request(const OpenVNRequest *request) {
    char *end;
    long index;

    if (request == 0 || !ensure_initialized()) {
        return 0;
    }

    switch (request->command) {
        case OPENVN_CMD_LOAD:
            openvn_player_init(&g_player, &g_graphics);
            return 1;
        case OPENVN_CMD_RUN:
            return openvn_player_start(&g_player);
        case OPENVN_CMD_STEP:
            return openvn_player_step(&g_player);
        case OPENVN_CMD_CHOOSE:
            index = strtol(request->argument1, &end, 10);
            if (*request->argument1 == '\0' || *end != '\0' || index < 0) {
                return 0;
            }
            return openvn_player_choose(&g_player, (size_t)index);
        case OPENVN_CMD_STATUS:
            return g_player.story.story != 0;
        case OPENVN_CMD_SCENE:
            return openvn_graphics_scene(&g_graphics, request->argument1) &&
                   openvn_graphics_present(&g_graphics);
        case OPENVN_CMD_SHOW:
            return openvn_graphics_show(
                       &g_graphics,
                       request->argument1,
                       request->argument2
                   ) &&
                   openvn_graphics_present(&g_graphics);
        case OPENVN_CMD_HIDE:
            return openvn_graphics_hide(&g_graphics, request->argument1) &&
                   openvn_graphics_present(&g_graphics);
        case OPENVN_CMD_MUSIC:
        case OPENVN_CMD_SOUND:
            return 1;
        case OPENVN_CMD_QUIT:
            g_player.story.ended = 1;
            openvn_graphics_close(&g_graphics);
            g_initialized = 0;
            return 1;
        default:
            return 0;
    }
}
