#ifndef OPENVN_MOD_PLAYER_H
#define OPENVN_MOD_PLAYER_H

#include "openvn_mod.h"

typedef struct OpenVNMODChannelState {
    unsigned short base_period;
    unsigned short period;
    unsigned char sample;
    unsigned char volume;
    unsigned char effect;
    unsigned char parameter;
    int triggered;
} OpenVNMODChannelState;

typedef struct OpenVNMODPlayer {
    OpenVNMODModule module;
    OpenVNMODChannelState channels[OPENVN_MOD_CHANNELS];
    unsigned int order;
    unsigned int row;
    unsigned int tick;
    unsigned int speed;
    unsigned int bpm;
    unsigned int next_order;
    unsigned int next_row;
    int position_pending;
    int playing;
    int loop;
} OpenVNMODPlayer;

void openvn_mod_player_reset(OpenVNMODPlayer *player);
void openvn_mod_player_free(OpenVNMODPlayer *player);

int openvn_mod_player_load(
    OpenVNMODPlayer *player,
    const char *path
);

int openvn_mod_player_start(OpenVNMODPlayer *player, int loop);
void openvn_mod_player_stop(OpenVNMODPlayer *player);
int openvn_mod_player_tick(OpenVNMODPlayer *player);

const OpenVNMODNote *openvn_mod_player_channel(
    const OpenVNMODPlayer *player,
    unsigned int channel
);

const OpenVNMODChannelState *openvn_mod_player_channel_state(
    const OpenVNMODPlayer *player,
    unsigned int channel
);

#endif
