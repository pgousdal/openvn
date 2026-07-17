#include "assets.generated.h"

static const OpenVNAssetEntry backgrounds[] = {
    {"lighthouse_storm", "tests/fixtures/lighthouse_storm.iff"}
};

static const OpenVNAssetEntry characters[] = {
    {"erik.neutral", "tests/fixtures/erik_neutral.iff"}
};

const OpenVNAssetTable OPENVN_GENERATED_ASSETS = {
    backgrounds,
    1U,
    characters,
    1U
};
