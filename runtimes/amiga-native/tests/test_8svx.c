#include "openvn_8svx.h"

#include <assert.h>

int main(int argc, char **argv) {
    OpenVN8SVXSample sample;

    assert(argc == 2);
    assert(openvn_8svx_load_file(&sample, argv[1]));
    assert(sample.sample_rate == 8000U);
    assert(sample.octave_count == 1U);
    assert(sample.compression == 0U);
    assert(sample.data_size == 32U);

    openvn_8svx_free(&sample);
    return 0;
}
