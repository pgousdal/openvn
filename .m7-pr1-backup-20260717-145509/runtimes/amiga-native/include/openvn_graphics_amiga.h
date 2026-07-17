#ifndef OPENVN_GRAPHICS_AMIGA_H
#define OPENVN_GRAPHICS_AMIGA_H

#include "openvn_bitmap_amiga.h"
#include "openvn_display_amiga.h"
#include "openvn_graphics.h"
#include "openvn_image.h"
#include "openvn_palette.h"
#include "openvn_planar.h"

#ifdef __AMIGA__

#include <intuition/classes.h>

typedef struct OpenVNAmigaGraphicsContext {
    OpenVNAmigaDisplay display;
    Object *background_datatype;
    Object *character_datatype;
    const OpenVNAssetTable *assets;

    OpenVNILBMImage background_ilbm;
    OpenVNILBMImage character_ilbm;
    OpenVNPlanarBitmap background_planar;
    OpenVNPlanarBitmap character_planar;
    OpenVNAmigaBitmap background_bitmap;
    OpenVNAmigaBitmap character_bitmap;
    OpenVNPalette background_palette;

    int opened;
    int use_datatypes;
    int character_visible;
} OpenVNAmigaGraphicsContext;

void openvn_graphics_amiga_init(
    OpenVNGraphicsService *service,
    OpenVNAmigaGraphicsContext *context
);

#endif

#endif
