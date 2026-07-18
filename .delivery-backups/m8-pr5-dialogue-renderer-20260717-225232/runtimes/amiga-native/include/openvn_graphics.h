#ifndef OPENVN_GRAPHICS_H
#define OPENVN_GRAPHICS_H

#include "openvn_assets_runtime.h"

typedef struct OpenVNGraphicsConfig {
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    int use_datatypes;
    int fullscreen;
    const OpenVNAssetTable *assets;
} OpenVNGraphicsConfig;

typedef struct OpenVNGraphicsService OpenVNGraphicsService;

typedef struct OpenVNGraphicsVTable {
    int (*open)(
        OpenVNGraphicsService *service,
        const OpenVNGraphicsConfig *config
    );
    void (*close)(OpenVNGraphicsService *service);
    int (*scene)(
        OpenVNGraphicsService *service,
        const char *background
    );
    int (*show)(
        OpenVNGraphicsService *service,
        const char *character,
        const char *pose
    );
    int (*hide)(
        OpenVNGraphicsService *service,
        const char *character
    );
    int (*present)(OpenVNGraphicsService *service);
} OpenVNGraphicsVTable;

struct OpenVNGraphicsService {
    const OpenVNGraphicsVTable *vtable;
    void *context;
};

int openvn_graphics_open(
    OpenVNGraphicsService *service,
    const OpenVNGraphicsConfig *config
);
void openvn_graphics_close(OpenVNGraphicsService *service);
int openvn_graphics_scene(
    OpenVNGraphicsService *service,
    const char *background
);
int openvn_graphics_show(
    OpenVNGraphicsService *service,
    const char *character,
    const char *pose
);
int openvn_graphics_hide(
    OpenVNGraphicsService *service,
    const char *character
);
int openvn_graphics_present(OpenVNGraphicsService *service);

#endif
