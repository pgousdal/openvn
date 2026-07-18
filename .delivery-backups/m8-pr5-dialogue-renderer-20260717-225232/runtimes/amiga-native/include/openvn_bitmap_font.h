#ifndef OPENVN_BITMAP_FONT_H
#define OPENVN_BITMAP_FONT_H

#define OPENVN_BITMAP_FONT_FIRST 32U
#define OPENVN_BITMAP_FONT_LAST 126U
#define OPENVN_BITMAP_FONT_GLYPH_WIDTH 8U
#define OPENVN_BITMAP_FONT_GLYPH_HEIGHT 12U
#define OPENVN_BITMAP_FONT_ADVANCE 8U
#define OPENVN_BITMAP_FONT_LINE_HEIGHT 14U

typedef void (*OpenVNBitmapFontPlot)(
    void *context,
    int x,
    int y
);

typedef struct OpenVNBitmapFontMetrics {
    unsigned int width;
    unsigned int height;
    unsigned int lines;
} OpenVNBitmapFontMetrics;

int openvn_bitmap_font_glyph_pixel(
    unsigned int character,
    unsigned int x,
    unsigned int y
);

void openvn_bitmap_font_measure(
    const char *text,
    OpenVNBitmapFontMetrics *metrics
);

int openvn_bitmap_font_draw(
    const char *text,
    int x,
    int y,
    OpenVNBitmapFontPlot plot,
    void *context
);

#ifdef __AMIGA__
struct RastPort;
int openvn_bitmap_font_draw_amiga(
    struct RastPort *rastport,
    const char *text,
    int x,
    int y,
    unsigned int pen
);
#endif

#endif
