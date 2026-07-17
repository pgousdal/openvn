#ifndef OPENVN_MOD_PLAYER_H
#define OPENVN_MOD_PLAYER_H

#include "openvn_mod.h"

typedef struct OpenVNMODPlayer {
    OpenVNMODModule module;
    unsigned int order;
    unsigned int row;
    unsigned int tick;
    unsigned int speed;
    unsigned int bpm;
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

#endif
