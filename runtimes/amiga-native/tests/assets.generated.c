#include "assets.generated.h"

static const OpenVNAssetEntry backgrounds[] = {
    {"lighthouse_storm", "runtimes/amiga-native/tests/fixtures/lighthouse_storm.iff"}
};

static const OpenVNAssetEntry characters[] = {
    {"erik.neutral", "runtimes/amiga-native/tests/fixtures/erik_neutral.iff"}
};

static const OpenVNAssetEntry music[] = {
    {"storm", "runtimes/amiga-native/tests/fixtures/storm.mod"}
};

static const OpenVNAssetEntry sounds[] = {
    {"thunder", "runtimes/amiga-native/tests/fixtures/thunder.8svx"}
};

const OpenVNAssetTable OPENVN_GENERATED_ASSETS = {
    backgrounds,
    1U,
    characters,
    1U,
    music,
    1U,
    sounds,
    1U
};
