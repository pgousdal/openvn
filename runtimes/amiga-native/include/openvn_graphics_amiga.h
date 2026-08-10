#ifndef OPENVN_GRAPHICS_AMIGA_H
#define OPENVN_GRAPHICS_AMIGA_H

#include "openvn_bitmap_amiga.h"
#include "openvn_dialogue.h"
#include "openvn_display_amiga.h"
#include "openvn_graphics.h"
#include "openvn_image.h"
#include "openvn_palette.h"
#include "openvn_planar.h"
#include "openvn_scene_layout.h"

#ifdef __AMIGA__

#include <intuition/classusr.h>

typedef struct OpenVNAmigaGraphicsContext {
    OpenVNAmigaDisplay display;
    const OpenVNAssetTable *assets;

    int use_datatypes;
    Object *background_datatype;
    Object *character_datatype;

    OpenVNILBMImage background_ilbm;
    OpenVNILBMImage character_ilbm;
    OpenVNPlanarBitmap background_planar;
    OpenVNPlanarBitmap character_planar;
    OpenVNAmigaBitmap background_bitmap;
    OpenVNAmigaBitmap character_bitmap;
    OpenVNPalette background_palette;

    int opened;
    int character_visible;
    OpenVNCharacterAnchor character_anchor;
    unsigned int character_origin_x;
    unsigned int character_origin_y;
    int dialogue_visible;
    char dialogue_text[OPENVN_DIALOGUE_MAX_WRAPPED];
    const OpenVNGeneratedChoice *choice_options;
    size_t choice_count;
    size_t choice_selected;
    int choices_visible;
} OpenVNAmigaGraphicsContext;

void openvn_graphics_amiga_init(
    OpenVNGraphicsService *service,
    OpenVNAmigaGraphicsContext *context
);

int openvn_graphics_amiga_wait_choice(
    OpenVNGraphicsService *service,
    size_t *selected_index
);

unsigned long openvn_graphics_amiga_choice_signal_mask(
    OpenVNGraphicsService *service
);

int openvn_graphics_amiga_poll_choice(
    OpenVNGraphicsService *service,
    size_t *selected_index,
    int *selected
);

#endif

#endif
