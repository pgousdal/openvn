#include "openvn_audio_host.h"

#include <stdio.h>
#include <string.h>

static int host_open(
    OpenVNAudioService *service,
    const OpenVNAudioConfig *config
) {
    OpenVNHostAudioContext *context;

    context = (OpenVNHostAudioContext *)service->context;
    if (context == 0) {
        return 0;
    }

    memset(context, 0, sizeof(*context));
    context->opened = 1;
    context->assets = config->assets;
    openvn_8svx_reset(&context->sample);
    openvn_mod_player_reset(&context->mod_player);

    printf(
        "AUDIO OPEN %u %u\n",
        config->sample_rate,
        config->channels
    );
    return 1;
}

static void host_close(OpenVNAudioService *service) {
    OpenVNHostAudioContext *context;

    context = (OpenVNHostAudioContext *)service->context;
    if (context != 0 && context->opened) {
        openvn_8svx_free(&context->sample);
        openvn_mod_player_free(&context->mod_player);
        puts("AUDIO CLOSE");
        context->opened = 0;
    }
}

static int host_music(
    OpenVNAudioService *service,
    const char *track
) {
    OpenVNHostAudioContext *context;
    const char *path;

    context = (OpenVNHostAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_music(context->assets, track);
    if (path == 0) {
        return 0;
    }

    if (!openvn_mod_player_load(&context->mod_player, path) ||
        !openvn_mod_player_start(&context->mod_player, 1)) {
        return 0;
    }

    snprintf(context->music, sizeof(context->music), "%s", track);
    snprintf(context->music_path, sizeof(context->music_path), "%s", path);
    printf(
        "AUDIO MUSIC %s %s %u\n",
        track,
        path,
        context->mod_player.module.pattern_count
    );
    return 1;
}

static int host_sound(
    OpenVNAudioService *service,
    const char *effect
) {
    OpenVNHostAudioContext *context;
    const char *path;

    context = (OpenVNHostAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_sound(context->assets, effect);
    if (path == 0) {
        return 0;
    }

    openvn_8svx_free(&context->sample);
    if (!openvn_8svx_load_file(&context->sample, path)) {
        return 0;
    }

    snprintf(context->sound, sizeof(context->sound), "%s", effect);
    snprintf(context->sound_path, sizeof(context->sound_path), "%s", path);

    printf(
        "AUDIO SOUND %s %s %u %lu\n",
        effect,
        path,
        context->sample.sample_rate,
        (unsigned long)context->sample.data_size
    );
    return 1;
}

static int host_stop_music(OpenVNAudioService *service) {
    OpenVNHostAudioContext *context;

    context = (OpenVNHostAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    openvn_mod_player_stop(&context->mod_player);
    context->music[0] = '\0';
    context->music_path[0] = '\0';
    puts("AUDIO MUSIC STOP");
    return 1;
}

static int host_update(OpenVNAudioService *service) {
    OpenVNHostAudioContext *context;

    context = (OpenVNHostAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    if (context->mod_player.playing) {
        openvn_mod_player_tick(&context->mod_player);
    }

    context->update_count++;
    return 1;
}

static unsigned long host_signal_mask(OpenVNAudioService *service) {
    (void)service;
    return 0UL;
}

static const OpenVNAudioVTable HOST_VTABLE = {
    host_open,
    host_close,
    host_music,
    host_sound,
    host_stop_music,
    host_update,
    host_signal_mask
};

void openvn_audio_host_init(
    OpenVNAudioService *service,
    OpenVNHostAudioContext *context
) {
    if (service != 0) {
        service->vtable = &HOST_VTABLE;
        service->context = context;
    }
}
