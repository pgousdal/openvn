#include "openvn_dispatch.h"

#include "test_check.h"
#include <string.h>

int main(void) {
    OpenVNRequest request;

    OPENVN_TEST_CHECK(openvn_parse_request("LOAD story/story.openvn.json", &request));
    OPENVN_TEST_CHECK(request.command == OPENVN_CMD_LOAD);
    OPENVN_TEST_CHECK(strcmp(request.argument1, "story/story.openvn.json") == 0);

    OPENVN_TEST_CHECK(openvn_parse_request("SHOW erik neutral", &request));
    OPENVN_TEST_CHECK(request.command == OPENVN_CMD_SHOW);
    OPENVN_TEST_CHECK(strcmp(request.argument1, "erik") == 0);
    OPENVN_TEST_CHECK(strcmp(request.argument2, "neutral") == 0);

    OPENVN_TEST_CHECK(openvn_parse_request("RUN", &request));
    OPENVN_TEST_CHECK(request.command == OPENVN_CMD_RUN);

    OPENVN_TEST_CHECK(!openvn_parse_request("SHOW erik", &request));
    OPENVN_TEST_CHECK(!openvn_parse_request("UNKNOWN thing", &request));

    return 0;
}
