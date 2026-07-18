#include "openvn_bitmap_font.h"

#include <assert.h>
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
    assert(metrics.width == 32U);
    assert(metrics.height == 14U);
    assert(metrics.lines == 1U);
}

static void test_measure_multiple_lines(void) {
    OpenVNBitmapFontMetrics metrics;

    openvn_bitmap_font_measure("Anna\nHello", &metrics);
    assert(metrics.width == 40U);
    assert(metrics.height == 28U);
    assert(metrics.lines == 2U);
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

    assert(pixels > 0);
    assert(pixels == capture.count);

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

    assert(first_line);
    assert(second_line);
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
    assert(question_pixels > 0);
    assert(question_pixels == unknown_pixels);
}

int main(void) {
    test_measure_single_line();
    test_measure_multiple_lines();
    test_draw_produces_pixels_and_newline();
    test_unknown_character_uses_fallback();
    puts("bitmap font tests passed");
    return 0;
}
