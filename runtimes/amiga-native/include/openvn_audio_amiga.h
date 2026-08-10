#ifndef OPENVN_AUDIO_AMIGA_H
#define OPENVN_AUDIO_AMIGA_H

#include "openvn_8svx.h"
#include "openvn_audio.h"
#include "openvn_mod_player.h"
#include "openvn_paula.h"

#ifdef __AMIGA__

struct MsgPort;
struct IOAudio;
struct timerequest;

typedef struct OpenVNAmigaAudioContext {
    struct MsgPort *channel_ports[OPENVN_MOD_CHANNELS];
    struct IOAudio *channel_requests[OPENVN_MOD_CHANNELS];
    struct IOAudio *loop_requests[OPENVN_MOD_CHANNELS];
    struct MsgPort *timer_port;
    struct timerequest *timer_request;
    const OpenVNAssetTable *assets;
    OpenVN8SVXSample sample;
    OpenVNMODPlayer mod_player;
    OpenVNPaulaMixer paula;
    unsigned char *music_host_data[OPENVN_MOD_SAMPLE_COUNT];
    unsigned char *music_chip_data[OPENVN_MOD_SAMPLE_COUNT];
    unsigned char *sound_chip_data;
    unsigned long sound_chip_size;
    unsigned char channel_masks[OPENVN_MOD_CHANNELS];
    unsigned long voice_generations[OPENVN_MOD_CHANNELS];
    int channel_pending[OPENVN_MOD_CHANNELS];
    int loop_pending[OPENVN_MOD_CHANNELS];
    int opened;
    int sound_pending;
    int timer_pending;
    unsigned int diagnostic_ticks;
    unsigned int diagnostic_voices;
} OpenVNAmigaAudioContext;

void openvn_audio_amiga_init(
    OpenVNAudioService *service,
    OpenVNAmigaAudioContext *context
);

#endif

#endif
