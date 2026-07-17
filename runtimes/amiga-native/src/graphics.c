#include "openvn_graphics.h"

int openvn_graphics_open(
    OpenVNGraphicsService *service,
    const OpenVNGraphicsConfig *config
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->open == 0 || config == 0) {
        return 0;
    }

    return service->vtable->open(service, config);
}

void openvn_graphics_close(OpenVNGraphicsService *service) {
    if (service != 0 && service->vtable != 0 &&
        service->vtable->close != 0) {
        service->vtable->close(service);
    }
}

int openvn_graphics_scene(
    OpenVNGraphicsService *service,
    const char *background
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->scene == 0 || background == 0) {
        return 0;
    }

    return service->vtable->scene(service, background);
}

int openvn_graphics_show(
    OpenVNGraphicsService *service,
    const char *character,
    const char *pose
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->show == 0 || character == 0 || pose == 0) {
        return 0;
    }

    return service->vtable->show(service, character, pose);
}

int openvn_graphics_hide(
    OpenVNGraphicsService *service,
    const char *character
) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->hide == 0 || character == 0) {
        return 0;
    }

    return service->vtable->hide(service, character);
}

int openvn_graphics_present(OpenVNGraphicsService *service) {
    if (service == 0 || service->vtable == 0 ||
        service->vtable->present == 0) {
        return 0;
    }

    return service->vtable->present(service);
}
