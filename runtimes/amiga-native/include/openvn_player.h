#ifndef OPENVN_PLAYER_H
#define OPENVN_PLAYER_H

#include "openvn_audio.h"
#include "openvn_graphics.h"
#include "openvn_story.h"

typedef struct OpenVNPlayer {
    OpenVNStoryState story;
    OpenVNGraphicsService *graphics;
    OpenVNAudioService *audio;
} OpenVNPlayer;

void openvn_player_init(
    OpenVNPlayer *player,
    OpenVNGraphicsService *graphics,
    OpenVNAudioService *audio
);
int openvn_player_start(OpenVNPlayer *player);
int openvn_player_step(OpenVNPlayer *player);
int openvn_player_choose(OpenVNPlayer *player, size_t index);
const char *openvn_player_status(const OpenVNPlayer *player);

#endif
