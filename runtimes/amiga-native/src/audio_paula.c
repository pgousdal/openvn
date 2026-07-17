#include "openvn_paula.h"

#include <string.h>

void openvn_paula_reset(OpenVNPaulaMixer *mixer) {
    if (mixer != 0) {
        memset(mixer, 0, sizeof(*mixer));
    }
}

int openvn_paula_trigger_note(
    OpenVNPaulaMixer *mixer,
    unsigned int channel,
    const OpenVNMODModule *module,
    const OpenVNMODNote *note
) {
    OpenVNPaulaVoice *voice;
    const OpenVNMODSample *sample;

    if (mixer == 0 || module == 0 || note == 0 ||
        channel >= OPENVN_MOD_CHANNELS || note->sample == 0U ||
        note->sample > OPENVN_MOD_SAMPLE_COUNT || note->period == 0U) {
        return 0;
    }

    sample = &module->samples[note->sample - 1U];
    if (sample->data == 0 || sample->length < 2UL) {
        return 0;
    }

    voice = &mixer->voices[channel];
    voice->data = sample->data;
    voice->length = sample->length;
    voice->period = note->period;
    voice->volume = sample->volume > 64U ? 64U : sample->volume;
    voice->generation++;
    voice->active = 1;

    voice->loop_data = 0;
    voice->loop_length = 0UL;
    if (sample->loop_length > 2UL &&
        sample->loop_start < sample->length &&
        sample->loop_length <= sample->length - sample->loop_start) {
        voice->loop_data = sample->data + sample->loop_start;
        voice->loop_length = sample->loop_length;
    }

    return 1;
}

const OpenVNPaulaVoice *openvn_paula_voice(
    const OpenVNPaulaMixer *mixer,
    unsigned int channel
) {
    if (mixer == 0 || channel >= OPENVN_MOD_CHANNELS) {
        return 0;
    }

    return &mixer->voices[channel];
}
