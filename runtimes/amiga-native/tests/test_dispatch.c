#include "openvn_dispatch.h"

#include <assert.h>
#include <string.h>

int main(void) {
    OpenVNRequest request;

    assert(openvn_parse_request("LOAD story/story.openvn.json", &request));
    assert(request.command == OPENVN_CMD_LOAD);
    assert(strcmp(request.argument1, "story/story.openvn.json") == 0);

    assert(openvn_parse_request("SHOW erik neutral", &request));
    assert(request.command == OPENVN_CMD_SHOW);
    assert(strcmp(request.argument1, "erik") == 0);
    assert(strcmp(request.argument2, "neutral") == 0);

    assert(openvn_parse_request("RUN", &request));
    assert(request.command == OPENVN_CMD_RUN);

    assert(!openvn_parse_request("SHOW erik", &request));
    assert(!openvn_parse_request("UNKNOWN thing", &request));

    return 0;
}
