#ifndef OPENVN_ASSETS_RUNTIME_H
#define OPENVN_ASSETS_RUNTIME_H

#include <stddef.h>

typedef struct OpenVNAssetEntry {
    const char *id;
    const char *path;
} OpenVNAssetEntry;

typedef struct OpenVNAssetTable {
    const OpenVNAssetEntry *backgrounds;
    size_t background_count;
    const OpenVNAssetEntry *characters;
    size_t character_count;
    const OpenVNAssetEntry *music;
    size_t music_count;
    const OpenVNAssetEntry *sounds;
    size_t sound_count;
} OpenVNAssetTable;

const char *openvn_asset_find_background(
    const OpenVNAssetTable *table,
    const char *id
);

const char *openvn_asset_find_character(
    const OpenVNAssetTable *table,
    const char *character,
    const char *pose
);

const char *openvn_asset_find_music(
    const OpenVNAssetTable *table,
    const char *id
);

const char *openvn_asset_find_sound(
    const OpenVNAssetTable *table,
    const char *id
);

#endif
