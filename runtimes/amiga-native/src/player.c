#include "openvn_player.h"
#include "story.generated.h"

#include <errno.h>
#include <stdlib.h>

static int apply_current(OpenVNPlayer *player) {
    const OpenVNGeneratedNode *node;

    node = openvn_story_current(&player->story);
    if (node == 0) {
        return 0;
    }

    switch (node->type) {
        case OPENVN_NODE_TEXT:
            return openvn_graphics_text(
                       player->graphics,
                       node->text
                   ) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_CHOICE:
            return openvn_graphics_choices(
                       player->graphics,
                       node->options,
                       node->option_count,
                       0U
                   ) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_SCENE:
            return openvn_graphics_scene(
                       player->graphics,
                       node->argument1
                   ) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_SHOW:
            return openvn_graphics_show(
                       player->graphics,
                       node->argument1,
                       node->argument2
                   ) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_HIDE:
            return openvn_graphics_hide(
                       player->graphics,
                       node->argument1
                   ) &&
                   openvn_graphics_present(player->graphics);
        case OPENVN_NODE_MUSIC:
            if (node->argument1[0] == '\0') {
                return openvn_audio_stop_music(player->audio);
            }
            return openvn_audio_music(player->audio, node->argument1);
        case OPENVN_NODE_SOUND:
            return openvn_audio_sound(player->audio, node->argument1);
        case OPENVN_NODE_SET_BOOL:
            if (node->argument2[0] == 't') {
                return openvn_set_bool(node->argument1, 1);
            }
            if (node->argument2[0] == 'f') {
                return openvn_set_bool(node->argument1, 0);
            }
            return 0;
        case OPENVN_NODE_SET_INT: {
            char *end;
            long value;

            errno = 0;
            value = strtol(node->argument2, &end, 10);
            if (errno != 0 || *end != '\0' ||
                value < INT32_MIN || value > INT32_MAX) {
                return 0;
            }
            return openvn_set_int(node->argument1, (int32_t)value);
        }
        case OPENVN_NODE_SET_STRING:
            return openvn_set_string(node->argument1, node->argument2);
        default:
            return 1;
    }
}

void openvn_player_init(
    OpenVNPlayer *player,
    OpenVNGraphicsService *graphics,
    OpenVNAudioService *audio
) {
    if (player == 0) {
        return;
    }

    openvn_story_attach(&player->story, &OPENVN_GENERATED_STORY);
    openvn_variables_reset();
    player->graphics = graphics;
    player->audio = audio;
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

int openvn_player_update(OpenVNPlayer *player) {
    if (player == 0 || player->audio == 0) {
        return 0;
    }

    return openvn_audio_update(player->audio);
}

unsigned long openvn_player_signal_mask(const OpenVNPlayer *player) {
    if (player == 0 || player->audio == 0) {
        return 0UL;
    }

    return openvn_audio_signal_mask(player->audio);
}
