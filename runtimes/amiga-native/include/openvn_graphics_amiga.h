#ifndef OPENVN_GRAPHICS_AMIGA_H
#define OPENVN_GRAPHICS_AMIGA_H

#include "openvn_graphics.h"
#include "openvn_image.h"

#ifdef __AMIGA__

struct Screen;
struct Window;
struct BitMap;
struct Object;

typedef struct OpenVNAmigaGraphicsContext {
    struct Screen *screen;
    struct Window *window;
    struct BitMap *background;
    struct BitMap *character;
    struct Object *background_datatype;
    struct Object *character_datatype;
    const OpenVNAssetTable *assets;
    OpenVNILBMImage background_ilbm;
    OpenVNILBMImage character_ilbm;
    int opened;
    int use_datatypes;
} OpenVNAmigaGraphicsContext;

void openvn_graphics_amiga_init(
    OpenVNGraphicsService *service,
    OpenVNAmigaGraphicsContext *context
);

#endif

#endif
