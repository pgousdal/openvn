#include "openvn_bitmap_font.h"

#include "test_check.h"
#include <stdio.h>
#include <string.h>

typedef struct PlotCapture {
    unsigned char pixels[64][128];
    int count;
} PlotCapture;

static void capture_plot(void *context, int x, int y) {
    PlotCapture *capture;

    capture = (PlotCapture *)context;
    if (x >= 0 && x < 128 && y >= 0 && y < 64) {
        capture->pixels[y][x] = 1U;
    }
    ++capture->count;
}

static void test_measure_single_line(void) {
    OpenVNBitmapFontMetrics metrics;

    openvn_bitmap_font_measure("Anna", &metrics);
    OPENVN_TEST_CHECK(metrics.width == 32U);
    OPENVN_TEST_CHECK(metrics.height == 14U);
    OPENVN_TEST_CHECK(metrics.lines == 1U);
}

static void test_measure_multiple_lines(void) {
    OpenVNBitmapFontMetrics metrics;

    openvn_bitmap_font_measure("Anna\nHello", &metrics);
    OPENVN_TEST_CHECK(metrics.width == 40U);
    OPENVN_TEST_CHECK(metrics.height == 28U);
    OPENVN_TEST_CHECK(metrics.lines == 2U);
}

static void test_draw_produces_pixels_and_newline(void) {
    PlotCapture capture;
    int pixels;
    int first_line;
    int second_line;
    int x;
    int y;

    memset(&capture, 0, sizeof(capture));
    pixels = openvn_bitmap_font_draw(
        "A\nB",
        3,
        4,
        capture_plot,
        &capture
    );

    OPENVN_TEST_CHECK(pixels > 0);
    OPENVN_TEST_CHECK(pixels == capture.count);

    first_line = 0;
    second_line = 0;
    for (y = 0; y < 64; ++y) {
        for (x = 0; x < 128; ++x) {
            if (capture.pixels[y][x] != 0U) {
                if (y < 4 + (int)OPENVN_BITMAP_FONT_LINE_HEIGHT) {
                    first_line = 1;
                } else {
                    second_line = 1;
                }
            }
        }
    }

    OPENVN_TEST_CHECK(first_line);
    OPENVN_TEST_CHECK(second_line);
}

static void test_unknown_character_uses_fallback(void) {
    int question_pixels;
    int unknown_pixels;
    unsigned int x;
    unsigned int y;

    question_pixels = 0;
    unknown_pixels = 0;
    for (y = 0U; y < OPENVN_BITMAP_FONT_GLYPH_HEIGHT; ++y) {
        for (x = 0U; x < OPENVN_BITMAP_FONT_GLYPH_WIDTH; ++x) {
            question_pixels += openvn_bitmap_font_glyph_pixel('?', x, y);
            unknown_pixels += openvn_bitmap_font_glyph_pixel(1U, x, y);
        }
    }
    OPENVN_TEST_CHECK(question_pixels > 0);
    OPENVN_TEST_CHECK(question_pixels == unknown_pixels);
}

int main(void) {
    test_measure_single_line();
    test_measure_multiple_lines();
    test_draw_produces_pixels_and_newline();
    test_unknown_character_uses_fallback();
    puts("bitmap font tests passed");
    return 0;
}
