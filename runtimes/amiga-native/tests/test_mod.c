#include "openvn_mod.h"

#include <assert.h>
#include <string.h>

int main(int argc, char **argv) {
    OpenVNMODModule module;
    const OpenVNMODNote *note;

    assert(argc == 2);
    assert(openvn_mod_load_file(&module, argv[1]));
    assert(strcmp(module.title, "OpenVN Test Module") == 0);
    assert(module.song_length == 1U);
    assert(module.pattern_count == 1U);
    assert(module.samples[0].length == 4UL);

    note = openvn_mod_note(&module, 0U, 0U, 0U);
    assert(note != 0);
    assert(note->sample == 1U);
    assert(note->period == 428U);

    openvn_mod_free(&module);
    return 0;
}
