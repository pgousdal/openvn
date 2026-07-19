#include "openvn_mod_player.h"

#include "test_check.h"
#include <stdlib.h>
#include <string.h>

static OpenVNMODNote *note_at(
    OpenVNMODPlayer *player,
    unsigned int pattern,
    unsigned int row,
    unsigned int channel
) {
    return &player->module.patterns[
        (pattern * OPENVN_MOD_ROWS + row) * OPENVN_MOD_CHANNELS + channel
    ];
}

static void prepare(OpenVNMODPlayer *player) {
    openvn_mod_player_reset(player);
    player->module.song_length = 2U;
    player->module.pattern_count = 2U;
    player->module.pattern_table[0] = 0U;
    player->module.pattern_table[1] = 1U;
    player->module.samples[0].volume = 32U;
    player->module.patterns = (OpenVNMODNote *)calloc(
        2U * OPENVN_MOD_ROWS * OPENVN_MOD_CHANNELS,
        sizeof(OpenVNMODNote)
    );
    OPENVN_TEST_CHECK(player->module.patterns != 0);
}

static void run_ticks(OpenVNMODPlayer *player, unsigned int count) {
    unsigned int tick;
    for (tick = 0U; tick < count; tick++) {
        OPENVN_TEST_CHECK(openvn_mod_player_tick(player));
    }
}

int main(void) {
    OpenVNMODPlayer player = {0};
    OpenVNMODNote *note;
    const OpenVNMODChannelState *state;

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U;
    note->period = 428U;
    note->effect = 0x00U;
    note->parameter = 0x37U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 1U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->period == 428U);
    run_ticks(&player, 1U);
    OPENVN_TEST_CHECK(state->period == 360U);
    run_ticks(&player, 1U);
    OPENVN_TEST_CHECK(state->period == 285U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U;
    note->period = 428U;
    note->effect = 0x01U;
    note->parameter = 4U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 2U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->period == 424U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U;
    note->period = 428U;
    note->effect = 0x02U;
    note->parameter = 5U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 2U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->period == 433U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U;
    note->period = 428U;
    note->effect = 0x0AU;
    note->parameter = 0x03U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 2U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->volume == 29U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U;
    note->period = 428U;
    note->effect = 0x0CU;
    note->parameter = 80U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 1U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->volume == 64U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->effect = 0x0BU;
    note->parameter = 1U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 6U);
    OPENVN_TEST_CHECK(player.order == 1U);
    OPENVN_TEST_CHECK(player.row == 0U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->effect = 0x0DU;
    note->parameter = 0x12U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 6U);
    OPENVN_TEST_CHECK(player.order == 1U);
    OPENVN_TEST_CHECK(player.row == 12U);
    openvn_mod_player_free(&player);


    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U; note->period = 428U;
    note_at(&player, 0U, 1U, 0U)->period = 320U;
    note_at(&player, 0U, 1U, 0U)->effect = 0x03U;
    note_at(&player, 0U, 1U, 0U)->parameter = 8U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 8U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->period == 420U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U; note->period = 428U; note->effect = 0x04U; note->parameter = 0x47U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 3U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->period != 428U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U; note->period = 428U; note->effect = 0x09U; note->parameter = 2U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 1U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->sample_offset == 512U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U; note->period = 428U; note->effect = 0x0EU; note->parameter = 0xD3U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 1U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(!state->triggered);
    run_ticks(&player, 3U);
    OPENVN_TEST_CHECK(state->triggered);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U; note->period = 428U; note->effect = 0x0EU; note->parameter = 0xC2U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 3U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->volume == 0U);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->sample = 1U; note->period = 428U; note->effect = 0x0EU; note->parameter = 0x93U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 4U);
    state = openvn_mod_player_channel_state(&player, 0U);
    OPENVN_TEST_CHECK(state->triggered);
    openvn_mod_player_free(&player);

    prepare(&player);
    note = note_at(&player, 0U, 0U, 0U);
    note->effect = 0x0EU; note->parameter = 0xE2U;
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));
    run_ticks(&player, 6U);
    OPENVN_TEST_CHECK(player.row == 0U);
    run_ticks(&player, 6U);
    OPENVN_TEST_CHECK(player.row == 1U);
    openvn_mod_player_free(&player);

    return 0;
}
