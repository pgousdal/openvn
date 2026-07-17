#ifndef OPENVN_AUDIO_HOST_H
#define OPENVN_AUDIO_HOST_H

#include "openvn_8svx.h"
#include "openvn_audio.h"
#include "openvn_mod_player.h"

typedef struct OpenVNHostAudioContext {
    int opened;
    const OpenVNAssetTable *assets;
    char music[256];
    char sound[256];
    char music_path[512];
    char sound_path[512];
    OpenVN8SVXSample sample;
    OpenVNMODPlayer mod_player;
    unsigned int update_count;
} OpenVNHostAudioContext;

void openvn_audio_host_init(
    OpenVNAudioService *service,
    OpenVNHostAudioContext *context
);

#endif
