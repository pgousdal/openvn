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
    unsigned char channel_masks[OPENVN_MOD_CHANNELS];
    unsigned long voice_generations[OPENVN_MOD_CHANNELS];
    int channel_pending[OPENVN_MOD_CHANNELS];
    int loop_pending[OPENVN_MOD_CHANNELS];
    int opened;
    int sound_pending;
    int timer_pending;
} OpenVNAmigaAudioContext;

void openvn_audio_amiga_init(
    OpenVNAudioService *service,
    OpenVNAmigaAudioContext *context
);

#endif

#endif
