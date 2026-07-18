#include "openvn_bitmap_font.h"

#ifdef __AMIGA__

#include <graphics/rastport.h>
#include <proto/graphics.h>

#include <stddef.h>

typedef struct OpenVNAmigaFontPlotContext {
    struct RastPort *rastport;
} OpenVNAmigaFontPlotContext;

static void amiga_plot(void *context, int x, int y) {
    OpenVNAmigaFontPlotContext *amiga_context;

    amiga_context = (OpenVNAmigaFontPlotContext *)context;
    WritePixel(amiga_context->rastport, x, y);
}

int openvn_bitmap_font_draw_amiga(
    struct RastPort *rastport,
    const char *text,
    int x,
    int y,
    unsigned int pen
) {
    OpenVNAmigaFontPlotContext context;

    if (rastport == NULL || text == NULL) {
        return 0;
    }

    SetAPen(rastport, (ULONG)pen);
    context.rastport = rastport;
    return openvn_bitmap_font_draw(text, x, y, amiga_plot, &context);
}

#endif
