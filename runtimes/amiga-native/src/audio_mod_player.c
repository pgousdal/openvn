#include "openvn_mod_player.h"

#include <string.h>

static const unsigned short PERIODS[] = {
    1712U, 1616U, 1524U, 1440U, 1356U, 1280U,
    1208U, 1140U, 1076U, 1016U, 960U, 906U,
    856U, 808U, 762U, 720U, 678U, 640U,
    604U, 570U, 538U, 508U, 480U, 453U,
    428U, 404U, 381U, 360U, 339U, 320U,
    302U, 285U, 269U, 254U, 240U, 226U,
    214U, 202U, 190U, 180U, 170U, 160U,
    151U, 143U, 135U, 127U, 120U, 113U
};

#define PERIOD_COUNT (sizeof(PERIODS) / sizeof(PERIODS[0]))

static unsigned char clamp_volume(int volume) {
    if (volume < 0) {
        return 0U;
    }
    if (volume > 64) {
        return 64U;
    }
    return (unsigned char)volume;
}

static unsigned short transpose_period(
    unsigned short period,
    unsigned int semitones
) {
    unsigned int index;
    unsigned int nearest = 0U;
    unsigned long best = 65535UL;

    if (period == 0U) {
        return 0U;
    }

    for (index = 0U; index < PERIOD_COUNT; index++) {
        unsigned long difference;

        difference = PERIODS[index] > period
            ? (unsigned long)(PERIODS[index] - period)
            : (unsigned long)(period - PERIODS[index]);
        if (difference < best) {
            best = difference;
            nearest = index;
        }
    }

    nearest += semitones;
    if (nearest >= PERIOD_COUNT) {
        nearest = PERIOD_COUNT - 1U;
    }
    return PERIODS[nearest];
}

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

    memset(player->channels, 0, sizeof(player->channels));
    player->order = 0U;
    player->row = 0U;
    player->tick = 0U;
    player->speed = 6U;
    player->bpm = 125U;
    player->position_pending = 0;
    player->playing = 1;
    player->loop = loop;
    return 1;
}

void openvn_mod_player_stop(OpenVNMODPlayer *player) {
    if (player != 0) {
        player->playing = 0;
    }
}

static void schedule_position(
    OpenVNMODPlayer *player,
    unsigned int order,
    unsigned int row
) {
    player->next_order = order;
    player->next_row = row < OPENVN_MOD_ROWS ? row : 0U;
    player->position_pending = 1;
}

static void begin_row(OpenVNMODPlayer *player) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        const OpenVNMODNote *note;
        OpenVNMODChannelState *state;

        note = openvn_mod_player_channel(player, channel);
        state = &player->channels[channel];
        state->triggered = 0;
        if (note == 0) {
            continue;
        }

        state->effect = note->effect;
        state->parameter = note->parameter;
        if (note->sample > 0U && note->sample <= OPENVN_MOD_SAMPLE_COUNT) {
            state->sample = note->sample;
            state->volume = player->module.samples[note->sample - 1U].volume;
        }
        if (note->period > 0U) {
            state->base_period = note->period;
            state->period = note->period;
            state->triggered = 1;
        }

        switch (note->effect) {
            case 0x0BU:
                schedule_position(player, note->parameter, 0U);
                break;
            case 0x0CU:
                state->volume = clamp_volume(note->parameter);
                break;
            case 0x0DU:
                schedule_position(
                    player,
                    player->order + 1U,
                    (unsigned int)(note->parameter >> 4) * 10U +
                        (unsigned int)(note->parameter & 0x0FU)
                );
                break;
            case 0x0FU:
                if (note->parameter >= 32U) {
                    player->bpm = note->parameter;
                } else if (note->parameter > 0U) {
                    player->speed = note->parameter;
                }
                break;
            default:
                break;
        }
    }
}

static void apply_tick_effects(OpenVNMODPlayer *player) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        OpenVNMODChannelState *state = &player->channels[channel];
        unsigned int amount;

        state->triggered = 0;
        switch (state->effect) {
            case 0x00U:
                if (state->parameter != 0U) {
                    unsigned int step = player->tick % 3U;
                    unsigned int semitones = 0U;
                    if (step == 1U) {
                        semitones = state->parameter >> 4;
                    } else if (step == 2U) {
                        semitones = state->parameter & 0x0FU;
                    }
                    state->period = transpose_period(
                        state->base_period,
                        semitones
                    );
                }
                break;
            case 0x01U:
                amount = state->parameter;
                state->period = state->period > amount + 112U
                    ? (unsigned short)(state->period - amount)
                    : 113U;
                state->base_period = state->period;
                break;
            case 0x02U:
                amount = state->parameter;
                state->period = state->period + amount < 1712U
                    ? (unsigned short)(state->period + amount)
                    : 1712U;
                state->base_period = state->period;
                break;
            case 0x0AU:
                amount = state->parameter >> 4;
                if (amount > 0U) {
                    state->volume = clamp_volume(
                        (int)state->volume + (int)amount
                    );
                } else {
                    amount = state->parameter & 0x0FU;
                    state->volume = clamp_volume(
                        (int)state->volume - (int)amount
                    );
                }
                break;
            default:
                break;
        }
    }
}

static int normalize_position(OpenVNMODPlayer *player) {
    if (player->order < player->module.song_length) {
        return 1;
    }

    if (!player->loop) {
        player->playing = 0;
        return 0;
    }

    player->order = player->module.restart_position;
    if (player->order >= player->module.song_length) {
        player->order = 0U;
    }
    return 1;
}

int openvn_mod_player_tick(OpenVNMODPlayer *player) {
    if (player == 0 || !player->playing) {
        return 0;
    }

    if (player->tick == 0U) {
        begin_row(player);
    } else {
        apply_tick_effects(player);
    }

    player->tick++;
    if (player->tick < player->speed) {
        return 1;
    }

    player->tick = 0U;
    if (player->position_pending) {
        player->order = player->next_order;
        player->row = player->next_row;
        player->position_pending = 0;
        normalize_position(player);
        return 1;
    }

    player->row++;
    if (player->row < OPENVN_MOD_ROWS) {
        return 1;
    }

    player->row = 0U;
    player->order++;
    normalize_position(player);
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

const OpenVNMODChannelState *openvn_mod_player_channel_state(
    const OpenVNMODPlayer *player,
    unsigned int channel
) {
    if (player == 0 || channel >= OPENVN_MOD_CHANNELS) {
        return 0;
    }
    return &player->channels[channel];
}
