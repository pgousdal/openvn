#include "openvn_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int read_u16_be(const unsigned char *data) {
    return ((unsigned int)data[0] << 8) | (unsigned int)data[1];
}

static unsigned long read_u32_be(const unsigned char *data) {
    return ((unsigned long)data[0] << 24) |
           ((unsigned long)data[1] << 16) |
           ((unsigned long)data[2] << 8) |
           (unsigned long)data[3];
}

static int read_file(
    const char *path,
    unsigned char **data_out,
    size_t *size_out
) {
    FILE *file;
    long size;
    unsigned char *data;

    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    data = (unsigned char *)malloc((size_t)size);
    if (data == 0) {
        fclose(file);
        return 0;
    }

    if (fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }

    fclose(file);
    *data_out = data;
    *size_out = (size_t)size;
    return 1;
}

void openvn_ilbm_reset(OpenVNILBMImage *image) {
    if (image != 0) {
        memset(image, 0, sizeof(*image));
    }
}

void openvn_ilbm_free(OpenVNILBMImage *image) {
    if (image == 0) {
        return;
    }

    free(image->palette);
    free(image->body);
    openvn_ilbm_reset(image);
}

int openvn_ilbm_load_file(
    OpenVNILBMImage *image,
    const char *path
) {
    unsigned char *data;
    size_t size;
    size_t cursor;
    int have_bmhd;
    int have_body;

    if (image == 0 || path == 0) {
        return 0;
    }

    openvn_ilbm_reset(image);

    if (!read_file(path, &data, &size)) {
        return 0;
    }

    if (size < 12U ||
        memcmp(data, "FORM", 4U) != 0 ||
        memcmp(data + 8U, "ILBM", 4U) != 0) {
        free(data);
        return 0;
    }

    cursor = 12U;
    have_bmhd = 0;
    have_body = 0;

    while (cursor + 8U <= size) {
        const unsigned char *chunk_id = data + cursor;
        unsigned long chunk_size = read_u32_be(data + cursor + 4U);
        size_t payload = cursor + 8U;
        size_t next = payload + (size_t)chunk_size;

        if (next > size) {
            openvn_ilbm_free(image);
            free(data);
            return 0;
        }

        if (memcmp(chunk_id, "BMHD", 4U) == 0) {
            if (chunk_size < 20U) {
                openvn_ilbm_free(image);
                free(data);
                return 0;
            }

            image->width = read_u16_be(data + payload);
            image->height = read_u16_be(data + payload + 2U);
            image->depth = data[payload + 8U];
            have_bmhd = 1;
        } else if (memcmp(chunk_id, "CMAP", 4U) == 0) {
            image->palette = (unsigned char *)malloc((size_t)chunk_size);
            if (image->palette == 0) {
                openvn_ilbm_free(image);
                free(data);
                return 0;
            }

            memcpy(image->palette, data + payload, (size_t)chunk_size);
            image->palette_size = (unsigned int)chunk_size;
        } else if (memcmp(chunk_id, "BODY", 4U) == 0) {
            image->body = (unsigned char *)malloc((size_t)chunk_size);
            if (image->body == 0) {
                openvn_ilbm_free(image);
                free(data);
                return 0;
            }

            memcpy(image->body, data + payload, (size_t)chunk_size);
            image->body_size = (size_t)chunk_size;
            have_body = 1;
        }

        cursor = next + (chunk_size & 1UL);
    }

    free(data);

    if (!have_bmhd || !have_body ||
        image->width == 0U ||
        image->height == 0U ||
        image->depth == 0U) {
        openvn_ilbm_free(image);
        return 0;
    }

    return 1;
}
