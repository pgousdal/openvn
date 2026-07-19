#include "openvn_palette.h"

#include "test_check.h"

int main(void) {
    OpenVNPalette palette;
    unsigned char cmap[] = {
        0U, 0U, 0U,
        255U, 128U, 64U
    };

    OPENVN_TEST_CHECK(openvn_palette_from_ilbm(
        &palette,
        cmap,
        sizeof(cmap)
    ));

    OPENVN_TEST_CHECK(palette.color_count == 2U);
    OPENVN_TEST_CHECK(palette.colors[0] == (2UL << 16));
    OPENVN_TEST_CHECK(palette.colors[1] == 0UL);
    OPENVN_TEST_CHECK(palette.colors[4] == 0xFFFFFFFFUL);
    OPENVN_TEST_CHECK(palette.colors[5] == 0x80808080UL);
    OPENVN_TEST_CHECK(palette.colors[6] == 0x40404040UL);
    OPENVN_TEST_CHECK(palette.colors[7] == 0UL);

    return 0;
}
