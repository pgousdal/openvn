#include "assets.generated.h"
#include "openvn_audio.h"
#include "openvn_audio_host.h"

#include "test_check.h"
#include <string.h>

int main(void) {
    OpenVNAudioService service = {0};
    OpenVNHostAudioContext context = {0};
    OpenVNAudioConfig config = {0};

    openvn_audio_host_init(&service, &context);

    config.assets = &OPENVN_GENERATED_ASSETS;
    config.sample_rate = 11025U;
    config.channels = 4U;

    OPENVN_TEST_CHECK(openvn_audio_open(&service, &config));
    OPENVN_TEST_CHECK(context.opened);

    OPENVN_TEST_CHECK(openvn_audio_music(&service, "storm"));
    OPENVN_TEST_CHECK(strcmp(context.music, "storm") == 0);
    OPENVN_TEST_CHECK(context.mod_player.playing);
    OPENVN_TEST_CHECK(context.mod_player.module.pattern_count == 1U);

    OPENVN_TEST_CHECK(openvn_audio_sound(&service, "thunder"));
    OPENVN_TEST_CHECK(strcmp(context.sound, "thunder") == 0);
    OPENVN_TEST_CHECK(context.sample.sample_rate == 8000U);
    OPENVN_TEST_CHECK(context.sample.data_size == 32U);

    OPENVN_TEST_CHECK(openvn_audio_signal_mask(&service) == 0UL);
    OPENVN_TEST_CHECK(openvn_audio_update(&service));
    OPENVN_TEST_CHECK(context.update_count == 1U);

    OPENVN_TEST_CHECK(openvn_audio_stop_music(&service));
    OPENVN_TEST_CHECK(context.music[0] == '\0');
    OPENVN_TEST_CHECK(!context.mod_player.playing);

    openvn_audio_close(&service);
    OPENVN_TEST_CHECK(!context.opened);

    return 0;
}
