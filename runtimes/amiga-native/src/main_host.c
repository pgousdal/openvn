#include "openvn_dispatch.h"
#include "openvn_story.h"

#include <stdio.h>

extern void openvn_state_reset(void);
extern int openvn_dispatch_request(const OpenVNRequest *request);
extern const OpenVNStoryState *openvn_state_story(void);
extern int openvn_state_update(void);

static void print_current(void) {
    const OpenVNStoryState *state = openvn_state_story();
    const OpenVNGeneratedNode *node = openvn_story_current(state);
    size_t index;

    printf("STATUS %s\n", openvn_story_status(state));
    if (node == 0 || state->ended) {
        return;
    }

    if (node->type == OPENVN_NODE_TEXT) {
        printf("TEXT %s\n", node->text);
    } else if (node->type == OPENVN_NODE_CHOICE) {
        for (index = 0U; index < node->option_count; index++) {
            printf(
                "CHOICE %lu %s\n",
                (unsigned long)index,
                node->options[index].text
            );
        }
    } else if (node->type == OPENVN_NODE_SCENE) {
        printf("SCENE %s\n", node->argument1);
    } else if (node->type == OPENVN_NODE_SHOW) {
        printf("SHOW %s %s\n", node->argument1, node->argument2);
    } else if (node->type == OPENVN_NODE_HIDE) {
        printf("HIDE %s\n", node->argument1);
    } else if (node->type == OPENVN_NODE_MUSIC) {
        printf("MUSIC %s\n", node->argument1[0] ? node->argument1 : "stop");
    } else if (node->type == OPENVN_NODE_SOUND) {
        printf("SOUND %s\n", node->argument1);
    }
}

int main(void) {
    char line[768];
    OpenVNRequest request;

    openvn_state_reset();

    while (fgets(line, sizeof(line), stdin) != 0) {
        if (!openvn_parse_request(line, &request)) {
            puts("ERROR INVALID");
            continue;
        }

        if (!openvn_dispatch_request(&request)) {
            puts("ERROR FAILED");
            continue;
        }

        if (request.command != OPENVN_CMD_QUIT && !openvn_state_update()) {
            puts("ERROR AUDIO UPDATE");
            continue;
        }

        printf("OK %s\n", openvn_command_name(request.command));

        if (request.command == OPENVN_CMD_RUN ||
            request.command == OPENVN_CMD_STEP ||
            request.command == OPENVN_CMD_CHOOSE ||
            request.command == OPENVN_CMD_STATUS) {
            print_current();
        }

        if (request.command == OPENVN_CMD_QUIT) {
            break;
        }
    }

    return 0;
}
