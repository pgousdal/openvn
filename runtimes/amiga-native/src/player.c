#include "openvn_player.h"
#include "story.generated.h"

static int apply_current(OpenVNPlayer *player) {
    const OpenVNGeneratedNode *node;

    node = openvn_story_current(&player->story);
    if (node == 0) {
        return 0;
    }

    switch (node->type) {
        case OPENVN_NODE_SCENE:
            return openvn_graphics_scene(player->graphics, node->argument1) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_SHOW:
            return openvn_graphics_show(
                       player->graphics,
                       node->argument1,
                       node->argument2
                   ) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_HIDE:
            return openvn_graphics_hide(player->graphics, node->argument1) &&
                   openvn_graphics_present(player->graphics);
        default:
            return 1;
    }
}

void openvn_player_init(
    OpenVNPlayer *player,
    OpenVNGraphicsService *graphics
) {
    if (player == 0) {
        return;
    }

    openvn_story_attach(&player->story, &OPENVN_GENERATED_STORY);
    player->graphics = graphics;
}

int openvn_player_start(OpenVNPlayer *player) {
    if (player == 0 || !openvn_story_start(&player->story)) {
        return 0;
    }

    return apply_current(player);
}

int openvn_player_step(OpenVNPlayer *player) {
    if (player == 0 || !openvn_story_step(&player->story)) {
        return 0;
    }

    return apply_current(player);
}

int openvn_player_choose(OpenVNPlayer *player, size_t index) {
    if (player == 0 || !openvn_story_choose(&player->story, index)) {
        return 0;
    }

    return apply_current(player);
}

const char *openvn_player_status(const OpenVNPlayer *player) {
    if (player == 0) {
        return "UNLOADED";
    }

    return openvn_story_status(&player->story);
}
