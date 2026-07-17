#include "openvn_dispatch.h"

#include <stdio.h>
#include <string.h>

static void copy_token(char *destination, size_t size, const char *source) {
    if (size == 0U) {
        return;
    }

    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    snprintf(destination, size, "%s", source);
}

OpenVNCommand openvn_command_from_string(const char *name) {
    if (name == NULL) return OPENVN_CMD_INVALID;
    if (strcmp(name, "LOAD") == 0) return OPENVN_CMD_LOAD;
    if (strcmp(name, "RUN") == 0) return OPENVN_CMD_RUN;
    if (strcmp(name, "STEP") == 0) return OPENVN_CMD_STEP;
    if (strcmp(name, "CHOOSE") == 0) return OPENVN_CMD_CHOOSE;
    if (strcmp(name, "STATUS") == 0) return OPENVN_CMD_STATUS;
    if (strcmp(name, "SCENE") == 0) return OPENVN_CMD_SCENE;
    if (strcmp(name, "SHOW") == 0) return OPENVN_CMD_SHOW;
    if (strcmp(name, "HIDE") == 0) return OPENVN_CMD_HIDE;
    if (strcmp(name, "MUSIC") == 0) return OPENVN_CMD_MUSIC;
    if (strcmp(name, "SOUND") == 0) return OPENVN_CMD_SOUND;
    if (strcmp(name, "QUIT") == 0) return OPENVN_CMD_QUIT;
    return OPENVN_CMD_INVALID;
}

const char *openvn_command_name(OpenVNCommand command) {
    switch (command) {
        case OPENVN_CMD_LOAD: return "LOAD";
        case OPENVN_CMD_RUN: return "RUN";
        case OPENVN_CMD_STEP: return "STEP";
        case OPENVN_CMD_CHOOSE: return "CHOOSE";
        case OPENVN_CMD_STATUS: return "STATUS";
        case OPENVN_CMD_SCENE: return "SCENE";
        case OPENVN_CMD_SHOW: return "SHOW";
        case OPENVN_CMD_HIDE: return "HIDE";
        case OPENVN_CMD_MUSIC: return "MUSIC";
        case OPENVN_CMD_SOUND: return "SOUND";
        case OPENVN_CMD_QUIT: return "QUIT";
        default: return "INVALID";
    }
}

int openvn_parse_request(const char *line, OpenVNRequest *request) {
    char buffer[768];
    char *command_name;
    char *argument1;
    char *argument2;

    if (line == NULL || request == NULL) {
        return 0;
    }

    snprintf(buffer, sizeof(buffer), "%s", line);
    command_name = strtok(buffer, " \t\r\n");
    argument1 = strtok(NULL, " \t\r\n");
    argument2 = strtok(NULL, " \t\r\n");

    request->command = openvn_command_from_string(command_name);
    copy_token(request->argument1, sizeof(request->argument1), argument1);
    copy_token(request->argument2, sizeof(request->argument2), argument2);

    switch (request->command) {
        case OPENVN_CMD_LOAD:
        case OPENVN_CMD_CHOOSE:
        case OPENVN_CMD_SCENE:
        case OPENVN_CMD_HIDE:
        case OPENVN_CMD_MUSIC:
        case OPENVN_CMD_SOUND:
            return request->argument1[0] != '\0';
        case OPENVN_CMD_SHOW:
            return request->argument1[0] != '\0' &&
                   request->argument2[0] != '\0';
        case OPENVN_CMD_RUN:
        case OPENVN_CMD_STEP:
        case OPENVN_CMD_STATUS:
        case OPENVN_CMD_QUIT:
            return 1;
        default:
            return 0;
    }
}
