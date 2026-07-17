#include "openvn_dispatch.h"

#include <stdio.h>

extern void openvn_state_reset(void);
extern int openvn_dispatch_request(const OpenVNRequest *request);

int main(void) {
    char line[768];
    OpenVNRequest request;

    openvn_state_reset();

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (!openvn_parse_request(line, &request)) {
            puts("ERROR INVALID");
            continue;
        }

        if (!openvn_dispatch_request(&request)) {
            puts("ERROR FAILED");
            continue;
        }

        printf("OK %s\n", openvn_command_name(request.command));

        if (request.command == OPENVN_CMD_QUIT) {
            break;
        }
    }

    return 0;
}
