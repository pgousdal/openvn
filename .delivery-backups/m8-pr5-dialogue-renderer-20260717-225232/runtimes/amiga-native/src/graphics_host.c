#include "openvn_graphics_host.h"

#include <stdio.h>
#include <string.h>

static int host_open(
    OpenVNGraphicsService *service,
    const OpenVNGraphicsConfig *config
) {
    OpenVNHostGraphicsContext *context;

    context = (OpenVNHostGraphicsContext *)service->context;
    if (context == 0) {
        return 0;
    }

    memset(context, 0, sizeof(*context));
    context->opened = 1;
    context->use_datatypes = config->use_datatypes;
    context->assets = config->assets;
    openvn_ilbm_reset(&context->background_image);
    openvn_ilbm_reset(&context->character_image);

    printf(
        "GRAPHICS OPEN %u %u %u %s\n",
        config->width,
        config->height,
        config->depth,
        config->use_datatypes ? "DATATYPES" : "NATIVE"
    );
    return 1;
}

static void host_close(OpenVNGraphicsService *service) {
    OpenVNHostGraphicsContext *context;

    context = (OpenVNHostGraphicsContext *)service->context;
    if (context != 0 && context->opened) {
        openvn_ilbm_free(&context->background_image);
        openvn_ilbm_free(&context->character_image);
        puts("GRAPHICS CLOSE");
        context->opened = 0;
    }
}

static int host_scene(
    OpenVNGraphicsService *service,
    const char *background
) {
    OpenVNHostGraphicsContext *context;
    const char *path;

    context = (OpenVNHostGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_background(context->assets, background);
    if (path == 0) {
        return 0;
    }

    snprintf(context->scene, sizeof(context->scene), "%s", background);
    snprintf(context->scene_path, sizeof(context->scene_path), "%s", path);

    if (!context->use_datatypes) {
        openvn_ilbm_free(&context->background_image);
        if (!openvn_ilbm_load_file(&context->background_image, path)) {
            return 0;
        }

        printf(
            "GRAPHICS LOAD ILBM %s %u %u %u\n",
            path,
            context->background_image.width,
            context->background_image.height,
            context->background_image.depth
        );
    } else {
        printf("GRAPHICS LOAD DATATYPE %s\n", path);
    }

    printf("GRAPHICS SCENE %s\n", background);
    return 1;
}

static int host_show(
    OpenVNGraphicsService *service,
    const char *character,
    const char *pose
) {
    OpenVNHostGraphicsContext *context;
    const char *path;

    context = (OpenVNHostGraphicsContext *)service->context;
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

    snprintf(context->character, sizeof(context->character), "%s", character);
    snprintf(context->pose, sizeof(context->pose), "%s", pose);
    snprintf(
        context->character_path,
        sizeof(context->character_path),
        "%s",
        path
    );

    if (!context->use_datatypes) {
        openvn_ilbm_free(&context->character_image);
        if (!openvn_ilbm_load_file(&context->character_image, path)) {
            return 0;
        }

        printf(
            "GRAPHICS LOAD ILBM %s %u %u %u\n",
            path,
            context->character_image.width,
            context->character_image.height,
            context->character_image.depth
        );
    } else {
        printf("GRAPHICS LOAD DATATYPE %s\n", path);
    }

    printf("GRAPHICS SHOW %s %s\n", character, pose);
    return 1;
}

static int host_hide(
    OpenVNGraphicsService *service,
    const char *character
) {
    OpenVNHostGraphicsContext *context;

    context = (OpenVNHostGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    if (strcmp(context->character, character) == 0) {
        openvn_ilbm_free(&context->character_image);
        context->character[0] = '\0';
        context->pose[0] = '\0';
        context->character_path[0] = '\0';
    }

    printf("GRAPHICS HIDE %s\n", character);
    return 1;
}

static int host_present(OpenVNGraphicsService *service) {
    OpenVNHostGraphicsContext *context;

    context = (OpenVNHostGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    context->present_count++;
    printf("GRAPHICS PRESENT %u\n", context->present_count);
    return 1;
}

static const OpenVNGraphicsVTable HOST_VTABLE = {
    host_open,
    host_close,
    host_scene,
    host_show,
    host_hide,
    host_present
};

void openvn_graphics_host_init(
    OpenVNGraphicsService *service,
    OpenVNHostGraphicsContext *context
) {
    if (service != 0) {
        service->vtable = &HOST_VTABLE;
        service->context = context;
    }
}
