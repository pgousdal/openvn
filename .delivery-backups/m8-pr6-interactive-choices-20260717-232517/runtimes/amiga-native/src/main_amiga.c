#include "openvn_dispatch.h"
#include "openvn_story.h"

#ifdef __AMIGA__

#include <proto/dos.h>

extern void openvn_state_reset(void);
extern int openvn_dispatch_request(const OpenVNRequest *request);
extern int openvn_state_update(void);
extern const OpenVNStoryState *openvn_state_story(void);
extern const char *openvn_state_last_error(void);

static int dispatch_simple(OpenVNCommand command) {
    OpenVNRequest request;

    request.command = command;
    request.argument1[0] = '\0';
    request.argument2[0] = '\0';
    return openvn_dispatch_request(&request);
}

static int choose_first(void) {
    OpenVNRequest request;

    request.command = OPENVN_CMD_CHOOSE;
    request.argument1[0] = '0';
    request.argument1[1] = '\0';
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
            if (node->option_count == 0U || !choose_first()) {
                PutStr("OpenVN: unable to resolve choice.\n");
                return 20;
            }
        } else if (!dispatch_simple(OPENVN_CMD_STEP)) {
            PutStr("OpenVN: unable to advance story.\n");
            return 20;
        }

        openvn_state_update();
        Delay(10);
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
