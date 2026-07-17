#include "openvn_planar.h"

#include <stdlib.h>
#include <string.h>

static unsigned int padded_bytes_per_row(unsigned int width) {
    return ((width + 15U) / 16U) * 2U;
}

void openvn_planar_reset(OpenVNPlanarBitmap *bitmap) {
    if (bitmap != 0) {
        memset(bitmap, 0, sizeof(*bitmap));
    }
}

void openvn_planar_free(OpenVNPlanarBitmap *bitmap) {
    unsigned int plane;

    if (bitmap == 0) {
        return;
    }

    if (bitmap->planes != 0) {
        for (plane = 0U; plane < bitmap->depth; plane++) {
            free(bitmap->planes[plane]);
        }
        free(bitmap->planes);
    }

    free(bitmap->mask);
    openvn_planar_reset(bitmap);
}

int openvn_planar_from_chunky(
    OpenVNPlanarBitmap *bitmap,
    const OpenVNILBMImage *image,
    int create_mask,
    unsigned int transparent_index
) {
    unsigned int x;
    unsigned int y;
    unsigned int plane;
    unsigned int bytes_per_row;
    size_t plane_size;
    size_t expected_pixels;

    if (bitmap == 0 || image == 0 || image->body == 0 ||
        image->width == 0U || image->height == 0U ||
        image->depth == 0U || image->depth > 8U) {
        return 0;
    }

    expected_pixels = (size_t)image->width * (size_t)image->height;
    if (image->body_size < expected_pixels) {
        return 0;
    }

    openvn_planar_reset(bitmap);

    bytes_per_row = padded_bytes_per_row(image->width);
    plane_size = (size_t)bytes_per_row * (size_t)image->height;

    bitmap->planes = (unsigned char **)calloc(
        image->depth,
        sizeof(unsigned char *)
    );
    if (bitmap->planes == 0) {
        return 0;
    }

    bitmap->width = image->width;
    bitmap->height = image->height;
    bitmap->depth = image->depth;
    bitmap->bytes_per_row = bytes_per_row;
    bitmap->plane_size = plane_size;

    for (plane = 0U; plane < bitmap->depth; plane++) {
        bitmap->planes[plane] = (unsigned char *)calloc(
            plane_size,
            1U
        );
        if (bitmap->planes[plane] == 0) {
            openvn_planar_free(bitmap);
            return 0;
        }
    }

    if (create_mask) {
        bitmap->mask = (unsigned char *)calloc(plane_size, 1U);
        if (bitmap->mask == 0) {
            openvn_planar_free(bitmap);
            return 0;
        }
    }

    for (y = 0U; y < image->height; y++) {
        for (x = 0U; x < image->width; x++) {
            unsigned int pixel = image->body[
                (size_t)y * image->width + x
            ];
            size_t offset = (size_t)y * bytes_per_row + (x / 8U);
            unsigned char bit = (unsigned char)(0x80U >> (x & 7U));

            for (plane = 0U; plane < bitmap->depth; plane++) {
                if ((pixel & (1U << plane)) != 0U) {
                    bitmap->planes[plane][offset] |= bit;
                }
            }

            if (bitmap->mask != 0 && pixel != transparent_index) {
                bitmap->mask[offset] |= bit;
            }
        }
    }

    return 1;
}

unsigned int openvn_planar_pixel(
    const OpenVNPlanarBitmap *bitmap,
    unsigned int x,
    unsigned int y
) {
    unsigned int value;
    unsigned int plane;
    size_t offset;
    unsigned char bit;

    if (bitmap == 0 || bitmap->planes == 0 ||
        x >= bitmap->width || y >= bitmap->height) {
        return 0U;
    }

    offset = (size_t)y * bitmap->bytes_per_row + (x / 8U);
    bit = (unsigned char)(0x80U >> (x & 7U));
    value = 0U;

    for (plane = 0U; plane < bitmap->depth; plane++) {
        if ((bitmap->planes[plane][offset] & bit) != 0U) {
            value |= 1U << plane;
        }
    }

    return value;
}

int openvn_planar_mask_pixel(
    const OpenVNPlanarBitmap *bitmap,
    unsigned int x,
    unsigned int y
) {
    size_t offset;
    unsigned char bit;

    if (bitmap == 0 || bitmap->mask == 0 ||
        x >= bitmap->width || y >= bitmap->height) {
        return 0;
    }

    offset = (size_t)y * bitmap->bytes_per_row + (x / 8U);
    bit = (unsigned char)(0x80U >> (x & 7U));
    return (bitmap->mask[offset] & bit) != 0U;
}
