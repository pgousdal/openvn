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

static void stop_timer(OpenVNAmigaAudioContext *context) {
    if (context->timer_pending) {
        AbortIO((struct IORequest *)context->timer_request);
        WaitIO((struct IORequest *)context->timer_request);
        context->timer_pending = 0;
    }
}

static void stop_channel(
    OpenVNAmigaAudioContext *context,
    unsigned int channel
) {
    if (context->loop_pending[channel]) {
        AbortIO((struct IORequest *)context->loop_requests[channel]);
        WaitIO((struct IORequest *)context->loop_requests[channel]);
        context->loop_pending[channel] = 0;
    }

    if (context->channel_pending[channel]) {
        AbortIO((struct IORequest *)context->channel_requests[channel]);
        WaitIO((struct IORequest *)context->channel_requests[channel]);
        context->channel_pending[channel] = 0;
    }
}

static void stop_all_channels(OpenVNAmigaAudioContext *context) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        stop_channel(context, channel);
    }
    context->sound_pending = 0;
}

static int open_channel(
    OpenVNAmigaAudioContext *context,
    unsigned int channel
) {
    struct IOAudio *request;
    struct IOAudio *loop_request;

    context->channel_ports[channel] = CreateMsgPort();
    if (context->channel_ports[channel] == 0) {
        return 0;
    }

    request = (struct IOAudio *)CreateIORequest(
        context->channel_ports[channel],
        sizeof(struct IOAudio)
    );
    if (request == 0) {
        return 0;
    }
    context->channel_requests[channel] = request;

    request->ioa_Request.io_Message.mn_Node.ln_Pri = 0;
    request->ioa_Data = &context->channel_masks[channel];
    request->ioa_Length = 1U;

    if (OpenDevice(
            AUDIONAME,
            0U,
            (struct IORequest *)request,
            0U
        ) != 0) {
        return 0;
    }

    loop_request = (struct IOAudio *)CreateIORequest(
        context->channel_ports[channel],
        sizeof(struct IOAudio)
    );
    if (loop_request == 0) {
        return 0;
    }
    context->loop_requests[channel] = loop_request;
    loop_request->ioa_Request.io_Device = request->ioa_Request.io_Device;
    loop_request->ioa_Request.io_Unit = request->ioa_Request.io_Unit;
    loop_request->ioa_AllocKey = request->ioa_AllocKey;
    return 1;
}

static void close_channels(OpenVNAmigaAudioContext *context) {
    unsigned int channel;

    stop_all_channels(context);
    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        if (context->loop_requests[channel] != 0) {
            DeleteIORequest(
                (struct IORequest *)context->loop_requests[channel]
            );
            context->loop_requests[channel] = 0;
        }
        if (context->channel_requests[channel] != 0) {
            CloseDevice(
                (struct IORequest *)context->channel_requests[channel]
            );
            DeleteIORequest(
                (struct IORequest *)context->channel_requests[channel]
            );
            context->channel_requests[channel] = 0;
        }
        if (context->channel_ports[channel] != 0) {
            DeleteMsgPort(context->channel_ports[channel]);
            context->channel_ports[channel] = 0;
        }
    }
}

static int amiga_open(
    OpenVNAudioService *service,
    const OpenVNAudioConfig *config
) {
    OpenVNAmigaAudioContext *context;
    unsigned int channel;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || config == 0) {
        return 0;
    }

    memset(context, 0, sizeof(*context));
    context->assets = config->assets;
    context->channel_masks[0] = 1U;
    context->channel_masks[1] = 2U;
    context->channel_masks[2] = 4U;
    context->channel_masks[3] = 8U;
    openvn_8svx_reset(&context->sample);
    openvn_mod_player_reset(&context->mod_player);
    openvn_paula_reset(&context->paula);

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        if (!open_channel(context, channel)) {
            close_channels(context);
            return 0;
        }
    }

    context->timer_port = CreateMsgPort();
    if (context->timer_port == 0) {
        close_channels(context);
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
            DeleteIORequest((struct IORequest *)context->timer_request);
        }
        DeleteMsgPort(context->timer_port);
        context->timer_request = 0;
        context->timer_port = 0;
        close_channels(context);
        return 0;
    }

    context->opened = 1;
    return 1;
}

static void amiga_close(OpenVNAudioService *service) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0) {
        return;
    }

    stop_timer(context);
    close_channels(context);
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

    stop_all_channels(context);
    openvn_paula_reset(&context->paula);
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
    struct IOAudio *request;
    const char *path;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    path = openvn_asset_find_sound(context->assets, effect);
    if (path == 0) {
        return 0;
    }

    openvn_mod_player_stop(&context->mod_player);
    stop_timer(context);
    stop_all_channels(context);
    openvn_8svx_free(&context->sample);
    if (!openvn_8svx_load_file(&context->sample, path)) {
        return 0;
    }

    request = context->channel_requests[0];
    request->ioa_Request.io_Command = CMD_WRITE;
    request->ioa_Data = context->sample.data;
    request->ioa_Length = context->sample.data_size;
    request->ioa_Period = PAL_CLOCK / context->sample.sample_rate;
    request->ioa_Volume = 64U;
    request->ioa_Cycles = 1U;
    SendIO((struct IORequest *)request);
    context->channel_pending[0] = 1;
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
    stop_all_channels(context);
    openvn_paula_reset(&context->paula);
    return 1;
}

static void queue_voice(
    OpenVNAmigaAudioContext *context,
    unsigned int channel,
    const OpenVNPaulaVoice *voice
) {
    struct IOAudio *request;
    struct IOAudio *loop_request;

    stop_channel(context, channel);
    request = context->channel_requests[channel];
    request->ioa_Request.io_Command = CMD_WRITE;
    request->ioa_Data = (unsigned char *)voice->data;
    request->ioa_Length = voice->length;
    request->ioa_Period = period_to_audio_period(voice->period);
    request->ioa_Volume = voice->volume;
    request->ioa_Cycles = 1U;
    SendIO((struct IORequest *)request);
    context->channel_pending[channel] = 1;

    if (voice->loop_data != 0 && voice->loop_length > 2UL) {
        loop_request = context->loop_requests[channel];
        loop_request->ioa_Request.io_Command = CMD_WRITE;
        loop_request->ioa_Data = (unsigned char *)voice->loop_data;
        loop_request->ioa_Length = voice->loop_length;
        loop_request->ioa_Period = period_to_audio_period(voice->period);
        loop_request->ioa_Volume = voice->volume;
        loop_request->ioa_Cycles = 0U;
        SendIO((struct IORequest *)loop_request);
        context->loop_pending[channel] = 1;
    }
}

static void trigger_row(OpenVNAmigaAudioContext *context) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        const OpenVNMODNote *note;
        const OpenVNPaulaVoice *voice;

        note = openvn_mod_player_channel(&context->mod_player, channel);
        if (!openvn_paula_trigger_note(
                &context->paula,
                channel,
                &context->mod_player.module,
                note
            )) {
            continue;
        }

        voice = openvn_paula_voice(&context->paula, channel);
        if (voice != 0 &&
            voice->generation != context->voice_generations[channel]) {
            queue_voice(context, channel, voice);
            context->voice_generations[channel] = voice->generation;
        }
    }
}

static void collect_channel_completions(
    OpenVNAmigaAudioContext *context
) {
    unsigned int channel;

    for (channel = 0U; channel < OPENVN_MOD_CHANNELS; channel++) {
        if (context->channel_pending[channel] &&
            CheckIO(
                (struct IORequest *)context->channel_requests[channel]
            ) != 0) {
            WaitIO((struct IORequest *)context->channel_requests[channel]);
            context->channel_pending[channel] = 0;
        }
    }
    if (!context->channel_pending[0]) {
        context->sound_pending = 0;
    }
}

static int amiga_update(OpenVNAudioService *service) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || !context->opened) {
        return 0;
    }

    collect_channel_completions(context);
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
