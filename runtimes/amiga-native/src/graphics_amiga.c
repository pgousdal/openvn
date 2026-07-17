#include "openvn_graphics_amiga.h"

#ifdef __AMIGA__

#include <datatypes/datatypes.h>
#include <intuition/intuition.h>
#include <proto/datatypes.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include <string.h>

static void dispose_object(struct Object **object) {
    if (object != 0 && *object != 0) {
        DisposeDTObject(*object);
        *object = 0;
    }
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
    openvn_ilbm_reset(&context->background_ilbm);
    openvn_ilbm_reset(&context->character_ilbm);

    context->screen = OpenScreenTags(
        0,
        SA_Width,
        config->width,
        SA_Height,
        config->height,
        SA_Depth,
        config->depth,
        SA_Title,
        (ULONG)"OpenVN",
        TAG_DONE
    );

    if (context->screen == 0) {
        return 0;
    }

    context->window = OpenWindowTags(
        0,
        WA_CustomScreen,
        (ULONG)context->screen,
        WA_Left,
        0,
        WA_Top,
        0,
        WA_Width,
        config->width,
        WA_Height,
        config->height,
        WA_Borderless,
        TRUE,
        WA_Activate,
        TRUE,
        TAG_DONE
    );

    if (context->window == 0) {
        CloseScreen(context->screen);
        context->screen = 0;
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
    openvn_ilbm_free(&context->background_ilbm);
    openvn_ilbm_free(&context->character_ilbm);

    if (context->window != 0) {
        CloseWindow(context->window);
        context->window = 0;
    }

    if (context->screen != 0) {
        CloseScreen(context->screen);
        context->screen = 0;
    }

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

    openvn_ilbm_free(&context->background_ilbm);
    return openvn_ilbm_load_file(&context->background_ilbm, path);
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

    if (context->use_datatypes) {
        return load_datatype(&context->character_datatype, path);
    }

    openvn_ilbm_free(&context->character_ilbm);
    return openvn_ilbm_load_file(&context->character_ilbm, path);
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
    openvn_ilbm_free(&context->character_ilbm);
    return 1;
}

static int amiga_present(OpenVNGraphicsService *service) {
    OpenVNAmigaGraphicsContext *context;
    struct RastPort *rastport;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened || context->window == 0) {
        return 0;
    }

    rastport = context->window->RPort;

    if (context->use_datatypes) {
        if (context->background_datatype != 0) {
            DrawDTObject(
                rastport,
                context->background_datatype,
                0,
                0,
                context->window->Width,
                context->window->Height,
                0,
                0,
                0
            );
        }

        if (context->character_datatype != 0) {
            DrawDTObject(
                rastport,
                context->character_datatype,
                0,
                0,
                context->window->Width,
                context->window->Height,
                0,
                0,
                0
            );
        }

        return 1;
    }

    /*
     * Classic ILBM data is loaded and validated here. Converting BODY
     * bytes into native planar BitMaps and blitting them is M5 PR5.2.
     */
    return context->background_ilbm.body != 0;
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
