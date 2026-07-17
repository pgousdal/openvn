#include "openvn_audio.h"

int openvn_audio_open(
    OpenVNAudioService *service,
    const OpenVNAudioConfig *config
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->open == 0 || config == 0) {
        return 0;
    }

    return service->vtable->open(service, config);
}

void openvn_audio_close(OpenVNAudioService *service) {
    if (service != 0 && service->vtable != 0 &&
        service->vtable->close != 0) {
        service->vtable->close(service);
    }
}

int openvn_audio_music(
    OpenVNAudioService *service,
    const char *track
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->music == 0 || track == 0) {
        return 0;
    }

    return service->vtable->music(service, track);
}

int openvn_audio_sound(
    OpenVNAudioService *service,
    const char *effect
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->sound == 0 || effect == 0) {
        return 0;
    }

    return service->vtable->sound(service, effect);
}

int openvn_audio_stop_music(OpenVNAudioService *service) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->stop_music == 0) {
        return 0;
    }

    return service->vtable->stop_music(service);
}

int openvn_audio_update(OpenVNAudioService *service) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->update == 0) {
        return 0;
    }

    return service->vtable->update(service);
}
