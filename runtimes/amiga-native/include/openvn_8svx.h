#ifndef OPENVN_8SVX_H
#define OPENVN_8SVX_H

#include <stddef.h>

typedef struct OpenVN8SVXSample {
    unsigned int sample_rate;
    unsigned int octave_count;
    unsigned int compression;
    unsigned long volume;
    unsigned char *data;
    size_t data_size;
} OpenVN8SVXSample;

void openvn_8svx_reset(OpenVN8SVXSample *sample);
void openvn_8svx_free(OpenVN8SVXSample *sample);

int openvn_8svx_load_file(
    OpenVN8SVXSample *sample,
    const char *path
);

#endif
