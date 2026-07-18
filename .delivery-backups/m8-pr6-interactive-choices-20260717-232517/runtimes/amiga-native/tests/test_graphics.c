#include "assets.generated.h"
#include "openvn_graphics.h"
#include "openvn_graphics_host.h"

#include <assert.h>
#include <string.h>

int main(void) {
    OpenVNGraphicsService service;
    OpenVNHostGraphicsContext context;
    OpenVNGraphicsConfig config;

    openvn_graphics_host_init(&service, &context);

    config.width = 320U;
    config.height = 256U;
    config.depth = 5U;
    config.use_datatypes = 0;
    config.fullscreen = 1;
    config.assets = &OPENVN_GENERATED_ASSETS;

    assert(openvn_graphics_open(&service, &config));
    assert(context.opened);

    assert(openvn_graphics_scene(&service, "lighthouse_storm"));
    assert(strcmp(context.scene, "lighthouse_storm") == 0);
    assert(context.background_image.width == 16U);

    assert(openvn_graphics_show(&service, "erik", "neutral"));
    assert(strcmp(context.character, "erik") == 0);
    assert(strcmp(context.pose, "neutral") == 0);
    assert(context.character_image.width == 8U);

    assert(openvn_graphics_text(&service, "Hello from OpenVN."));
    assert(strcmp(context.dialogue, "Hello from OpenVN.") == 0);

    assert(openvn_graphics_present(&service));
    assert(context.present_count == 1U);

    assert(openvn_graphics_hide(&service, "erik"));
    assert(context.character[0] == '\0');

    openvn_graphics_close(&service);
    assert(!context.opened);

    return 0;
}
