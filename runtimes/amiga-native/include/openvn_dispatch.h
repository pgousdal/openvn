#ifndef OPENVN_DISPATCH_H
#define OPENVN_DISPATCH_H

#include <stddef.h>

typedef enum OpenVNCommand {
    OPENVN_CMD_INVALID = 0,
    OPENVN_CMD_LOAD,
    OPENVN_CMD_RUN,
    OPENVN_CMD_SCENE,
    OPENVN_CMD_SHOW,
    OPENVN_CMD_HIDE,
    OPENVN_CMD_MUSIC,
    OPENVN_CMD_SOUND,
    OPENVN_CMD_QUIT
} OpenVNCommand;

typedef struct OpenVNRequest {
    OpenVNCommand command;
    char argument1[256];
    char argument2[256];
} OpenVNRequest;

OpenVNCommand openvn_command_from_string(const char *name);
int openvn_parse_request(const char *line, OpenVNRequest *request);
const char *openvn_command_name(OpenVNCommand command);

#endif
