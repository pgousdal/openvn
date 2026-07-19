#include "assets.generated.h"
#include "openvn_graphics.h"
#include "openvn_graphics_host.h"

#include "test_check.h"
#include <string.h>

int main(void) {
    OpenVNGraphicsService service = {0};
    OpenVNHostGraphicsContext context = {0};
    OpenVNGraphicsConfig config = {0};
    OpenVNGeneratedChoice choices[2] = {{0}};

    openvn_graphics_host_init(&service, &context);

    config.width = 320U;
    config.height = 256U;
    config.depth = 5U;
    config.use_datatypes = 0;
    config.fullscreen = 1;
    config.assets = &OPENVN_GENERATED_ASSETS;

    OPENVN_TEST_CHECK(openvn_graphics_open(&service, &config));
    OPENVN_TEST_CHECK(context.opened);

    OPENVN_TEST_CHECK(openvn_graphics_scene(&service, "lighthouse_storm"));
    OPENVN_TEST_CHECK(strcmp(context.scene, "lighthouse_storm") == 0);
    OPENVN_TEST_CHECK(context.background_image.width == 16U);

    OPENVN_TEST_CHECK(openvn_graphics_show(&service, "erik", "neutral"));
    OPENVN_TEST_CHECK(strcmp(context.character, "erik") == 0);
    OPENVN_TEST_CHECK(strcmp(context.pose, "neutral") == 0);
    OPENVN_TEST_CHECK(context.character_image.width == 8U);

    OPENVN_TEST_CHECK(openvn_graphics_text(&service, "Hello from OpenVN."));
    OPENVN_TEST_CHECK(strcmp(context.dialogue, "Hello from OpenVN.") == 0);

    choices[0].text = "Continue";
    choices[0].target = "next";
    choices[1].text = "Finish";
    choices[1].target = "end";
    OPENVN_TEST_CHECK(openvn_graphics_choices(&service, choices, 2U, 1U));
    OPENVN_TEST_CHECK(context.choices_visible);
    OPENVN_TEST_CHECK(context.choice_count == 2U);
    OPENVN_TEST_CHECK(context.choice_selected == 1U);

    OPENVN_TEST_CHECK(openvn_graphics_present(&service));
    OPENVN_TEST_CHECK(context.present_count == 1U);

    OPENVN_TEST_CHECK(openvn_graphics_hide(&service, "erik"));
    OPENVN_TEST_CHECK(context.character[0] == '\0');

    openvn_graphics_close(&service);
    OPENVN_TEST_CHECK(!context.opened);

    return 0;
}
