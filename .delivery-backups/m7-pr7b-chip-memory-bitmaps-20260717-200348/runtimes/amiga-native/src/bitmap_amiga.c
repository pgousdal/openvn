#include "openvn_bitmap_amiga.h"

#ifdef __AMIGA__

#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/graphics.h>

#include <string.h>

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
        FreeBitMap(bitmap->mask_bitmap);
    }

    if (bitmap->bitmap != 0) {
        FreeBitMap(bitmap->bitmap);
    }

    openvn_amiga_bitmap_reset(bitmap);
}

static int copy_planes(
    struct BitMap *destination,
    const OpenVNPlanarBitmap *source
) {
    unsigned int plane;

    for (plane = 0U; plane < source->depth; plane++) {
        if (destination->Planes[plane] == 0) {
            return 0;
        }

        CopyMem(
            source->planes[plane],
            destination->Planes[plane],
            source->plane_size
        );
    }

    return 1;
}

int openvn_amiga_bitmap_from_planar(
    OpenVNAmigaBitmap *destination,
    const OpenVNPlanarBitmap *source
) {
    if (destination == 0 || source == 0 ||
        source->planes == 0 || source->depth == 0U) {
        return 0;
    }

    openvn_amiga_bitmap_reset(destination);

    destination->bitmap = AllocBitMap(
        source->width,
        source->height,
        source->depth,
        BMF_CLEAR,
        0
    );

    if (destination->bitmap == 0) {
        return 0;
    }

    if (!copy_planes(destination->bitmap, source)) {
        openvn_amiga_bitmap_free(destination);
        return 0;
    }

    if (source->mask != 0) {
        destination->mask_bitmap = AllocBitMap(
            source->width,
            source->height,
            1U,
            BMF_CLEAR,
            0
        );

        if (destination->mask_bitmap == 0) {
            openvn_amiga_bitmap_free(destination);
            return 0;
        }

        CopyMem(
            source->mask,
            destination->mask_bitmap->Planes[0],
            source->plane_size
        );
    }

    destination->width = source->width;
    destination->height = source->height;
    destination->depth = source->depth;
    return 1;
}

static unsigned int bitmap_pixel_width(const struct BitMap *bitmap) {
    if (bitmap == 0) {
        return 0U;
    }

    /*
     * Amiga BitMap does not store a logical pixel width. BytesPerRow is
     * word-aligned, so this is the safe writable width of the destination.
     */
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

    if (source == 0 || source->bitmap == 0 ||
        destination == 0 || destination->BitMap == 0) {
        return 0;
    }

    source_x = 0;
    source_y = 0;
    width = source->width;
    height = source->height;
    destination_width = bitmap_pixel_width(destination->BitMap);
    destination_height = (unsigned int)destination->BitMap->Rows;

    /*
     * Clip before invoking the blitter. The demo currently contains a
     * 640x360 background while the OCS screen is 320x256. Passing the source
     * dimensions unchanged can make the hardware blitter write beyond the
     * destination bitmap and leave WaitBlit() waiting indefinitely.
     */
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

    WaitBlit();
    return 1;
}

#endif
