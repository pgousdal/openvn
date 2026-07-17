#include "openvn_image.h"
#include "openvn_planar.h"

#include <assert.h>
#include <string.h>

static void test_basic_pixels_and_mask(void) {
    OpenVNILBMImage image;
    OpenVNPlanarBitmap bitmap;
    unsigned char pixels[] = {
        0U, 1U, 2U, 3U,
        3U, 2U, 1U, 0U
    };

    openvn_ilbm_reset(&image);
    openvn_planar_reset(&bitmap);
    image.width = 4U;
    image.height = 2U;
    image.depth = 2U;
    image.body = pixels;
    image.body_size = sizeof(pixels);

    assert(openvn_planar_from_chunky(&bitmap, &image, 1, 0U));

    assert(bitmap.bytes_per_row == 2U);
    assert(bitmap.plane_size == 4U);

    assert(openvn_planar_pixel(&bitmap, 0U, 0U) == 0U);
    assert(openvn_planar_pixel(&bitmap, 1U, 0U) == 1U);
    assert(openvn_planar_pixel(&bitmap, 2U, 0U) == 2U);
    assert(openvn_planar_pixel(&bitmap, 3U, 0U) == 3U);
    assert(openvn_planar_pixel(&bitmap, 0U, 1U) == 3U);
    assert(openvn_planar_pixel(&bitmap, 3U, 1U) == 0U);

    assert(!openvn_planar_mask_pixel(&bitmap, 0U, 0U));
    assert(openvn_planar_mask_pixel(&bitmap, 1U, 0U));
    assert(openvn_planar_mask_pixel(&bitmap, 2U, 0U));
    assert(openvn_planar_mask_pixel(&bitmap, 3U, 0U));

    openvn_planar_free(&bitmap);
}

static void test_crosses_byte_and_word_boundaries(void) {
    OpenVNILBMImage image;
    OpenVNPlanarBitmap bitmap;
    unsigned char pixels[17];
    unsigned int x;

    for (x = 0U; x < 17U; x++) {
        pixels[x] = (unsigned char)(x & 7U);
    }

    openvn_ilbm_reset(&image);
    openvn_planar_reset(&bitmap);
    image.width = 17U;
    image.height = 1U;
    image.depth = 3U;
    image.body = pixels;
    image.body_size = sizeof(pixels);

    assert(openvn_planar_from_chunky(&bitmap, &image, 0, 0U));
    assert(bitmap.bytes_per_row == 4U);

    for (x = 0U; x < 17U; x++) {
        assert(openvn_planar_pixel(&bitmap, x, 0U) == (x & 7U));
    }

    openvn_planar_free(&bitmap);
}

int main(void) {
    test_basic_pixels_and_mask();
    test_crosses_byte_and_word_boundaries();
    return 0;
}
