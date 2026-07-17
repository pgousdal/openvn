#include "openvn_mod_player.h"

#include <assert.h>

int main(int argc, char **argv) {
    OpenVNMODPlayer player;
    const OpenVNMODNote *note;
    unsigned int tick;

    assert(argc == 2);
    openvn_mod_player_reset(&player);

    assert(openvn_mod_player_load(&player, argv[1]));
    assert(openvn_mod_player_start(&player, 0));

    note = openvn_mod_player_channel(&player, 0U);
    assert(note != 0);
    assert(note->sample == 1U);
    assert(note->period == 428U);

    for (tick = 0U; tick < 6U; tick++) {
        assert(openvn_mod_player_tick(&player));
    }

    assert(player.row == 1U);
    assert(player.tick == 0U);

    openvn_mod_player_stop(&player);
    assert(!player.playing);

    openvn_mod_player_free(&player);
    return 0;
}
