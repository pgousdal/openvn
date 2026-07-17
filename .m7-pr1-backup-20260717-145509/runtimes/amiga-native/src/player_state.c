#include "assets.generated.h"

#include "openvn_dispatch.h"
#if defined(__amigaos__) || defined(__AMIGA__) || defined(AMIGA)
#include "openvn_audio_amiga.h"
#include "openvn_graphics_amiga.h"
#define openvn_platform_audio_init openvn_audio_amiga_init
#define openvn_platform_graphics_init openvn_graphics_amiga_init
typedef OpenVNAmigaGraphicsContext OpenVNPlatformGraphicsContext;
typedef OpenVNAmigaAudioContext OpenVNPlatformAudioContext;
#else
#include "openvn_audio_host.h"
#include "openvn_graphics_host.h"
#define openvn_platform_audio_init openvn_audio_host_init
#define openvn_platform_graphics_init openvn_graphics_host_init
typedef OpenVNHostGraphicsContext OpenVNPlatformGraphicsContext;
typedef OpenVNHostAudioContext OpenVNPlatformAudioContext;
#endif
#include "openvn_player.h"

#include <stdlib.h>

static OpenVNPlayer g_player;

static OpenVNGraphicsService g_graphics;
static OpenVNPlatformGraphicsContext g_graphics_context;

static OpenVNAudioService g_audio;
static OpenVNPlatformAudioContext g_audio_context;

static int g_initialized;

static int ensure_initialized(void) {
    OpenVNGraphicsConfig graphics_config;
    OpenVNAudioConfig audio_config;

    if (g_initialized) {
        return 1;
    }

    openvn_platform_graphics_init(&g_graphics, &g_graphics_context);
    openvn_platform_audio_init(&g_audio, &g_audio_context);

    graphics_config.width = 640U;
    graphics_config.height = 256U;
    graphics_config.depth = 8U;
    graphics_config.use_datatypes = 1;
    graphics_config.fullscreen = 0;
    graphics_config.assets = &OPENVN_GENERATED_ASSETS;

    audio_config.sample_rate = 8000U;
    audio_config.channels = 4U;
    audio_config.assets = &OPENVN_GENERATED_ASSETS;

    if (!openvn_graphics_open(&g_graphics, &graphics_config)) {
        return 0;
    }

    if (!openvn_audio_open(&g_audio, &audio_config)) {
        openvn_graphics_close(&g_graphics);
        return 0;
    }

    openvn_player_init(&g_player, &g_graphics, &g_audio);
    g_initialized = 1;
    return 1;
}

void openvn_state_reset(void) {
    if (g_initialized) {
        openvn_audio_close(&g_audio);
        openvn_graphics_close(&g_graphics);
        g_initialized = 0;
    }

    ensure_initialized();
}


int openvn_state_update(void) {
    if (!ensure_initialized()) {
        return 0;
    }
    return openvn_player_update(&g_player);
}

unsigned long openvn_state_signal_mask(void) {
    if (!ensure_initialized()) {
        return 0UL;
    }
    return openvn_player_signal_mask(&g_player);
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
            openvn_player_init(&g_player, &g_graphics, &g_audio);
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
            if (request->argument1[0] == '\0' ||
                request->argument1[0] == '-') {
                return openvn_audio_stop_music(&g_audio);
            }
            return openvn_audio_music(&g_audio, request->argument1);
        case OPENVN_CMD_SOUND:
            return openvn_audio_sound(&g_audio, request->argument1);
        case OPENVN_CMD_QUIT:
            g_player.story.ended = 1;
            openvn_audio_close(&g_audio);
            openvn_graphics_close(&g_graphics);
            g_initialized = 0;
            return 1;
        default:
            return 0;
    }
}
