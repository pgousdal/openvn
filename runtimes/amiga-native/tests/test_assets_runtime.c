#include "assets.generated.h"
#include "openvn_assets_runtime.h"

#include <assert.h>
#include <string.h>

int main(void) {
    const char *background;
    const char *character;

    background = openvn_asset_find_background(
        &OPENVN_GENERATED_ASSETS,
        "lighthouse_storm"
    );
    character = openvn_asset_find_character(
        &OPENVN_GENERATED_ASSETS,
        "erik",
        "neutral"
    );

    assert(background != 0);
    assert(character != 0);
    assert(strcmp(
        background,
        "runtimes/amiga-native/tests/fixtures/lighthouse_storm.iff"
    ) == 0);
    assert(strcmp(
        character,
        "runtimes/amiga-native/tests/fixtures/erik_neutral.iff"
    ) == 0);

    return 0;
}
