#include "openvn_8svx.h"

#include "test_check.h"

int main(int argc, char **argv) {
    OpenVN8SVXSample sample = {0};

    OPENVN_TEST_CHECK(argc == 2);
    OPENVN_TEST_CHECK(openvn_8svx_load_file(&sample, argv[1]));
    OPENVN_TEST_CHECK(sample.sample_rate == 8000U);
    OPENVN_TEST_CHECK(sample.octave_count == 1U);
    OPENVN_TEST_CHECK(sample.compression == 0U);
    OPENVN_TEST_CHECK(sample.data_size == 32U);

    openvn_8svx_free(&sample);
    return 0;
}
