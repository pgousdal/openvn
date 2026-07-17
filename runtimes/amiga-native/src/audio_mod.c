#include "openvn_mod.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned short read_u16_be(const unsigned char *data) {
    return (unsigned short)(
        ((unsigned short)data[0] << 8) |
        (unsigned short)data[1]
    );
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

void openvn_mod_reset(OpenVNMODModule *module) {
    if (module != 0) {
        memset(module, 0, sizeof(*module));
    }
}

void openvn_mod_free(OpenVNMODModule *module) {
    unsigned int sample;

    if (module == 0) {
        return;
    }

    free(module->patterns);

    for (sample = 0U; sample < OPENVN_MOD_SAMPLE_COUNT; sample++) {
        free(module->samples[sample].data);
    }

    openvn_mod_reset(module);
}

static int signature_supported(const unsigned char *signature) {
    return memcmp(signature, "M.K.", 4U) == 0 ||
           memcmp(signature, "M!K!", 4U) == 0 ||
           memcmp(signature, "4CHN", 4U) == 0 ||
           memcmp(signature, "FLT4", 4U) == 0;
}

int openvn_mod_load_file(
    OpenVNMODModule *module,
    const char *path
) {
    unsigned char *data;
    size_t size;
    size_t cursor;
    unsigned int sample;
    unsigned int pattern;
    unsigned int row;
    unsigned int channel;
    unsigned int highest_pattern;
    size_t pattern_bytes;

    if (module == 0 || path == 0) {
        return 0;
    }

    openvn_mod_reset(module);

    if (!read_file(path, &data, &size)) {
        return 0;
    }

    if (size < 1084U || !signature_supported(data + 1080U)) {
        free(data);
        return 0;
    }

    memcpy(module->title, data, 20U);
    module->title[20] = '\0';

    cursor = 20U;
    for (sample = 0U; sample < OPENVN_MOD_SAMPLE_COUNT; sample++) {
        OpenVNMODSample *destination = &module->samples[sample];

        memcpy(destination->name, data + cursor, 22U);
        destination->name[22] = '\0';
        destination->length =
            (unsigned long)read_u16_be(data + cursor + 22U) * 2UL;
        destination->finetune = data[cursor + 24U] & 0x0FU;
        destination->volume = data[cursor + 25U];
        destination->loop_start =
            (unsigned long)read_u16_be(data + cursor + 26U) * 2UL;
        destination->loop_length =
            (unsigned long)read_u16_be(data + cursor + 28U) * 2UL;

        if (destination->volume > 64U) {
            free(data);
            return 0;
        }

        cursor += 30U;
    }

    module->song_length = data[950U];
    module->restart_position = data[951U];
    memcpy(module->pattern_table, data + 952U, 128U);

    if (module->song_length == 0U || module->song_length > 128U) {
        free(data);
        return 0;
    }

    highest_pattern = 0U;
    for (pattern = 0U; pattern < module->song_length; pattern++) {
        if (module->pattern_table[pattern] > highest_pattern) {
            highest_pattern = module->pattern_table[pattern];
        }
    }

    module->pattern_count = highest_pattern + 1U;
    if (module->pattern_count > OPENVN_MOD_MAX_PATTERNS) {
        free(data);
        return 0;
    }

    pattern_bytes = (size_t)module->pattern_count * 1024U;
    if (1084U + pattern_bytes > size) {
        free(data);
        return 0;
    }

    module->patterns = (OpenVNMODNote *)calloc(
        (size_t)module->pattern_count *
            OPENVN_MOD_ROWS *
            OPENVN_MOD_CHANNELS,
        sizeof(OpenVNMODNote)
    );
    if (module->patterns == 0) {
        free(data);
        return 0;
    }

    cursor = 1084U;
    for (pattern = 0U; pattern < module->pattern_count; pattern++) {
        for (row = 0U; row < OPENVN_MOD_ROWS; row++) {
            for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
                unsigned char a = data[cursor++];
                unsigned char b = data[cursor++];
                unsigned char c = data[cursor++];
                unsigned char d = data[cursor++];
                OpenVNMODNote *note = &module->patterns[
                    ((size_t)pattern * OPENVN_MOD_ROWS + row) *
                        OPENVN_MOD_CHANNELS +
                    channel
                ];

                note->sample = (unsigned char)(
                    (a & 0xF0U) | ((c & 0xF0U) >> 4)
                );
                note->period = (unsigned short)(
                    ((unsigned short)(a & 0x0FU) << 8) |
                    (unsigned short)b
                );
                note->effect = c & 0x0FU;
                note->parameter = d;
            }
        }
    }

    for (sample = 0U; sample < OPENVN_MOD_SAMPLE_COUNT; sample++) {
        OpenVNMODSample *destination = &module->samples[sample];

        if (destination->length == 0UL) {
            continue;
        }

        if (cursor + destination->length > size) {
            openvn_mod_free(module);
            free(data);
            return 0;
        }

        destination->data = (unsigned char *)malloc(
            (size_t)destination->length
        );
        if (destination->data == 0) {
            openvn_mod_free(module);
            free(data);
            return 0;
        }

        memcpy(
            destination->data,
            data + cursor,
            (size_t)destination->length
        );
        cursor += (size_t)destination->length;
    }

    free(data);
    return 1;
}

const OpenVNMODNote *openvn_mod_note(
    const OpenVNMODModule *module,
    unsigned int pattern,
    unsigned int row,
    unsigned int channel
) {
    if (module == 0 || module->patterns == 0 ||
        pattern >= module->pattern_count ||
        row >= OPENVN_MOD_ROWS ||
        channel >= OPENVN_MOD_CHANNELS) {
        return 0;
    }

    return &module->patterns[
        ((size_t)pattern * OPENVN_MOD_ROWS + row) *
            OPENVN_MOD_CHANNELS +
        channel
    ];
}
