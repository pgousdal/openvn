#ifndef OPENVN_PALETTE_H
#define OPENVN_PALETTE_H

typedef struct OpenVNPalette {
    unsigned int color_count;
    unsigned long colors[256U * 3U + 2U];
} OpenVNPalette;

void openvn_palette_reset(OpenVNPalette *palette);

int openvn_palette_from_ilbm(
    OpenVNPalette *palette,
    const unsigned char *cmap,
    unsigned int cmap_size
);

#endif
