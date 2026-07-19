#include "openvn_audio.h"
#include "openvn_graphics.h"
#include "openvn_player.h"

#include "test_check.h"

static int update_count;

static int fake_open(
    OpenVNAudioService *service,
    const OpenVNAudioConfig *config
) {
    (void)service;
    (void)config;
    return 1;
}

static void fake_close(OpenVNAudioService *service) {
    (void)service;
}

static int fake_music(OpenVNAudioService *service, const char *track) {
    (void)service;
    (void)track;
    return 1;
}

static int fake_sound(OpenVNAudioService *service, const char *effect) {
    (void)service;
    (void)effect;
    return 1;
}

static int fake_stop(OpenVNAudioService *service) {
    (void)service;
    return 1;
}

static int fake_update(OpenVNAudioService *service) {
    (void)service;
    update_count++;
    return 1;
}

static unsigned long fake_signal_mask(OpenVNAudioService *service) {
    (void)service;
    return 0x20UL;
}

static const OpenVNAudioVTable AUDIO_VTABLE = {
    fake_open,
    fake_close,
    fake_music,
    fake_sound,
    fake_stop,
    fake_update,
    fake_signal_mask
};

int main(void) {
    OpenVNPlayer player;
    OpenVNAudioService audio;

    audio.vtable = &AUDIO_VTABLE;
    audio.context = 0;
    openvn_player_init(&player, (OpenVNGraphicsService *)0, &audio);

    OPENVN_TEST_CHECK(openvn_player_signal_mask(&player) == 0x20UL);
    OPENVN_TEST_CHECK(openvn_player_update(&player));
    OPENVN_TEST_CHECK(update_count == 1);
    return 0;
}
