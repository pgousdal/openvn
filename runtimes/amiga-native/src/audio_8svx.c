#include "openvn_8svx.h"

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

void openvn_8svx_reset(OpenVN8SVXSample *sample) {
    if (sample != 0) {
        memset(sample, 0, sizeof(*sample));
    }
}

void openvn_8svx_free(OpenVN8SVXSample *sample) {
    if (sample == 0) {
        return;
    }

    free(sample->data);
    openvn_8svx_reset(sample);
}

int openvn_8svx_load_file(
    OpenVN8SVXSample *sample,
    const char *path
) {
    unsigned char *data;
    size_t size;
    size_t cursor;
    int have_vhdr;
    int have_body;

    if (sample == 0 || path == 0) {
        return 0;
    }

    openvn_8svx_reset(sample);

    if (!read_file(path, &data, &size)) {
        return 0;
    }

    if (size < 12U ||
        memcmp(data, "FORM", 4U) != 0 ||
        memcmp(data + 8U, "8SVX", 4U) != 0) {
        free(data);
        return 0;
    }

    cursor = 12U;
    have_vhdr = 0;
    have_body = 0;

    while (cursor + 8U <= size) {
        const unsigned char *chunk_id = data + cursor;
        unsigned long chunk_size = read_u32_be(data + cursor + 4U);
        size_t payload = cursor + 8U;
        size_t next = payload + (size_t)chunk_size;

        if (next > size) {
            openvn_8svx_free(sample);
            free(data);
            return 0;
        }

        if (memcmp(chunk_id, "VHDR", 4U) == 0) {
            if (chunk_size < 20U) {
                openvn_8svx_free(sample);
                free(data);
                return 0;
            }

            sample->sample_rate = read_u16_be(data + payload + 12U);
            sample->octave_count = data[payload + 14U];
            sample->compression = data[payload + 15U];
            sample->volume = read_u32_be(data + payload + 16U);
            have_vhdr = 1;
        } else if (memcmp(chunk_id, "BODY", 4U) == 0) {
            sample->data = (unsigned char *)malloc((size_t)chunk_size);
            if (sample->data == 0) {
                openvn_8svx_free(sample);
                free(data);
                return 0;
            }

            memcpy(sample->data, data + payload, (size_t)chunk_size);
            sample->data_size = (size_t)chunk_size;
            have_body = 1;
        }

        cursor = next + (chunk_size & 1UL);
    }

    free(data);

    if (!have_vhdr || !have_body ||
        sample->sample_rate == 0U ||
        sample->data_size == 0U ||
        sample->compression != 0U) {
        openvn_8svx_free(sample);
        return 0;
    }

    return 1;
}
