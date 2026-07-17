#include "openvn_palette.h"

#include <assert.h>

int main(void) {
    OpenVNPalette palette;
    unsigned char cmap[] = {
        0U, 0U, 0U,
        255U, 128U, 64U
    };

    assert(openvn_palette_from_ilbm(
        &palette,
        cmap,
        sizeof(cmap)
    ));

    assert(palette.color_count == 2U);
    assert(palette.colors[0] == (2UL << 16));
    assert(palette.colors[1] == 0UL);
    assert(palette.colors[4] == 0xFFFFFFFFUL);
    assert(palette.colors[5] == 0x80808080UL);
    assert(palette.colors[6] == 0x40404040UL);
    assert(palette.colors[7] == 0UL);

    return 0;
}
