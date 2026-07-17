#ifndef OPENVN_IMAGE_H
#define OPENVN_IMAGE_H

#include <stddef.h>

typedef enum OpenVNImageFormat {
    OPENVN_IMAGE_UNKNOWN = 0,
    OPENVN_IMAGE_ILBM,
    OPENVN_IMAGE_DATATYPE
} OpenVNImageFormat;

typedef struct OpenVNILBMImage {
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int palette_size;
    unsigned char *palette;
    unsigned char *body;
    size_t body_size;
} OpenVNILBMImage;

void openvn_ilbm_reset(OpenVNILBMImage *image);
void openvn_ilbm_free(OpenVNILBMImage *image);
int openvn_ilbm_load_file(
    OpenVNILBMImage *image,
    const char *path
);

#endif
