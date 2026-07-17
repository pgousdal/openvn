#include "openvn_assets_runtime.h"

#include <stdio.h>
#include <string.h>

const char *openvn_asset_find_background(
    const OpenVNAssetTable *table,
    const char *id
) {
    size_t index;

    if (table == 0 || id == 0) {
        return 0;
    }

    for (index = 0U; index < table->background_count; index++) {
        if (strcmp(table->backgrounds[index].id, id) == 0) {
            return table->backgrounds[index].path;
        }
    }

    return 0;
}

const char *openvn_asset_find_character(
    const OpenVNAssetTable *table,
    const char *character,
    const char *pose
) {
    char key[256];
    size_t index;

    if (table == 0 || character == 0 || pose == 0) {
        return 0;
    }

    snprintf(key, sizeof(key), "%s.%s", character, pose);

    for (index = 0U; index < table->character_count; index++) {
        if (strcmp(table->characters[index].id, key) == 0) {
            return table->characters[index].path;
        }
    }

    return 0;
}
