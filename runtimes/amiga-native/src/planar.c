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

static void convert_byte_group(
    OpenVNPlanarBitmap *bitmap,
    const OpenVNILBMImage *image,
    unsigned int y,
    unsigned int byte_index,
    unsigned int create_mask,
    unsigned int transparent_index
) {
    unsigned char plane_bytes[8];
    unsigned char mask_byte;
    unsigned int bit_index;
    unsigned int plane;
    unsigned int x;
    size_t source_row;
    size_t destination_offset;

    memset(plane_bytes, 0, sizeof(plane_bytes));
    mask_byte = 0U;
    source_row = (size_t)y * (size_t)image->width;
    destination_offset =
        (size_t)y * (size_t)bitmap->bytes_per_row + byte_index;

    for (bit_index = 0U; bit_index < 8U; bit_index++) {
        unsigned int pixel;
        unsigned char output_bit;

        x = byte_index * 8U + bit_index;
        if (x >= image->width) {
            break;
        }

        pixel = image->body[source_row + x];
        output_bit = (unsigned char)(0x80U >> bit_index);

        /*
         * Build one complete destination byte in registers before touching
         * the plane buffers. This avoids the old per-pixel read/modify/write
         * loop, which is prohibitively slow on a 68000.
         */
        for (plane = 0U; plane < bitmap->depth; plane++) {
            if ((pixel & (1U << plane)) != 0U) {
                plane_bytes[plane] |= output_bit;
            }
        }

        if (create_mask && pixel != transparent_index) {
            mask_byte |= output_bit;
        }
    }

    for (plane = 0U; plane < bitmap->depth; plane++) {
        bitmap->planes[plane][destination_offset] = plane_bytes[plane];
    }

    if (bitmap->mask != 0) {
        bitmap->mask[destination_offset] = mask_byte;
    }
}

int openvn_planar_from_chunky(
    OpenVNPlanarBitmap *bitmap,
    const OpenVNILBMImage *image,
    int create_mask,
    unsigned int transparent_index
) {
    unsigned int y;
    unsigned int byte_index;
    unsigned int plane;
    unsigned int bytes_per_row;
    unsigned int source_bytes_per_row;
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
    source_bytes_per_row = (image->width + 7U) / 8U;
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
        for (byte_index = 0U;
             byte_index < source_bytes_per_row;
             byte_index++) {
            convert_byte_group(
                bitmap,
                image,
                y,
                byte_index,
                create_mask != 0,
                transparent_index
            );
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
