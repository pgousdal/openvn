#ifndef OPENVN_DISPLAY_AMIGA_H
#define OPENVN_DISPLAY_AMIGA_H

#ifdef __AMIGA__

struct Screen;
struct Window;
struct ScreenBuffer;
struct MsgPort;
struct RastPort;

typedef struct OpenVNAmigaDisplay {
    struct Screen *screen;
    struct Window *window;
    struct ScreenBuffer *buffers[2];
    struct MsgPort *safe_port;
    struct MsgPort *display_port;
    unsigned int draw_index;
    unsigned int visible_index;
    int double_buffered;
    int swap_pending;
} OpenVNAmigaDisplay;

void openvn_amiga_display_reset(OpenVNAmigaDisplay *display);

int openvn_amiga_display_open(
    OpenVNAmigaDisplay *display,
    unsigned int width,
    unsigned int height,
    unsigned int depth,
    int fullscreen,
    int double_buffered
);

void openvn_amiga_display_close(OpenVNAmigaDisplay *display);

struct RastPort *openvn_amiga_display_draw_rastport(
    OpenVNAmigaDisplay *display
);

int openvn_amiga_display_present(OpenVNAmigaDisplay *display);

int openvn_amiga_display_load_palette(
    OpenVNAmigaDisplay *display,
    const unsigned long *colors
);

#endif

#endif
