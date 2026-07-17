#ifndef OPENVN_AUDIO_AMIGA_H
#define OPENVN_AUDIO_AMIGA_H

#include "openvn_8svx.h"
#include "openvn_audio.h"
#include "openvn_mod_player.h"

#ifdef __AMIGA__

struct MsgPort;
struct IOAudio;
struct timerequest;

typedef struct OpenVNAmigaAudioContext {
    struct MsgPort *port;
    struct IOAudio *request;
    struct MsgPort *timer_port;
    struct timerequest *timer_request;
    const OpenVNAssetTable *assets;
    OpenVN8SVXSample sample;
    OpenVNMODPlayer mod_player;
    unsigned char channels[4];
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
