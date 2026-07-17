#include "openvn_mod_player.h"

#include <string.h>

void openvn_mod_player_reset(OpenVNMODPlayer *player) {
    if (player != 0) {
        memset(player, 0, sizeof(*player));
        player->speed = 6U;
        player->bpm = 125U;
    }
}

void openvn_mod_player_free(OpenVNMODPlayer *player) {
    if (player == 0) {
        return;
    }

    openvn_mod_free(&player->module);
    openvn_mod_player_reset(player);
}

int openvn_mod_player_load(
    OpenVNMODPlayer *player,
    const char *path
) {
    if (player == 0 || path == 0) {
        return 0;
    }

    openvn_mod_player_free(player);
    return openvn_mod_load_file(&player->module, path);
}

int openvn_mod_player_start(OpenVNMODPlayer *player, int loop) {
    if (player == 0 || player->module.patterns == 0) {
        return 0;
    }

    player->order = 0U;
    player->row = 0U;
    player->tick = 0U;
    player->speed = 6U;
    player->bpm = 125U;
    player->playing = 1;
    player->loop = loop;
    return 1;
}

void openvn_mod_player_stop(OpenVNMODPlayer *player) {
    if (player != 0) {
        player->playing = 0;
    }
}

static void apply_row_effects(OpenVNMODPlayer *player) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        const OpenVNMODNote *note = openvn_mod_player_channel(
            player,
            channel
        );

        if (note == 0) {
            continue;
        }

        if (note->effect == 0x0FU) {
            if (note->parameter >= 32U) {
                player->bpm = note->parameter;
            } else if (note->parameter > 0U) {
                player->speed = note->parameter;
            }
        }
    }
}

int openvn_mod_player_tick(OpenVNMODPlayer *player) {
    unsigned int next_order;

    if (player == 0 || !player->playing) {
        return 0;
    }

    if (player->tick == 0U) {
        apply_row_effects(player);
    }

    player->tick++;
    if (player->tick < player->speed) {
        return 1;
    }

    player->tick = 0U;
    player->row++;

    if (player->row < OPENVN_MOD_ROWS) {
        return 1;
    }

    player->row = 0U;
    next_order = player->order + 1U;

    if (next_order >= player->module.song_length) {
        if (!player->loop) {
            player->playing = 0;
            return 1;
        }
        next_order = player->module.restart_position;
        if (next_order >= player->module.song_length) {
            next_order = 0U;
        }
    }

    player->order = next_order;
    return 1;
}

const OpenVNMODNote *openvn_mod_player_channel(
    const OpenVNMODPlayer *player,
    unsigned int channel
) {
    unsigned int pattern;

    if (player == 0 || player->module.song_length == 0U ||
        player->order >= player->module.song_length) {
        return 0;
    }

    pattern = player->module.pattern_table[player->order];
    return openvn_mod_note(
        &player->module,
        pattern,
        player->row,
        channel
    );
}
