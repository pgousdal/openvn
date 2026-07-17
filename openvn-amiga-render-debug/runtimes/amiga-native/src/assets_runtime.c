#include "openvn_assets_runtime.h"

#include <stdio.h>
#include <string.h>

static const char *find_entry(
    const OpenVNAssetEntry *entries,
    size_t count,
    const char *id
) {
    size_t index;

    if (entries == 0 || id == 0) {
        return 0;
    }

    for (index = 0U; index < count; index++) {
        if (strcmp(entries[index].id, id) == 0) {
            return entries[index].path;
        }
    }

    return 0;
}

const char *openvn_asset_find_background(
    const OpenVNAssetTable *table,
    const char *id
) {
    if (table == 0) {
        return 0;
    }

    return find_entry(
        table->backgrounds,
        table->background_count,
        id
    );
}

const char *openvn_asset_find_character(
    const OpenVNAssetTable *table,
    const char *character,
    const char *pose
) {
    char key[256];

    if (table == 0 || character == 0 || pose == 0) {
        return 0;
    }

    snprintf(key, sizeof(key), "%s.%s", character, pose);
    return find_entry(table->characters, table->character_count, key);
}

const char *openvn_asset_find_music(
    const OpenVNAssetTable *table,
    const char *id
) {
    if (table == 0) {
        return 0;
    }

    return find_entry(table->music, table->music_count, id);
}

const char *openvn_asset_find_sound(
    const OpenVNAssetTable *table,
    const char *id
) {
    if (table == 0) {
        return 0;
    }

    return find_entry(table->sounds, table->sound_count, id);
}
