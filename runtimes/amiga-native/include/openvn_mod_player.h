#ifndef OPENVN_MOD_PLAYER_H
#define OPENVN_MOD_PLAYER_H

#include "openvn_mod.h"

typedef struct OpenVNMODChannelState {
    unsigned short base_period;
    unsigned short period;
    unsigned short target_period;
    unsigned short porta_speed;
    unsigned char sample;
    unsigned char pending_sample;
    unsigned char volume;
    unsigned char effect;
    unsigned char parameter;
    unsigned char vibrato_position;
    unsigned char vibrato_speed;
    unsigned char vibrato_depth;
    unsigned char tremolo_position;
    unsigned char tremolo_speed;
    unsigned char tremolo_depth;
    unsigned char note_delay_tick;
    unsigned char note_cut_tick;
    unsigned char retrigger_interval;
    unsigned int sample_offset;
    unsigned int pattern_loop_row;
    unsigned int pattern_loop_count;
    int note_delayed;
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
    unsigned int pattern_delay_rows;
    int position_pending;
    int playing;
    int loop;
} OpenVNMODPlayer;

void openvn_mod_player_reset(OpenVNMODPlayer *player);
void openvn_mod_player_free(OpenVNMODPlayer *player);
int openvn_mod_player_load(OpenVNMODPlayer *player, const char *path);
int openvn_mod_player_start(OpenVNMODPlayer *player, int loop);
void openvn_mod_player_stop(OpenVNMODPlayer *player);
int openvn_mod_player_tick(OpenVNMODPlayer *player);
const OpenVNMODNote *openvn_mod_player_channel(const OpenVNMODPlayer *player, unsigned int channel);
const OpenVNMODChannelState *openvn_mod_player_channel_state(const OpenVNMODPlayer *player, unsigned int channel);

#endif
