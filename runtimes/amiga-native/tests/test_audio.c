#include "assets.generated.h"
#include "openvn_audio.h"
#include "openvn_audio_host.h"

#include <assert.h>
#include <string.h>

int main(void) {
    OpenVNAudioService service;
    OpenVNHostAudioContext context;
    OpenVNAudioConfig config;

    openvn_audio_host_init(&service, &context);

    config.assets = &OPENVN_GENERATED_ASSETS;
    config.sample_rate = 11025U;
    config.channels = 4U;

    assert(openvn_audio_open(&service, &config));
    assert(context.opened);

    assert(openvn_audio_music(&service, "storm"));
    assert(strcmp(context.music, "storm") == 0);
    assert(context.mod_player.playing);
    assert(context.mod_player.module.pattern_count == 1U);

    assert(openvn_audio_sound(&service, "thunder"));
    assert(strcmp(context.sound, "thunder") == 0);
    assert(context.sample.sample_rate == 8000U);
    assert(context.sample.data_size == 32U);

    assert(openvn_audio_update(&service));
    assert(context.update_count == 1U);

    assert(openvn_audio_stop_music(&service));
    assert(context.music[0] == '\0');
    assert(!context.mod_player.playing);

    openvn_audio_close(&service);
    assert(!context.opened);

    return 0;
}
