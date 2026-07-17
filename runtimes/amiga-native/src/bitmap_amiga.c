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

int openvn_amiga_bitmap_blit(
    const OpenVNAmigaBitmap *source,
    struct RastPort *destination,
    int x,
    int y,
    int masked
) {
    if (source == 0 || source->bitmap == 0 || destination == 0) {
        return 0;
    }

    if (masked && source->mask_bitmap != 0) {
        BltMaskBitMapRastPort(
            source->bitmap,
            0,
            0,
            destination,
            x,
            y,
            source->width,
            source->height,
            0xE0,
            source->mask_bitmap->Planes[0]
        );
    } else {
        BltBitMapRastPort(
            source->bitmap,
            0,
            0,
            destination,
            x,
            y,
            source->width,
            source->height,
            0xC0
        );
    }

    WaitBlit();
    return 1;
}

#endif
