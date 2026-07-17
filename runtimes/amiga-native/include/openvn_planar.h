#ifndef OPENVN_PLANAR_H
#define OPENVN_PLANAR_H

#include <stddef.h>

#include "openvn_image.h"

typedef struct OpenVNPlanarBitmap {
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int bytes_per_row;
    unsigned char **planes;
    unsigned char *mask;
    size_t plane_size;
} OpenVNPlanarBitmap;

void openvn_planar_reset(OpenVNPlanarBitmap *bitmap);
void openvn_planar_free(OpenVNPlanarBitmap *bitmap);

int openvn_planar_from_chunky(
    OpenVNPlanarBitmap *bitmap,
    const OpenVNILBMImage *image,
    int create_mask,
    unsigned int transparent_index
);

unsigned int openvn_planar_pixel(
    const OpenVNPlanarBitmap *bitmap,
    unsigned int x,
    unsigned int y
);

int openvn_planar_mask_pixel(
    const OpenVNPlanarBitmap *bitmap,
    unsigned int x,
    unsigned int y
);

#endif
