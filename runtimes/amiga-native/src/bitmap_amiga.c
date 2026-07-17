#include "openvn_bitmap_amiga.h"

#ifdef __AMIGA__

#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <string.h>

#define OPENVN_RENDER_LOG "openvn-render.log"

static void bitmap_trace(const char *message) {
    FILE *stream;

    stream = fopen(OPENVN_RENDER_LOG, "a");
    if (stream == 0) {
        return;
    }

    fputs(message, stream);
    fputc('\n', stream);
    fclose(stream);
}

static void bitmap_trace_uint(const char *prefix, unsigned int value) {
    FILE *stream;

    stream = fopen(OPENVN_RENDER_LOG, "a");
    if (stream == 0) {
        return;
    }

    fprintf(stream, "%s%u\n", prefix, value);
    fclose(stream);
}

static struct BitMap *allocate_classic_bitmap(
    unsigned int width,
    unsigned int height,
    unsigned int depth
) {
    struct BitMap *bitmap;
    unsigned int plane;

    bitmap = (struct BitMap *)AllocMem(
        sizeof(struct BitMap),
        MEMF_PUBLIC | MEMF_CLEAR
    );
    if (bitmap == 0) {
        return 0;
    }

    InitBitMap(bitmap, depth, width, height);

    for (plane = 0U; plane < depth; plane++) {
        bitmap->Planes[plane] = (PLANEPTR)AllocRaster(width, height);
        if (bitmap->Planes[plane] == 0) {
            while (plane > 0U) {
                plane--;
                FreeRaster(bitmap->Planes[plane], width, height);
                bitmap->Planes[plane] = 0;
            }

            FreeMem(bitmap, sizeof(struct BitMap));
            return 0;
        }

        BltClear(
            bitmap->Planes[plane],
            (ULONG)bitmap->BytesPerRow * (ULONG)bitmap->Rows,
            0
        );
        WaitBlit();
    }

    return bitmap;
}

static void free_classic_bitmap(
    struct BitMap *bitmap,
    unsigned int width,
    unsigned int height,
    unsigned int depth
) {
    unsigned int plane;

    if (bitmap == 0) {
        return;
    }

    for (plane = 0U; plane < depth; plane++) {
        if (bitmap->Planes[plane] != 0) {
            FreeRaster(bitmap->Planes[plane], width, height);
            bitmap->Planes[plane] = 0;
        }
    }

    FreeMem(bitmap, sizeof(struct BitMap));
}

void openvn_amiga_bitmap_reset(OpenVNAmigaBitmap *bitmap) {
    if (bitmap != 0) {
        memset(bitmap, 0, sizeof(*bitmap));
    }
}

void openvn_amiga_bitmap_free(OpenVNAmigaBitmap *bitmap) {
    if (bitmap == 0) {
        return;
    }

    if (bitmap->mask_bitmap != 0) {
        free_classic_bitmap(
            bitmap->mask_bitmap,
            bitmap->width,
            bitmap->height,
            1U
        );
    }

    if (bitmap->bitmap != 0) {
        free_classic_bitmap(
            bitmap->bitmap,
            bitmap->width,
            bitmap->height,
            bitmap->depth
        );
    }

    openvn_amiga_bitmap_reset(bitmap);
}

static int copy_planes(
    struct BitMap *destination,
    const OpenVNPlanarBitmap *source
) {
    unsigned int plane;

    bitmap_trace_uint("AMIGA BITMAP copy plane count=", source->depth);

    for (plane = 0U; plane < source->depth; plane++) {
        bitmap_trace_uint("AMIGA BITMAP copy plane begin=", plane);

        if (destination->Planes[plane] == 0) {
            bitmap_trace_uint("AMIGA BITMAP failed: null plane=", plane);
            return 0;
        }

        CopyMem(
            source->planes[plane],
            destination->Planes[plane],
            source->plane_size
        );

        bitmap_trace_uint("AMIGA BITMAP copy plane ok=", plane);
    }

    return 1;
}

int openvn_amiga_bitmap_from_planar(
    OpenVNAmigaBitmap *destination,
    const OpenVNPlanarBitmap *source
) {
    if (destination == 0 || source == 0 ||
        source->planes == 0 || source->depth == 0U) {
        bitmap_trace("AMIGA BITMAP failed: invalid input");
        return 0;
    }

    openvn_amiga_bitmap_reset(destination);

    bitmap_trace_uint("AMIGA BITMAP width=", source->width);
    bitmap_trace_uint("AMIGA BITMAP height=", source->height);
    bitmap_trace_uint("AMIGA BITMAP depth=", source->depth);
    bitmap_trace_uint("AMIGA BITMAP bytes per row=", source->bytes_per_row);
    bitmap_trace_uint("AMIGA BITMAP plane size=", (unsigned int)source->plane_size);
    bitmap_trace("AMIGA BITMAP classic allocation begin");

    destination->bitmap = allocate_classic_bitmap(
        source->width,
        source->height,
        source->depth
    );

    bitmap_trace("AMIGA BITMAP classic allocation returned");

    if (destination->bitmap == 0) {
        bitmap_trace("AMIGA BITMAP failed: classic allocation returned null");
        return 0;
    }

    bitmap_trace_uint(
        "AMIGA BITMAP destination bytes per row=",
        destination->bitmap->BytesPerRow
    );
    bitmap_trace_uint(
        "AMIGA BITMAP destination rows=",
        destination->bitmap->Rows
    );
    bitmap_trace("AMIGA BITMAP plane copy begin");

    if (!copy_planes(destination->bitmap, source)) {
        bitmap_trace("AMIGA BITMAP failed: plane copy");
        destination->width = source->width;
        destination->height = source->height;
        destination->depth = source->depth;
        openvn_amiga_bitmap_free(destination);
        return 0;
    }

    bitmap_trace("AMIGA BITMAP plane copy ok");

    if (source->mask != 0) {
        bitmap_trace("AMIGA BITMAP mask classic allocation begin");

        destination->mask_bitmap = allocate_classic_bitmap(
            source->width,
            source->height,
            1U
        );

        bitmap_trace("AMIGA BITMAP mask classic allocation returned");

        if (destination->mask_bitmap == 0) {
            bitmap_trace("AMIGA BITMAP failed: mask allocation returned null");
            destination->width = source->width;
            destination->height = source->height;
            destination->depth = source->depth;
            openvn_amiga_bitmap_free(destination);
            return 0;
        }

        bitmap_trace("AMIGA BITMAP mask copy begin");
        CopyMem(
            source->mask,
            destination->mask_bitmap->Planes[0],
            source->plane_size
        );
        bitmap_trace("AMIGA BITMAP mask copy ok");
    }

    destination->width = source->width;
    destination->height = source->height;
    destination->depth = source->depth;
    bitmap_trace("AMIGA BITMAP conversion complete");
    return 1;
}

static unsigned int bitmap_pixel_width(const struct BitMap *bitmap) {
    if (bitmap == 0) {
        return 0U;
    }

    return (unsigned int)bitmap->BytesPerRow * 8U;
}

int openvn_amiga_bitmap_blit(
    const OpenVNAmigaBitmap *source,
    struct RastPort *destination,
    int x,
    int y,
    int masked
) {
    int source_x;
    int source_y;
    unsigned int width;
    unsigned int height;
    unsigned int destination_width;
    unsigned int destination_height;
    unsigned int plane;
    ULONG plane_size;

    if (source == 0 || source->bitmap == 0 ||
        destination == 0 || destination->BitMap == 0) {
        bitmap_trace("AMIGA BLIT failed: invalid input");
        return 0;
    }

    /*
     * Full-screen OCS backgrounds have identical source and destination
     * geometry. Copy their planes directly instead of invoking the hardware
     * blitter. This avoids the Graphics.library/WaitBlit path that stalls on
     * the current classic Amiga configuration.
     */
    if (!masked &&
        x == 0 &&
        y == 0 &&
        source->width ==
            (unsigned int)destination->BitMap->BytesPerRow * 8U &&
        source->height == (unsigned int)destination->BitMap->Rows &&
        source->bitmap->BytesPerRow == destination->BitMap->BytesPerRow &&
        source->bitmap->Rows == destination->BitMap->Rows) {
        unsigned int depth;

        depth = source->depth;
        if (depth > (unsigned int)destination->BitMap->Depth) {
            depth = (unsigned int)destination->BitMap->Depth;
        }

        plane_size =
            (ULONG)destination->BitMap->BytesPerRow *
            (ULONG)destination->BitMap->Rows;

        bitmap_trace("AMIGA BLIT direct fullscreen copy begin");
        bitmap_trace_uint("AMIGA BLIT direct plane count=", depth);
        bitmap_trace_uint(
            "AMIGA BLIT direct plane size=",
            (unsigned int)plane_size
        );

        for (plane = 0U; plane < depth; plane++) {
            if (source->bitmap->Planes[plane] == 0 ||
                destination->BitMap->Planes[plane] == 0) {
                bitmap_trace_uint(
                    "AMIGA BLIT failed: null direct plane=",
                    plane
                );
                return 0;
            }

            bitmap_trace_uint(
                "AMIGA BLIT direct plane begin=",
                plane
            );
            CopyMem(
                source->bitmap->Planes[plane],
                destination->BitMap->Planes[plane],
                plane_size
            );
            bitmap_trace_uint(
                "AMIGA BLIT direct plane ok=",
                plane
            );
        }

        bitmap_trace("AMIGA BLIT direct fullscreen copy ok");
        return 1;
    }

    source_x = 0;
    source_y = 0;
    width = source->width;
    height = source->height;
    destination_width = bitmap_pixel_width(destination->BitMap);
    destination_height = (unsigned int)destination->BitMap->Rows;

    if (x < 0) {
        source_x = -x;
        if ((unsigned int)source_x >= width) {
            return 1;
        }
        width -= (unsigned int)source_x;
        x = 0;
    }

    if (y < 0) {
        source_y = -y;
        if ((unsigned int)source_y >= height) {
            return 1;
        }
        height -= (unsigned int)source_y;
        y = 0;
    }

    if ((unsigned int)x >= destination_width ||
        (unsigned int)y >= destination_height) {
        return 1;
    }

    if (width > destination_width - (unsigned int)x) {
        width = destination_width - (unsigned int)x;
    }

    if (height > destination_height - (unsigned int)y) {
        height = destination_height - (unsigned int)y;
    }

    if (width == 0U || height == 0U) {
        return 1;
    }

    bitmap_trace("AMIGA BLIT hardware path begin");

    if (masked && source->mask_bitmap != 0) {
        BltMaskBitMapRastPort(
            source->bitmap,
            source_x,
            source_y,
            destination,
            x,
            y,
            width,
            height,
            0xE0,
            source->mask_bitmap->Planes[0]
        );
    } else {
        BltBitMapRastPort(
            source->bitmap,
            source_x,
            source_y,
            destination,
            x,
            y,
            width,
            height,
            0xC0
        );
    }

    bitmap_trace("AMIGA BLIT hardware call returned");
    WaitBlit();
    bitmap_trace("AMIGA BLIT hardware wait ok");
    return 1;
}

#endif
