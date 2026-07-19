#include "openvn_mod.h"

#include "test_check.h"
#include <string.h>

int main(int argc, char **argv) {
    OpenVNMODModule module = {0};
    const OpenVNMODNote *note;

    OPENVN_TEST_CHECK(argc == 2);
    OPENVN_TEST_CHECK(openvn_mod_load_file(&module, argv[1]));
    OPENVN_TEST_CHECK(strcmp(module.title, "OpenVN Test Module") == 0);
    OPENVN_TEST_CHECK(module.song_length == 1U);
    OPENVN_TEST_CHECK(module.pattern_count == 1U);
    OPENVN_TEST_CHECK(module.samples[0].length == 4UL);

    note = openvn_mod_note(&module, 0U, 0U, 0U);
    OPENVN_TEST_CHECK(note != 0);
    OPENVN_TEST_CHECK(note->sample == 1U);
    OPENVN_TEST_CHECK(note->period == 428U);

    openvn_mod_free(&module);
    return 0;
}
