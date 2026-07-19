#include "openvn_paula.h"

#include "test_check.h"
#include <string.h>

int main(void) {
    OpenVNMODModule module;
    OpenVNMODNote notes[OPENVN_MOD_CHANNELS];
    OpenVNPaulaMixer mixer;
    OpenVNMODChannelState state;
    const OpenVNPaulaVoice *voice;
    unsigned char sample_data[16];
    unsigned int channel;

    memset(&module, 0, sizeof(module));
    memset(notes, 0, sizeof(notes));
    memset(sample_data, 0, sizeof(sample_data));
    memset(&state, 0, sizeof(state));
    openvn_paula_reset(&mixer);

    module.samples[0].data = sample_data;
    module.samples[0].length = sizeof(sample_data);
    module.samples[0].volume = 48U;
    module.samples[0].loop_start = 4UL;
    module.samples[0].loop_length = 8UL;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        notes[channel].sample = 1U;
        notes[channel].period = (unsigned short)(428U + channel * 16U);
        OPENVN_TEST_CHECK(openvn_paula_trigger_note(
            &mixer,
            channel,
            &module,
            &notes[channel]
        ));
    }

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        voice = openvn_paula_voice(&mixer, channel);
        OPENVN_TEST_CHECK(voice != 0);
        OPENVN_TEST_CHECK(voice->active);
        OPENVN_TEST_CHECK(voice->data == sample_data);
        OPENVN_TEST_CHECK(voice->length == sizeof(sample_data));
        OPENVN_TEST_CHECK(voice->loop_data == sample_data + 4);
        OPENVN_TEST_CHECK(voice->loop_length == 8UL);
        OPENVN_TEST_CHECK(voice->period == 428U + channel * 16U);
        OPENVN_TEST_CHECK(voice->volume == 48U);
        OPENVN_TEST_CHECK(voice->generation == 1UL);
    }

    notes[0].period = 214U;
    OPENVN_TEST_CHECK(openvn_paula_trigger_note(&mixer, 0U, &module, &notes[0]));
    voice = openvn_paula_voice(&mixer, 0U);
    OPENVN_TEST_CHECK(voice->generation == 2UL);
    OPENVN_TEST_CHECK(voice->period == 214U);

    state.period = 320U;
    state.volume = 24U;
    OPENVN_TEST_CHECK(openvn_paula_apply_channel_state(&mixer, 0U, &state));
    voice = openvn_paula_voice(&mixer, 0U);
    OPENVN_TEST_CHECK(voice->period == 320U);
    OPENVN_TEST_CHECK(voice->volume == 24U);
    OPENVN_TEST_CHECK(voice->generation == 2UL);

    OPENVN_TEST_CHECK(!openvn_paula_trigger_note(
        &mixer,
        OPENVN_MOD_CHANNELS,
        &module,
        &notes[0]
    ));

    return 0;
}
