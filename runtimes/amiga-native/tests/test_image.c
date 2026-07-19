#include "openvn_image.h"

#include "test_check.h"

int main(int argc, char **argv) {
    OpenVNILBMImage image = {0};

    OPENVN_TEST_CHECK(argc == 2);
    OPENVN_TEST_CHECK(openvn_ilbm_load_file(&image, argv[1]));
    OPENVN_TEST_CHECK(image.width == 16U);
    OPENVN_TEST_CHECK(image.height == 16U);
    OPENVN_TEST_CHECK(image.depth == 1U);
    OPENVN_TEST_CHECK(image.palette_size == 6U);
    OPENVN_TEST_CHECK(image.body_size > 0U);

    openvn_ilbm_free(&image);
    return 0;
}
