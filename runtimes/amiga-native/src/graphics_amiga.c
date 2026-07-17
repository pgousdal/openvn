#include "openvn_graphics_amiga.h"

#ifdef __AMIGA__

#include <datatypes/datatypes.h>
#include <proto/datatypes.h>

#include <string.h>

static void dispose_object(struct Object **object) {
    if (object != 0 && *object != 0) {
        DisposeDTObject(*object);
        *object = 0;
    }
}

static void free_classic_background(
    OpenVNAmigaGraphicsContext *context
) {
    openvn_amiga_bitmap_free(&context->background_bitmap);
    openvn_planar_free(&context->background_planar);
    openvn_ilbm_free(&context->background_ilbm);
    openvn_palette_reset(&context->background_palette);
}

static void free_classic_character(
    OpenVNAmigaGraphicsContext *context
) {
    openvn_amiga_bitmap_free(&context->character_bitmap);
    openvn_planar_free(&context->character_planar);
    openvn_ilbm_free(&context->character_ilbm);
}

static int amiga_open(
    OpenVNGraphicsService *service,
    const OpenVNGraphicsConfig *config
) {
    OpenVNAmigaGraphicsContext *context;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0) {
        return 0;
    }

    memset(context, 0, sizeof(*context));
    context->use_datatypes = config->use_datatypes;
    context->assets = config->assets;

    openvn_amiga_display_reset(&context->display);
    openvn_palette_reset(&context->background_palette);
    openvn_ilbm_reset(&context->background_ilbm);
    openvn_ilbm_reset(&context->character_ilbm);
    openvn_planar_reset(&context->background_planar);
    openvn_planar_reset(&context->character_planar);
    openvn_amiga_bitmap_reset(&context->background_bitmap);
    openvn_amiga_bitmap_reset(&context->character_bitmap);

    if (!openvn_amiga_display_open(
            &context->display,
            config->width,
            config->height,
            config->depth,
            config->fullscreen,
            1
        )) {
        return 0;
    }

    context->opened = 1;
    return 1;
}

static void amiga_close(OpenVNGraphicsService *service) {
    OpenVNAmigaGraphicsContext *context;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0) {
        return;
    }

    dispose_object(&context->background_datatype);
    dispose_object(&context->character_datatype);
    free_classic_background(context);
    free_classic_character(context);
    openvn_amiga_display_close(&context->display);
    context->opened = 0;
}

static int load_datatype(
    struct Object **destination,
    const char *path
) {
    dispose_object(destination);

    *destination = NewDTObject(
        (APTR)path,
        DTA_SourceType,
        DTST_FILE,
        DTA_GroupID,
        GID_PICTURE,
        PDTA_Remap,
        TRUE,
        TAG_DONE
    );

    return *destination != 0;
}

static int load_classic_bitmap(
    const char *path,
    OpenVNILBMImage *image,
    OpenVNPlanarBitmap *planar,
    OpenVNAmigaBitmap *bitmap,
    int masked
) {
    openvn_amiga_bitmap_free(bitmap);
    openvn_planar_free(planar);
    openvn_ilbm_free(image);

    if (!openvn_ilbm_load_file(image, path)) {
        return 0;
    }

    if (!openvn_planar_from_chunky(
            planar,
            image,
            masked,
            0U
        )) {
        openvn_ilbm_free(image);
        return 0;
    }

    if (!openvn_amiga_bitmap_from_planar(bitmap, planar)) {
        openvn_planar_free(planar);
        openvn_ilbm_free(image);
        return 0;
    }

    return 1;
}

static int amiga_scene(
    OpenVNGraphicsService *service,
    const char *background
) {
    OpenVNAmigaGraphicsContext *context;
    const char *path;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_background(context->assets, background);
    if (path == 0) {
        return 0;
    }

    if (context->use_datatypes) {
        return load_datatype(&context->background_datatype, path);
    }

    if (!load_classic_bitmap(
            path,
            &context->background_ilbm,
            &context->background_planar,
            &context->background_bitmap,
            0
        )) {
        return 0;
    }

    if (!openvn_palette_from_ilbm(
            &context->background_palette,
            context->background_ilbm.palette,
            context->background_ilbm.palette_size
        )) {
        return 0;
    }

    return openvn_amiga_display_load_palette(
        &context->display,
        context->background_palette.colors
    );
}

static int amiga_show(
    OpenVNGraphicsService *service,
    const char *character,
    const char *pose
) {
    OpenVNAmigaGraphicsContext *context;
    const char *path;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_character(
        context->assets,
        character,
        pose
    );
    if (path == 0) {
        return 0;
    }

    context->character_visible = 1;

    if (context->use_datatypes) {
        return load_datatype(&context->character_datatype, path);
    }

    return load_classic_bitmap(
        path,
        &context->character_ilbm,
        &context->character_planar,
        &context->character_bitmap,
        1
    );
}

static int amiga_hide(
    OpenVNGraphicsService *service,
    const char *character
) {
    OpenVNAmigaGraphicsContext *context;

    (void)character;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    dispose_object(&context->character_datatype);
    free_classic_character(context);
    context->character_visible = 0;
    return 1;
}

static int present_datatypes(
    OpenVNAmigaGraphicsContext *context
) {
    struct RastPort *rastport;

    rastport = openvn_amiga_display_draw_rastport(
        &context->display
    );
    if (rastport == 0) {
        return 0;
    }

    if (context->background_datatype != 0) {
        DrawDTObject(
            rastport,
            context->background_datatype,
            0,
            0,
            context->display.window->Width,
            context->display.window->Height,
            0,
            0,
            0
        );
    }

    if (context->character_visible &&
        context->character_datatype != 0) {
        DrawDTObject(
            rastport,
            context->character_datatype,
            0,
            0,
            context->display.window->Width,
            context->display.window->Height,
            0,
            0,
            0
        );
    }

    return openvn_amiga_display_present(&context->display);
}

static int present_classic(
    OpenVNAmigaGraphicsContext *context
) {
    struct RastPort *rastport;
    int x;
    int y;

    rastport = openvn_amiga_display_draw_rastport(
        &context->display
    );
    if (rastport == 0) {
        return 0;
    }

    if (!openvn_amiga_bitmap_blit(
            &context->background_bitmap,
            rastport,
            0,
            0,
            0
        )) {
        return 0;
    }

    if (context->character_visible &&
        context->character_bitmap.bitmap != 0) {
        x = (
            (int)context->display.window->Width -
            (int)context->character_bitmap.width
        ) / 2;
        y = (
            (int)context->display.window->Height -
            (int)context->character_bitmap.height
        );

        if (!openvn_amiga_bitmap_blit(
                &context->character_bitmap,
                rastport,
                x,
                y,
                1
            )) {
            return 0;
        }
    }

    return openvn_amiga_display_present(&context->display);
}

static int amiga_present(OpenVNGraphicsService *service) {
    OpenVNAmigaGraphicsContext *context;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened ||
        context->display.window == 0) {
        return 0;
    }

    if (context->use_datatypes) {
        return present_datatypes(context);
    }

    return present_classic(context);
}

static const OpenVNGraphicsVTable AMIGA_VTABLE = {
    amiga_open,
    amiga_close,
    amiga_scene,
    amiga_show,
    amiga_hide,
    amiga_present
};

void openvn_graphics_amiga_init(
    OpenVNGraphicsService *service,
    OpenVNAmigaGraphicsContext *context
) {
    if (service != 0) {
        service->vtable = &AMIGA_VTABLE;
        service->context = context;
    }
}

#endif
