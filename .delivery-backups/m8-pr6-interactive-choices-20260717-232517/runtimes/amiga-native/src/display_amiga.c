#include "openvn_display_amiga.h"

#ifdef __AMIGA__

#include <exec/ports.h>
#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#include <string.h>

void openvn_amiga_display_reset(OpenVNAmigaDisplay *display) {
    if (display != 0) {
        memset(display, 0, sizeof(*display));
    }
}

static void drain_port(struct MsgPort *port) {
    if (port == 0) {
        return;
    }

    while (GetMsg(port) != 0) {
    }
}

static int create_buffers(OpenVNAmigaDisplay *display) {
    display->buffers[0] = AllocScreenBuffer(
        display->screen,
        0,
        SB_SCREEN_BITMAP
    );
    if (display->buffers[0] == 0) {
        return 0;
    }

    display->buffers[1] = AllocScreenBuffer(
        display->screen,
        0,
        0
    );
    if (display->buffers[1] == 0) {
        return 0;
    }

    display->safe_port = CreateMsgPort();
    display->display_port = CreateMsgPort();

    if (display->safe_port == 0 || display->display_port == 0) {
        return 0;
    }

    display->buffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort =
        display->safe_port;
    display->buffers[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort =
        display->display_port;
    display->buffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort =
        display->safe_port;
    display->buffers[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort =
        display->display_port;

    display->visible_index = 0U;
    display->draw_index = 1U;
    display->double_buffered = 1;
    return 1;
}

int openvn_amiga_display_open(
    OpenVNAmigaDisplay *display,
    unsigned int width,
    unsigned int height,
    unsigned int depth,
    int fullscreen,
    int double_buffered
) {
    if (display == 0) {
        return 0;
    }

    openvn_amiga_display_reset(display);

    display->screen = OpenScreenTags(
        0,
        SA_Width,
        width,
        SA_Height,
        height,
        SA_Depth,
        depth,
        SA_Title,
        (ULONG)"OpenVN",
        SA_ShowTitle,
        FALSE,
        TAG_DONE
    );

    if (display->screen == 0) {
        return 0;
    }

    display->window = OpenWindowTags(
        0,
        WA_CustomScreen,
        (ULONG)display->screen,
        WA_Left,
        0,
        WA_Top,
        0,
        WA_Width,
        width,
        WA_Height,
        height,
        WA_Borderless,
        TRUE,
        WA_Backdrop,
        fullscreen ? TRUE : FALSE,
        WA_Activate,
        TRUE,
        TAG_DONE
    );

    if (display->window == 0) {
        openvn_amiga_display_close(display);
        return 0;
    }

    if (double_buffered && !create_buffers(display)) {
        openvn_amiga_display_close(display);
        return 0;
    }

    return 1;
}

void openvn_amiga_display_close(OpenVNAmigaDisplay *display) {
    if (display == 0) {
        return;
    }

    if (display->swap_pending && display->safe_port != 0) {
        WaitPort(display->safe_port);
        drain_port(display->safe_port);
        display->swap_pending = 0;
    }

    if (display->buffers[1] != 0 && display->screen != 0) {
        FreeScreenBuffer(display->screen, display->buffers[1]);
        display->buffers[1] = 0;
    }

    if (display->buffers[0] != 0 && display->screen != 0) {
        FreeScreenBuffer(display->screen, display->buffers[0]);
        display->buffers[0] = 0;
    }

    if (display->safe_port != 0) {
        DeleteMsgPort(display->safe_port);
        display->safe_port = 0;
    }

    if (display->display_port != 0) {
        DeleteMsgPort(display->display_port);
        display->display_port = 0;
    }

    if (display->window != 0) {
        CloseWindow(display->window);
        display->window = 0;
    }

    if (display->screen != 0) {
        CloseScreen(display->screen);
        display->screen = 0;
    }

    openvn_amiga_display_reset(display);
}

struct RastPort *openvn_amiga_display_draw_rastport(
    OpenVNAmigaDisplay *display
) {
    if (display == 0 || display->window == 0) {
        return 0;
    }

    if (!display->double_buffered) {
        return display->window->RPort;
    }

    display->window->RPort->BitMap =
        display->buffers[display->draw_index]->sb_BitMap;
    return display->window->RPort;
}

int openvn_amiga_display_present(OpenVNAmigaDisplay *display) {
    unsigned int old_visible;

    if (display == 0 || display->screen == 0) {
        return 0;
    }

    if (!display->double_buffered) {
        WaitBlit();
        return 1;
    }

    if (display->swap_pending) {
        WaitPort(display->safe_port);
        drain_port(display->safe_port);
        display->swap_pending = 0;
    }

    old_visible = display->visible_index;

    if (!ChangeScreenBuffer(
            display->screen,
            display->buffers[display->draw_index]
        )) {
        return 0;
    }

    display->visible_index = display->draw_index;
    display->draw_index = old_visible;
    display->swap_pending = 1;

    if (display->display_port != 0) {
        WaitPort(display->display_port);
        drain_port(display->display_port);
    }

    return 1;
}

int openvn_amiga_display_load_palette(
    OpenVNAmigaDisplay *display,
    const unsigned long *colors
) {
    if (display == 0 || display->screen == 0 || colors == 0) {
        return 0;
    }

    LoadRGB32(&display->screen->ViewPort, (ULONG *)colors);
    return 1;
}

#endif
