#ifndef OPENVN_GRAPHICS_HOST_H
#define OPENVN_GRAPHICS_HOST_H

#include "openvn_graphics.h"
#include "openvn_image.h"

typedef struct OpenVNHostGraphicsContext {
    int opened;
    int use_datatypes;
    unsigned int present_count;
    const OpenVNAssetTable *assets;
    OpenVNILBMImage background_image;
    OpenVNILBMImage character_image;
    char scene[256];
    char character[256];
    char pose[256];
    char scene_path[512];
    char character_path[512];
} OpenVNHostGraphicsContext;

void openvn_graphics_host_init(
    OpenVNGraphicsService *service,
    OpenVNHostGraphicsContext *context
);

#endif
