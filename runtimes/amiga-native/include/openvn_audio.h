#ifndef OPENVN_AUDIO_H
#define OPENVN_AUDIO_H

#include <stddef.h>

#include "openvn_assets_runtime.h"

typedef struct OpenVNAudioConfig {
    unsigned int sample_rate;
    unsigned int channels;
    const OpenVNAssetTable *assets;
} OpenVNAudioConfig;

typedef struct OpenVNAudioService OpenVNAudioService;

typedef struct OpenVNAudioVTable {
    int (*open)(
        OpenVNAudioService *service,
        const OpenVNAudioConfig *config
    );
    void (*close)(OpenVNAudioService *service);
    int (*music)(
        OpenVNAudioService *service,
        const char *track
    );
    int (*sound)(
        OpenVNAudioService *service,
        const char *effect
    );
    int (*stop_music)(OpenVNAudioService *service);
    int (*update)(OpenVNAudioService *service);
    unsigned long (*signal_mask)(OpenVNAudioService *service);
} OpenVNAudioVTable;

struct OpenVNAudioService {
    const OpenVNAudioVTable *vtable;
    void *context;
};

int openvn_audio_open(
    OpenVNAudioService *service,
    const OpenVNAudioConfig *config
);
void openvn_audio_close(OpenVNAudioService *service);
int openvn_audio_music(
    OpenVNAudioService *service,
    const char *track
);
int openvn_audio_sound(
    OpenVNAudioService *service,
    const char *effect
);
int openvn_audio_stop_music(OpenVNAudioService *service);
int openvn_audio_update(OpenVNAudioService *service);
unsigned long openvn_audio_signal_mask(OpenVNAudioService *service);

#endif
