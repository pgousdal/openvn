#include "openvn_mod_player.h"

#include "test_check.h"

int main(int argc, char **argv) {
    OpenVNMODPlayer player = {0};
    const OpenVNMODNote *note;
    unsigned int tick;

    OPENVN_TEST_CHECK(argc == 2);
    openvn_mod_player_reset(&player);

    OPENVN_TEST_CHECK(openvn_mod_player_load(&player, argv[1]));
    OPENVN_TEST_CHECK(openvn_mod_player_start(&player, 0));

    note = openvn_mod_player_channel(&player, 0U);
    OPENVN_TEST_CHECK(note != 0);
    OPENVN_TEST_CHECK(note->sample == 1U);
    OPENVN_TEST_CHECK(note->period == 428U);

    for (tick = 0U; tick < 6U; tick++) {
        OPENVN_TEST_CHECK(openvn_mod_player_tick(&player));
    }

    OPENVN_TEST_CHECK(player.row == 1U);
    OPENVN_TEST_CHECK(player.tick == 0U);

    openvn_mod_player_stop(&player);
    OPENVN_TEST_CHECK(!player.playing);

    openvn_mod_player_free(&player);
    return 0;
}
