#ifndef OPENVN_BITMAP_AMIGA_H
#define OPENVN_BITMAP_AMIGA_H

#include "openvn_planar.h"

#ifdef __AMIGA__

struct BitMap;
struct RastPort;

typedef struct OpenVNAmigaBitmap {
    struct BitMap *bitmap;
    struct BitMap *mask_bitmap;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
} OpenVNAmigaBitmap;

void openvn_amiga_bitmap_reset(OpenVNAmigaBitmap *bitmap);
void openvn_amiga_bitmap_free(OpenVNAmigaBitmap *bitmap);

int openvn_amiga_bitmap_from_planar(
    OpenVNAmigaBitmap *destination,
    const OpenVNPlanarBitmap *source
);

int openvn_amiga_bitmap_blit(
    const OpenVNAmigaBitmap *source,
    struct RastPort *destination,
    int x,
    int y,
    int masked
);

#endif

#endif
