#ifndef OPENVN_MOD_H
#define OPENVN_MOD_H

#include <stddef.h>

#define OPENVN_MOD_CHANNELS 4U
#define OPENVN_MOD_ROWS 64U
#define OPENVN_MOD_MAX_PATTERNS 128U
#define OPENVN_MOD_SAMPLE_COUNT 31U

typedef struct OpenVNMODSample {
    char name[23];
    unsigned long length;
    unsigned char finetune;
    unsigned char volume;
    unsigned long loop_start;
    unsigned long loop_length;
    unsigned char *data;
} OpenVNMODSample;

typedef struct OpenVNMODNote {
    unsigned short period;
    unsigned char sample;
    unsigned char effect;
    unsigned char parameter;
} OpenVNMODNote;

typedef struct OpenVNMODModule {
    char title[21];
    OpenVNMODSample samples[OPENVN_MOD_SAMPLE_COUNT];
    unsigned char song_length;
    unsigned char restart_position;
    unsigned char pattern_table[128];
    unsigned int pattern_count;
    OpenVNMODNote *patterns;
} OpenVNMODModule;

void openvn_mod_reset(OpenVNMODModule *module);
void openvn_mod_free(OpenVNMODModule *module);

int openvn_mod_load_file(
    OpenVNMODModule *module,
    const char *path
);

const OpenVNMODNote *openvn_mod_note(
    const OpenVNMODModule *module,
    unsigned int pattern,
    unsigned int row,
    unsigned int channel
);

#endif
