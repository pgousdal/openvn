#include "openvn_dispatch.h"
#include "openvn_story.h"
#include "openvn_graphics_amiga.h"

#ifdef __AMIGA__

#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>

extern void openvn_state_reset(void);
extern int openvn_dispatch_request(const OpenVNRequest *request);
extern int openvn_state_update(void);
extern unsigned long openvn_state_signal_mask(void);
extern const OpenVNStoryState *openvn_state_story(void);
extern const char *openvn_state_last_error(void);
extern OpenVNGraphicsService *openvn_state_graphics(void);

static int dispatch_simple(OpenVNCommand command) {
    OpenVNRequest request;

    request.command = command;
    request.argument1[0] = '\0';
    request.argument2[0] = '\0';
    return openvn_dispatch_request(&request);
}

static int choose_interactive(void) {
    OpenVNRequest request;
    OpenVNGraphicsService *graphics;
    size_t selected;
    int choice_made;
    int written;
    ULONG audio_mask;
    ULONG choice_mask;
    ULONG wait_mask;
    ULONG signals;

    selected = 0U;
    choice_made = 0;
    graphics = openvn_state_graphics();
    choice_mask = openvn_graphics_amiga_choice_signal_mask(graphics);
    if (choice_mask == 0UL) {
        return 0;
    }

    while (!choice_made) {
        audio_mask = openvn_state_signal_mask();
        wait_mask = choice_mask | audio_mask;
        signals = Wait(wait_mask);

        if ((signals & audio_mask) != 0UL && !openvn_state_update()) {
            return 0;
        }
        if ((signals & choice_mask) != 0UL &&
            !openvn_graphics_amiga_poll_choice(
                graphics,
                &selected,
                &choice_made
            )) {
            return 0;
        }
    }

    request.command = OPENVN_CMD_CHOOSE;
    written = snprintf(
        request.argument1,
        sizeof(request.argument1),
        "%lu",
        (unsigned long)selected
    );
    if (written <= 0 || (size_t)written >= sizeof(request.argument1)) {
        return 0;
    }
    request.argument2[0] = '\0';
    return openvn_dispatch_request(&request);
}

int main(void) {
    const OpenVNStoryState *story;
    const OpenVNGeneratedNode *node;

    PutStr("OpenVN native player started (compiled story mode).\n");
    PutStr("ARexx is optional and not required by this build.\n");

    PutStr("OpenVN trace: resetting runtime.\n");
    openvn_state_reset();

    PutStr("OpenVN trace: starting generated story.\n");
    if (!dispatch_simple(OPENVN_CMD_RUN)) {
        PutStr("OpenVN ERROR: ");
        PutStr((STRPTR)openvn_state_last_error());
        PutStr("\n");
        return 20;
    }
    PutStr("OpenVN trace: generated story started.\n");

    for (;;) {
        story = openvn_state_story();
        if (story == 0 || story->ended) {
            break;
        }

        node = openvn_story_current(story);
        if (node == 0) {
            PutStr("OpenVN: invalid story state.\n");
            return 20;
        }

        if (node->type == OPENVN_NODE_CHOICE) {
            if (node->option_count == 0U || !choose_interactive()) {
                PutStr("OpenVN: unable to resolve choice.\n");
                return 20;
            }
        } else if (!dispatch_simple(OPENVN_CMD_STEP)) {
            PutStr("OpenVN: unable to advance story.\n");
            return 20;
        }

        if (!openvn_state_update()) {
            PutStr("OpenVN: runtime update failed.\n");
            return 20;
        }
    }

    dispatch_simple(OPENVN_CMD_QUIT);
    PutStr("OpenVN compiled story finished.\n");
    return 0;
}

#else

int main(void) {
    return 0;
}

#endif
