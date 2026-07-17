#include "openvn_palette.h"

#include <string.h>

void openvn_palette_reset(OpenVNPalette *palette) {
    if (palette != 0) {
        memset(palette, 0, sizeof(*palette));
    }
}

static unsigned long expand_component(unsigned int value) {
    return ((unsigned long)value << 24) |
           ((unsigned long)value << 16) |
           ((unsigned long)value << 8) |
           (unsigned long)value;
}

int openvn_palette_from_ilbm(
    OpenVNPalette *palette,
    const unsigned char *cmap,
    unsigned int cmap_size
) {
    unsigned int index;
    unsigned int color_count;
    unsigned int output;

    if (palette == 0 || cmap == 0 || cmap_size == 0U ||
        (cmap_size % 3U) != 0U) {
        return 0;
    }

    color_count = cmap_size / 3U;
    if (color_count > 256U) {
        return 0;
    }

    openvn_palette_reset(palette);
    palette->color_count = color_count;
    palette->colors[0] = (unsigned long)color_count << 16;

    output = 1U;
    for (index = 0U; index < color_count; index++) {
        palette->colors[output++] = expand_component(cmap[index * 3U]);
        palette->colors[output++] = expand_component(cmap[index * 3U + 1U]);
        palette->colors[output++] = expand_component(cmap[index * 3U + 2U]);
    }

    palette->colors[output] = 0UL;
    return 1;
}
