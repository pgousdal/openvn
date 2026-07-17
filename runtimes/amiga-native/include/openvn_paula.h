#ifndef OPENVN_PAULA_H
#define OPENVN_PAULA_H

#include "openvn_mod.h"

typedef struct OpenVNPaulaVoice {
    const unsigned char *data;
    unsigned long length;
    const unsigned char *loop_data;
    unsigned long loop_length;
    unsigned short period;
    unsigned char volume;
    unsigned long generation;
    int active;
} OpenVNPaulaVoice;

typedef struct OpenVNPaulaMixer {
    OpenVNPaulaVoice voices[OPENVN_MOD_CHANNELS];
} OpenVNPaulaMixer;

void openvn_paula_reset(OpenVNPaulaMixer *mixer);

int openvn_paula_trigger_note(
    OpenVNPaulaMixer *mixer,
    unsigned int channel,
    const OpenVNMODModule *module,
    const OpenVNMODNote *note
);

const OpenVNPaulaVoice *openvn_paula_voice(
    const OpenVNPaulaMixer *mixer,
    unsigned int channel
);

#endif
