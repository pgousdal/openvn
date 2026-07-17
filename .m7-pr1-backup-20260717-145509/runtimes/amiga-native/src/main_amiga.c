#include "openvn_dispatch.h"

#ifdef __AMIGA__

#include <exec/types.h>
#include <exec/ports.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>

#include <rexx/rxslib.h>
#include <rexx/storage.h>

#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
extern void openvn_state_reset(void);
extern int openvn_dispatch_request(const OpenVNRequest *request);
extern int openvn_state_update(void);
extern unsigned long openvn_state_signal_mask(void);

static const char *PORT_NAME = "OPENVNPLAYER";

static void reply_message(struct RexxMsg *message, LONG result1, const char *result2) {
    message->rm_Result1 = result1;
    message->rm_Result2 = 0;

    if (result2 != NULL) {
        message->rm_Result2 = (LONG)CreateArgstring(result2, strlen(result2));
    }

    ReplyMsg((struct Message *)message);
}

int main(void) {
    struct MsgPort *port;
    struct RexxMsg *message;
    int running = 1;

    openvn_state_reset();

    port = CreatePort((STRPTR)PORT_NAME, 0);
    if (port == NULL) {
        PutStr("OpenVN: unable to create ARexx port\n");
        return 20;
    }

    PutStr("OpenVN native player skeleton started.\n");

    while (running) {
        ULONG rexx_mask = 1UL << port->mp_SigBit;
        ULONG wait_mask = rexx_mask | openvn_state_signal_mask();
        ULONG signals = Wait(wait_mask);

        if ((signals & openvn_state_signal_mask()) != 0UL) {
            openvn_state_update();
        }

        if ((signals & rexx_mask) == 0UL) {
            continue;
        }

        while ((message = (struct RexxMsg *)GetMsg(port)) != NULL) {
            OpenVNRequest request;
            const char *command = ARG0(message);

            if (!openvn_parse_request(command, &request)) {
                reply_message(message, 10, "INVALID COMMAND");
                continue;
            }

            if (!openvn_dispatch_request(&request)) {
                reply_message(message, 10, "COMMAND FAILED");
                continue;
            }

            if (request.command == OPENVN_CMD_QUIT) {
                running = 0;
            }

            reply_message(message, 0, openvn_command_name(request.command));
        }
    }

    RemPort(port);
    DeleteMsgPort(port);
    return 0;
}

#else

int main(void) {
    return 0;
}

#endif
