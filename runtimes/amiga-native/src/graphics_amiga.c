#include "openvn_graphics_amiga.h"
#include "openvn_bitmap_font.h"
#include "openvn_dialogue.h"
#include <stddef.h>

#ifdef __AMIGA__

#include <stdio.h>
#include <string.h>

#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <intuition/intuition.h>
#define DATATYPES_BASE_NAME g_openvn_datatypes_library
#include <proto/datatypes.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#define OPENVN_RENDER_LOG "openvn-render.log"
#define OPENVN_TEXT_PEN 2U
#define OPENVN_SELECTED_TEXT_PEN 1U

struct Library *g_openvn_datatypes_library = 0;

static void dispose_datatype(Object **object) {
    if (object != 0 && *object != 0) {
        DisposeDTObject(*object);
        *object = 0;
    }
}

static int load_datatype(Object **destination, const char *path) {
    dispose_datatype(destination);
    *destination = NewDTObject(
        (APTR)path,
        DTA_SourceType,
        DTST_FILE,
        DTA_GroupID,
        GID_PICTURE,
        PDTA_Remap,
        TRUE,
        TAG_DONE
    );
    return *destination != 0;
}

/*
 * Some classic NDKs declare no callable DrawDTObjectA stub. Dispatching its
 * documented DTM_DRAW message through DoDTMethodA keeps the library-vector
 * call native and avoids an unresolved external symbol.
 */
static ULONG openvn_DrawDTObjectA_compat(
    struct RastPort *rastport,
    Object *object,
    LONG left,
    LONG top,
    LONG width,
    LONG height,
    LONG top_horiz,
    LONG top_vert,
    ULONG flags,
    struct TagItem *attributes
) {
    struct dtDraw draw_message;

    (void)flags;
    draw_message.MethodID = DTM_DRAW;
    draw_message.dtd_RPort = rastport;
    draw_message.dtd_Left = left;
    draw_message.dtd_Top = top;
    draw_message.dtd_Width = width;
    draw_message.dtd_Height = height;
    draw_message.dtd_TopHoriz = top_horiz;
    draw_message.dtd_TopVert = top_vert;
    draw_message.dtd_AttrList = attributes;

    return DoDTMethodA(object, 0, 0, (Msg)&draw_message);
}

static void trace_reset(void) {
    FILE *file;

    file = fopen(OPENVN_RENDER_LOG, "w");
    if (file != 0) {
        fputs("OpenVN Amiga render diagnostics\n", file);
        fclose(file);
    }
}

static void trace_message(const char *message) {
    FILE *file;

    file = fopen(OPENVN_RENDER_LOG, "a");
    if (file != 0) {
        fputs(message, file);
        fputc('\n', file);
        fclose(file);
    }
}

static void trace_value(const char *label, const char *value) {
    FILE *file;

    file = fopen(OPENVN_RENDER_LOG, "a");
    if (file != 0) {
        fprintf(file, "%s%s\n", label, value != 0 ? value : "(null)");
        fclose(file);
    }
}

static void trace_image(const char *label, const OpenVNILBMImage *image) {
    FILE *file;

    file = fopen(OPENVN_RENDER_LOG, "a");
    if (file != 0) {
        fprintf(
            file,
            "%s width=%u height=%u depth=%u palette=%u body=%lu\n",
            label,
            image != 0 ? image->width : 0U,
            image != 0 ? image->height : 0U,
            image != 0 ? image->depth : 0U,
            image != 0 ? image->palette_size : 0U,
            image != 0 ? (unsigned long)image->body_size : 0UL
        );
        fclose(file);
    }
}

static void free_classic_background(
    OpenVNAmigaGraphicsContext *context
) {
    openvn_amiga_bitmap_free(&context->background_bitmap);
    openvn_planar_free(&context->background_planar);
    openvn_ilbm_free(&context->background_ilbm);
    openvn_palette_reset(&context->background_palette);
}

static void free_classic_character(
    OpenVNAmigaGraphicsContext *context
) {
    openvn_amiga_bitmap_free(&context->character_bitmap);
    openvn_planar_free(&context->character_planar);
    openvn_ilbm_free(&context->character_ilbm);
}

static int amiga_open(
    OpenVNGraphicsService *service,
    const OpenVNGraphicsConfig *config
) {
    OpenVNAmigaGraphicsContext *context;

    trace_reset();
    trace_message("OPEN begin");

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0) {
        trace_message("OPEN failed: context is null");
        return 0;
    }

    memset(context, 0, sizeof(*context));
    context->assets = config->assets;
    context->use_datatypes = config->use_datatypes;

    if (context->use_datatypes && g_openvn_datatypes_library == 0) {
        g_openvn_datatypes_library = OpenLibrary("datatypes.library", 39UL);
        if (g_openvn_datatypes_library == 0) {
            trace_message("OPEN failed: datatypes.library unavailable");
            return 0;
        }
    }

    openvn_amiga_display_reset(&context->display);
    openvn_palette_reset(&context->background_palette);
    openvn_ilbm_reset(&context->background_ilbm);
    openvn_ilbm_reset(&context->character_ilbm);
    openvn_planar_reset(&context->background_planar);
    openvn_planar_reset(&context->character_planar);
    openvn_amiga_bitmap_reset(&context->background_bitmap);
    openvn_amiga_bitmap_reset(&context->character_bitmap);
    context->character_anchor = OPENVN_ANCHOR_CENTER;

    trace_message("OPEN display open");
    if (!openvn_amiga_display_open(
            &context->display,
            config->width,
            config->height,
            config->depth,
            config->fullscreen,
            1
        )) {
        trace_message("OPEN failed: display open");
        if (g_openvn_datatypes_library != 0) {
            CloseLibrary(g_openvn_datatypes_library);
            g_openvn_datatypes_library = 0;
        }
        return 0;
    }

    context->opened = 1;
    trace_message("OPEN ok");
    return 1;
}

static void amiga_close(OpenVNGraphicsService *service) {
    OpenVNAmigaGraphicsContext *context;

    trace_message("CLOSE begin");
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0) {
        trace_message("CLOSE skipped: context is null");
        return;
    }

    free_classic_background(context);
    free_classic_character(context);
    dispose_datatype(&context->background_datatype);
    dispose_datatype(&context->character_datatype);
    openvn_amiga_display_close(&context->display);
    if (g_openvn_datatypes_library != 0) {
        CloseLibrary(g_openvn_datatypes_library);
        g_openvn_datatypes_library = 0;
    }
    context->opened = 0;
    trace_message("CLOSE ok");
}

static int load_classic_bitmap(
    const char *path,
    OpenVNILBMImage *image,
    OpenVNPlanarBitmap *planar,
    OpenVNAmigaBitmap *bitmap,
    int masked
) {
    trace_value("BITMAP path: ", path);
    openvn_amiga_bitmap_free(bitmap);
    openvn_planar_free(planar);
    openvn_ilbm_free(image);

    trace_message("BITMAP ILBM load begin");
    if (!openvn_ilbm_load_file(image, path)) {
        trace_message("BITMAP failed: ILBM load");
        return 0;
    }
    trace_image("BITMAP ILBM ok", image);

    trace_message("BITMAP planar conversion begin");
    if (!openvn_planar_from_chunky(planar, image, masked, 0U)) {
        trace_message("BITMAP failed: planar conversion");
        openvn_ilbm_free(image);
        return 0;
    }
    trace_message("BITMAP planar conversion ok");

    trace_message("BITMAP Amiga bitmap allocation begin");
    if (!openvn_amiga_bitmap_from_planar(bitmap, planar)) {
        trace_message("BITMAP failed: Amiga bitmap allocation");
        openvn_planar_free(planar);
        openvn_ilbm_free(image);
        return 0;
    }

    trace_message("BITMAP Amiga bitmap allocation ok");
    return 1;
}

static int amiga_scene(
    OpenVNGraphicsService *service,
    const char *background
) {
    OpenVNAmigaGraphicsContext *context;
    const char *path;

    trace_value("SCENE id: ", background);
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        trace_message("SCENE failed: graphics not open");
        return 0;
    }

    path = openvn_asset_find_background(context->assets, background);
    trace_value("SCENE resolved path: ", path);
    if (path == 0) {
        trace_message("SCENE failed: asset lookup");
        return 0;
    }

    if (context->use_datatypes) {
        return load_datatype(&context->background_datatype, path);
    }

    if (!load_classic_bitmap(
            path,
            &context->background_ilbm,
            &context->background_planar,
            &context->background_bitmap,
            0
        )) {
        trace_message("SCENE failed: bitmap load");
        return 0;
    }

    trace_message("SCENE palette conversion begin");
    if (!openvn_palette_from_ilbm(
            &context->background_palette,
            context->background_ilbm.palette,
            context->background_ilbm.palette_size
        )) {
        trace_message("SCENE failed: palette conversion");
        return 0;
    }
    trace_message("SCENE palette conversion ok");

    trace_message("SCENE palette upload begin");
    if (!openvn_amiga_display_load_palette(
            &context->display,
            context->background_palette.colors
        )) {
        trace_message("SCENE failed: palette upload");
        return 0;
    }

    trace_message("SCENE ok");
    return 1;
}

static int amiga_show(
    OpenVNGraphicsService *service,
    const char *character,
    const char *pose
) {
    OpenVNAmigaGraphicsContext *context;
    const char *path;

    trace_value("SHOW character: ", character);
    trace_value("SHOW pose: ", pose);
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        trace_message("SHOW failed: graphics not open");
        return 0;
    }

    path = openvn_asset_find_character(context->assets, character, pose);
    trace_value("SHOW resolved path: ", path);
    if (path == 0) {
        trace_message("SHOW failed: asset lookup");
        return 0;
    }

    context->character_visible = 1;
    context->character_anchor = OPENVN_ANCHOR_CENTER;
    if (context->use_datatypes) {
        return load_datatype(&context->character_datatype, path);
    }
    if (!load_classic_bitmap(
            path,
            &context->character_ilbm,
            &context->character_planar,
            &context->character_bitmap,
            1
        )) {
        trace_message("SHOW failed: bitmap load");
        return 0;
    }

    context->character_origin_x = context->character_bitmap.width / 2U;
    context->character_origin_y = context->character_bitmap.height;

    trace_message("SHOW ok");
    return 1;
}

static int amiga_hide(
    OpenVNGraphicsService *service,
    const char *character
) {
    OpenVNAmigaGraphicsContext *context;

    trace_value("HIDE character: ", character);
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened) {
        trace_message("HIDE failed: graphics not open");
        return 0;
    }

    if (context->use_datatypes) {
        dispose_datatype(&context->character_datatype);
    } else {
        free_classic_character(context);
    }
    context->character_visible = 0;
    trace_message("HIDE ok");
    return 1;
}

static int amiga_text(
    OpenVNGraphicsService *service,
    const char *text
) {
    OpenVNAmigaGraphicsContext *context;
    OpenVNDialogueLayout layout;
    unsigned int columns;

    trace_value("TEXT: ", text);
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened || text == 0) {
        trace_message("TEXT failed: graphics not open");
        return 0;
    }

    openvn_dialogue_layout(
        (unsigned int)context->display.window->Width,
        (unsigned int)context->display.window->Height,
        &layout
    );
    columns = (unsigned int)layout.text_width /
              OPENVN_BITMAP_FONT_ADVANCE;

    if (!openvn_dialogue_wrap(
            text,
            columns,
            context->dialogue_text,
            sizeof(context->dialogue_text)
        )) {
        trace_message("TEXT failed: wrapping");
        return 0;
    }

    context->dialogue_visible = 1;
    context->choices_visible = 0;
    trace_message("TEXT ok");
    return 1;
}

static int amiga_choices(
    OpenVNGraphicsService *service,
    const OpenVNGeneratedChoice *options,
    size_t option_count,
    size_t selected_index
) {
    OpenVNAmigaGraphicsContext *context;

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened || options == 0 ||
        option_count == 0U || selected_index >= option_count) {
        trace_message("CHOICES failed: invalid state");
        return 0;
    }

    context->choice_options = options;
    context->choice_count = option_count;
    context->choice_selected = selected_index;
    context->choices_visible = 1;
    context->dialogue_visible = 0;
    trace_message("CHOICES ok");
    return 1;
}

static int amiga_present(OpenVNGraphicsService *service) {
    OpenVNAmigaGraphicsContext *context;
    struct RastPort *rastport;
    OpenVNScenePosition position;

    trace_message("PRESENT begin");
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened || context->display.window == 0) {
        trace_message("PRESENT failed: graphics/window unavailable");
        return 0;
    }

    rastport = openvn_amiga_display_draw_rastport(&context->display);
    if (rastport == 0) {
        trace_message("PRESENT failed: draw RastPort unavailable");
        return 0;
    }

    if (context->use_datatypes) {
        if (context->background_datatype != 0) {
            openvn_DrawDTObjectA_compat(
                rastport,
                context->background_datatype,
                0,
                0,
                context->display.window->Width,
                context->display.window->Height,
                0,
                0,
                0,
                0
            );
        }
        if (context->character_visible && context->character_datatype != 0) {
            openvn_DrawDTObjectA_compat(
                rastport,
                context->character_datatype,
                0,
                0,
                context->display.window->Width,
                context->display.window->Height,
                0,
                0,
                0,
                0
            );
        }
    } else {

    trace_message("PRESENT background blit begin");
    if (!openvn_amiga_bitmap_blit(
            &context->background_bitmap,
            rastport,
            0,
            0,
            0
        )) {
        trace_message("PRESENT failed: background blit");
        return 0;
    }
    trace_message("PRESENT background blit ok");

    if (context->character_visible &&
        context->character_bitmap.bitmap != 0) {
        position = openvn_scene_character_position(
            (unsigned int)context->display.window->Width,
            (unsigned int)context->display.window->Height,
            context->character_bitmap.width,
            context->character_bitmap.height,
            context->character_origin_x,
            context->character_origin_y,
            context->character_anchor
        );

        trace_message("PRESENT character blit begin");
        if (!openvn_amiga_bitmap_blit(
                &context->character_bitmap,
                rastport,
                position.x,
                position.y,
                1
            )) {
            trace_message("PRESENT failed: character blit");
            return 0;
        }
        trace_message("PRESENT character blit ok");
    }
    }


    if (context->dialogue_visible) {
        OpenVNDialogueLayout layout;

        openvn_dialogue_layout(
            (unsigned int)context->display.window->Width,
            (unsigned int)context->display.window->Height,
            &layout
        );

        trace_message("PRESENT dialogue begin");
        SetAPen(rastport, 0UL);
        RectFill(
            rastport,
            layout.box_x,
            layout.box_y,
            layout.box_x + layout.box_width - 1,
            layout.box_y + layout.box_height - 1
        );
        SetAPen(rastport, 2UL);
        Move(rastport, layout.box_x, layout.box_y);
        Draw(rastport, layout.box_x + layout.box_width - 1, layout.box_y);
        Draw(
            rastport,
            layout.box_x + layout.box_width - 1,
            layout.box_y + layout.box_height - 1
        );
        Draw(rastport, layout.box_x, layout.box_y + layout.box_height - 1);
        Draw(rastport, layout.box_x, layout.box_y);
        openvn_bitmap_font_draw_amiga(
            rastport,
            context->dialogue_text,
            layout.text_x,
            layout.text_y,
            OPENVN_TEXT_PEN
        );
        trace_message("PRESENT dialogue ok");
    }


    if (context->choices_visible) {
        OpenVNDialogueLayout layout;
        size_t index;
        int y;
        char line[192];

        openvn_dialogue_layout(
            (unsigned int)context->display.window->Width,
            (unsigned int)context->display.window->Height,
            &layout
        );

        trace_message("PRESENT choices begin");
        SetAPen(rastport, 0UL);
        RectFill(
            rastport,
            layout.box_x,
            layout.box_y,
            layout.box_x + layout.box_width - 1,
            layout.box_y + layout.box_height - 1
        );
        SetAPen(rastport, 2UL);
        Move(rastport, layout.box_x, layout.box_y);
        Draw(rastport, layout.box_x + layout.box_width - 1, layout.box_y);
        Draw(
            rastport,
            layout.box_x + layout.box_width - 1,
            layout.box_y + layout.box_height - 1
        );
        Draw(rastport, layout.box_x, layout.box_y + layout.box_height - 1);
        Draw(rastport, layout.box_x, layout.box_y);

        y = layout.text_y;
        for (index = 0U; index < context->choice_count; ++index) {
            snprintf(
                line,
                sizeof(line),
                "%c %s",
                index == context->choice_selected ? '>' : ' ',
                context->choice_options[index].text
            );
            openvn_bitmap_font_draw_amiga(
                rastport,
                line,
                layout.text_x,
                y,
                index == context->choice_selected
                    ? OPENVN_SELECTED_TEXT_PEN
                    : OPENVN_TEXT_PEN
            );
            y += OPENVN_BITMAP_FONT_LINE_HEIGHT;
            if (y + OPENVN_BITMAP_FONT_LINE_HEIGHT >
                layout.box_y + layout.box_height) {
                break;
            }
        }
        trace_message("PRESENT choices ok");
    }

    trace_message("PRESENT display swap begin");
    if (!openvn_amiga_display_present(&context->display)) {
        trace_message("PRESENT failed: display swap");
        return 0;
    }

    trace_message("PRESENT ok");
    return 1;
}

static const OpenVNGraphicsVTable AMIGA_VTABLE = {
    amiga_open,
    amiga_close,
    amiga_scene,
    amiga_show,
    amiga_hide,
    amiga_text,
    amiga_choices,
    amiga_present
};

void openvn_graphics_amiga_init(
    OpenVNGraphicsService *service,
    OpenVNAmigaGraphicsContext *context
) {
    if (service != 0) {
        service->vtable = &AMIGA_VTABLE;
        service->context = context;
    }
}


int openvn_graphics_amiga_wait_choice(
    OpenVNGraphicsService *service,
    size_t *selected_index
) {
    OpenVNAmigaGraphicsContext *context;
    int selected;

    if (service == 0 || selected_index == 0) {
        return 0;
    }

    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->opened || context->display.window == 0 ||
        !context->choices_visible || context->choice_count == 0U) {
        return 0;
    }

    selected = 0;
    while (!selected) {
        WaitPort(context->display.window->UserPort);
        if (!openvn_graphics_amiga_poll_choice(
                service,
                selected_index,
                &selected
            )) {
            return 0;
        }
    }

    return 1;
}

unsigned long openvn_graphics_amiga_choice_signal_mask(
    OpenVNGraphicsService *service
) {
    OpenVNAmigaGraphicsContext *context;

    if (service == 0) {
        return 0UL;
    }
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || context->display.window == 0 ||
        context->display.window->UserPort == 0) {
        return 0UL;
    }
    return 1UL << context->display.window->UserPort->mp_SigBit;
}

int openvn_graphics_amiga_poll_choice(
    OpenVNGraphicsService *service,
    size_t *selected_index,
    int *selected
) {
    OpenVNAmigaGraphicsContext *context;
    struct IntuiMessage *message;
    ULONG message_class;
    UWORD code;
    WORD mouse_x;
    WORD mouse_y;

    if (service == 0 || selected_index == 0 || selected == 0) {
        return 0;
    }
    context = (OpenVNAmigaGraphicsContext *)service->context;
    if (context == 0 || !context->choices_visible ||
        context->display.window == 0) {
        return 0;
    }

    *selected = 0;
    while ((message = (struct IntuiMessage *)GetMsg(
                context->display.window->UserPort
            )) != 0) {
        message_class = message->Class;
        code = message->Code;
        mouse_x = message->MouseX;
        mouse_y = message->MouseY;
        ReplyMsg((struct Message *)message);

        if (message_class == IDCMP_MOUSEBUTTONS && code == SELECTDOWN) {
            OpenVNDialogueLayout layout;
            size_t clicked_index;

            openvn_dialogue_layout(
                (unsigned int)context->display.window->Width,
                (unsigned int)context->display.window->Height,
                &layout
            );
            if (mouse_x >= layout.box_x &&
                mouse_x < layout.box_x + layout.box_width &&
                mouse_y >= layout.text_y) {
                clicked_index = (size_t)(mouse_y - layout.text_y) /
                                OPENVN_BITMAP_FONT_LINE_HEIGHT;
                if (clicked_index < context->choice_count) {
                    context->choice_selected = clicked_index;
                    *selected = 1;
                }
            }
        } else if (message_class == IDCMP_RAWKEY && (code & 0x80U) == 0U) {
            if (code == 0x4cU) {
                if (context->choice_selected == 0U) {
                    context->choice_selected = context->choice_count - 1U;
                } else {
                    --context->choice_selected;
                }
                openvn_graphics_present(service);
            } else if (code == 0x4dU) {
                context->choice_selected =
                    (context->choice_selected + 1U) % context->choice_count;
                openvn_graphics_present(service);
            } else if (code == 0x44U || code == 0x40U) {
                *selected = 1;
            }
        }
    }

    *selected_index = context->choice_selected;
    if (*selected) {
        context->choices_visible = 0;
        trace_message("CHOICES selected");
    }
    return 1;
}

#endif
