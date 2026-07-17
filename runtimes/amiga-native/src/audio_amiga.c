#include "openvn_audio_amiga.h"

#ifdef __AMIGA__

#include <devices/audio.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <proto/exec.h>

#include <string.h>

#define PAL_CLOCK 3546895UL

static unsigned short period_to_audio_period(unsigned short period) {
    if (period == 0U) {
        return 0U;
    }

    return period;
}

static int schedule_tick(OpenVNAmigaAudioContext *context) {
    unsigned long micros;

    if (context->timer_pending || !context->mod_player.playing) {
        return 1;
    }

    micros = 2500000UL / context->mod_player.bpm;

    context->timer_request->tr_node.io_Command = TR_ADDREQUEST;
    context->timer_request->tr_time.tv_secs = 0;
    context->timer_request->tr_time.tv_micro = micros;

    SendIO((struct IORequest *)context->timer_request);
    context->timer_pending = 1;
    return 1;
}

static int amiga_open(
    OpenVNAudioService *service,
    const OpenVNAudioConfig *config
) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0) {
        return 0;
    }

    memset(context, 0, sizeof(*context));
    context->assets = config->assets;
    context->channels[0] = 1U;
    context->channels[1] = 2U;
    context->channels[2] = 4U;
    context->channels[3] = 8U;
    openvn_8svx_reset(&context->sample);
    openvn_mod_player_reset(&context->mod_player);

    context->port = CreateMsgPort();
    if (context->port == 0) {
        return 0;
    }

    context->request = (struct IOAudio *)CreateIORequest(
        context->port,
        sizeof(struct IOAudio)
    );
    if (context->request == 0) {
        DeleteMsgPort(context->port);
        context->port = 0;
        return 0;
    }

    context->request->ioa_Request.io_Message.mn_Node.ln_Pri = 0;
    context->request->ioa_Data = context->channels;
    context->request->ioa_Length = 4U;

    if (OpenDevice(
            AUDIONAME,
            0U,
            (struct IORequest *)context->request,
            0U
        ) != 0) {
        DeleteIORequest((struct IORequest *)context->request);
        DeleteMsgPort(context->port);
        context->request = 0;
        context->port = 0;
        return 0;
    }

    context->timer_port = CreateMsgPort();
    if (context->timer_port == 0) {
        CloseDevice((struct IORequest *)context->request);
        DeleteIORequest((struct IORequest *)context->request);
        DeleteMsgPort(context->port);
        return 0;
    }

    context->timer_request = (struct timerequest *)CreateIORequest(
        context->timer_port,
        sizeof(struct timerequest)
    );
    if (context->timer_request == 0 ||
        OpenDevice(
            TIMERNAME,
            UNIT_MICROHZ,
            (struct IORequest *)context->timer_request,
            0U
        ) != 0) {
        if (context->timer_request != 0) {
            DeleteIORequest(
                (struct IORequest *)context->timer_request
            );
        }
        DeleteMsgPort(context->timer_port);
        CloseDevice((struct IORequest *)context->request);
        DeleteIORequest((struct IORequest *)context->request);
        DeleteMsgPort(context->port);
        context->timer_request = 0;
        context->timer_port = 0;
        return 0;
    }

    context->opened = 1;
    return 1;
}

static void stop_timer(OpenVNAmigaAudioContext *context) {
    if (context->timer_pending) {
        AbortIO((struct IORequest *)context->timer_request);
        WaitIO((struct IORequest *)context->timer_request);
        context->timer_pending = 0;
    }
}

static void amiga_close(OpenVNAudioService *service) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0) {
        return;
    }

    stop_timer(context);

    if (context->sound_pending) {
        AbortIO((struct IORequest *)context->request);
        WaitIO((struct IORequest *)context->request);
        context->sound_pending = 0;
    }

    openvn_mod_player_free(&context->mod_player);
    openvn_8svx_free(&context->sample);

    if (context->timer_request != 0) {
        CloseDevice((struct IORequest *)context->timer_request);
        DeleteIORequest((struct IORequest *)context->timer_request);
        context->timer_request = 0;
    }

    if (context->timer_port != 0) {
        DeleteMsgPort(context->timer_port);
        context->timer_port = 0;
    }

    if (context->request != 0) {
        CloseDevice((struct IORequest *)context->request);
        DeleteIORequest((struct IORequest *)context->request);
        context->request = 0;
    }

    if (context->port != 0) {
        DeleteMsgPort(context->port);
        context->port = 0;
    }

    context->opened = 0;
}

static int amiga_music(
    OpenVNAudioService *service,
    const char *track
) {
    OpenVNAmigaAudioContext *context;
    const char *path;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_music(context->assets, track);
    if (path == 0) {
        return 0;
    }

    if (!openvn_mod_player_load(&context->mod_player, path) ||
        !openvn_mod_player_start(&context->mod_player, 1)) {
        return 0;
    }

    return schedule_tick(context);
}

static int amiga_sound(
    OpenVNAudioService *service,
    const char *effect
) {
    OpenVNAmigaAudioContext *context;
    const char *path;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_sound(context->assets, effect);
    if (path == 0) {
        return 0;
    }

    if (context->sound_pending) {
        AbortIO((struct IORequest *)context->request);
        WaitIO((struct IORequest *)context->request);
        context->sound_pending = 0;
    }

    openvn_8svx_free(&context->sample);
    if (!openvn_8svx_load_file(&context->sample, path)) {
        return 0;
    }

    context->request->ioa_Request.io_Command = CMD_WRITE;
    context->request->ioa_Data = context->sample.data;
    context->request->ioa_Length = context->sample.data_size;
    context->request->ioa_Period =
        PAL_CLOCK / context->sample.sample_rate;
    context->request->ioa_Volume = 64U;
    context->request->ioa_Cycles = 1U;

    SendIO((struct IORequest *)context->request);
    context->sound_pending = 1;
    return 1;
}

static int amiga_stop_music(OpenVNAudioService *service) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    openvn_mod_player_stop(&context->mod_player);
    stop_timer(context);
    return 1;
}

static void trigger_row(OpenVNAmigaAudioContext *context) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        const OpenVNMODNote *note = openvn_mod_player_channel(
            &context->mod_player,
            channel
        );

        if (note == 0 || note->sample == 0U || note->period == 0U) {
            continue;
        }

        /*
         * This is the native Paula channel scheduling boundary.
         * M5 PR6.2 adds one IOAudio request per channel and actual
         * simultaneous four-channel sample writes.
         */
        context->request->ioa_Period =
            period_to_audio_period(note->period);
    }
}

static int amiga_update(OpenVNAudioService *service) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    if (context->sound_pending &&
        CheckIO((struct IORequest *)context->request) != 0) {
        WaitIO((struct IORequest *)context->request);
        context->sound_pending = 0;
    }

    if (context->timer_pending &&
        CheckIO((struct IORequest *)context->timer_request) != 0) {
        WaitIO((struct IORequest *)context->timer_request);
        context->timer_pending = 0;

        if (context->mod_player.tick == 0U) {
            trigger_row(context);
        }

        openvn_mod_player_tick(&context->mod_player);
        schedule_tick(context);
    }

    return 1;
}

static const OpenVNAudioVTable AMIGA_VTABLE = {
    amiga_open,
    amiga_close,
    amiga_music,
    amiga_sound,
    amiga_stop_music,
    amiga_update
};

void openvn_audio_amiga_init(
    OpenVNAudioService *service,
    OpenVNAmigaAudioContext *context
) {
    if (service != 0) {
        service->vtable = &AMIGA_VTABLE;
        service->context = context;
    }
}

#endif
