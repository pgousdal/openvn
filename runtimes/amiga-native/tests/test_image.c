#include "openvn_image.h"

#include <assert.h>

int main(int argc, char **argv) {
    OpenVNILBMImage image;

    assert(argc == 2);
    assert(openvn_ilbm_load_file(&image, argv[1]));
    assert(image.width == 16U);
    assert(image.height == 16U);
    assert(image.depth == 1U);
    assert(image.palette_size == 6U);
    assert(image.body_size > 0U);

    openvn_ilbm_free(&image);
    return 0;
}
