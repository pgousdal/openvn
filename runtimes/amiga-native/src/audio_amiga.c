#include "openvn_audio_amiga.h"

#ifdef __AMIGA__

#include <devices/audio.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <proto/exec.h>

#include <stdio.h>
#include <string.h>

#define PAL_CLOCK 3546895UL
#define OPENVN_AUDIO_LOG "openvn-audio.log"
#define OPENVN_AUDIO_DIAGNOSTIC_TICKS 32U

static void audio_trace(const char *message) {
    FILE *file = fopen(OPENVN_AUDIO_LOG, "a");
    if (file != 0) {
        fputs(message, file);
        fputc('\n', file);
        fclose(file);
    }
}

static void audio_trace_values(
    const char *stage,
    unsigned long first,
    unsigned long second,
    unsigned long third,
    unsigned long fourth
) {
    FILE *file = fopen(OPENVN_AUDIO_LOG, "a");
    if (file != 0) {
        fprintf(
            file,
            "%s %lu %lu %lu %lu\n",
            stage,
            first,
            second,
            third,
            fourth
        );
        fclose(file);
    }
}

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

static void release_music_chip_data(OpenVNAmigaAudioContext *context) {
    unsigned int sample;

    for (sample = 0U; sample < OPENVN_MOD_SAMPLE_COUNT; sample++) {
        OpenVNMODSample *mod_sample;

        mod_sample = &context->mod_player.module.samples[sample];
        if (context->music_chip_data[sample] != 0) {
            mod_sample->data = context->music_host_data[sample];
            FreeMem(context->music_chip_data[sample], mod_sample->length);
            context->music_chip_data[sample] = 0;
            context->music_host_data[sample] = 0;
        }
    }
}

static int prepare_music_chip_data(OpenVNAmigaAudioContext *context) {
    unsigned int sample;

    for (sample = 0U; sample < OPENVN_MOD_SAMPLE_COUNT; sample++) {
        OpenVNMODSample *mod_sample;
        unsigned char *chip_data;

        mod_sample = &context->mod_player.module.samples[sample];
        if (mod_sample->length == 0UL) {
            continue;
        }
        chip_data = (unsigned char *)AllocMem(
            mod_sample->length,
            MEMF_CHIP | MEMF_PUBLIC
        );
        if (chip_data == 0) {
            audio_trace("MUSIC failed: chip sample allocation");
            release_music_chip_data(context);
            return 0;
        }
        CopyMem(mod_sample->data, chip_data, mod_sample->length);
        context->music_host_data[sample] = mod_sample->data;
        context->music_chip_data[sample] = chip_data;
        mod_sample->data = chip_data;
    }
    audio_trace("MUSIC samples copied to chip memory");
    return 1;
}

static void release_sound_chip_data(OpenVNAmigaAudioContext *context) {
    if (context->sound_chip_data != 0) {
        FreeMem(context->sound_chip_data, context->sound_chip_size);
        context->sound_chip_data = 0;
        context->sound_chip_size = 0UL;
    }
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
        audio_trace_values("CHANNEL open failed", channel, 0UL, 0UL, 0UL);
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
    audio_trace_values(
        "CHANNEL open ok",
        channel,
        (unsigned long)request->ioa_AllocKey,
        0UL,
        0UL
    );
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
    {
        FILE *file;

        file = fopen(OPENVN_AUDIO_LOG, "w");
        if (file != 0) {
            fputs("OpenVN Amiga audio diagnostics\n", file);
            fclose(file);
        }
    }
    audio_trace("OPEN begin");
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
    audio_trace("OPEN audio.device and timer.device ok");
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
    release_music_chip_data(context);
    openvn_mod_player_free(&context->mod_player);
    release_sound_chip_data(context);
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
    audio_trace("CLOSE ok");
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
        audio_trace("MUSIC failed: asset not found");
        return 0;
    }

    audio_trace("MUSIC asset found");
    stop_timer(context);
    stop_all_channels(context);
    release_music_chip_data(context);
    openvn_paula_reset(&context->paula);
    if (!openvn_mod_player_load(&context->mod_player, path) ||
        !prepare_music_chip_data(context) ||
        !openvn_mod_player_start(&context->mod_player, 1)) {
        audio_trace("MUSIC failed: load/chip-copy/start");
        return 0;
    }

    context->diagnostic_ticks = 0U;
    context->diagnostic_voices = 0U;
    audio_trace_values(
        "MUSIC recognized length/patterns/bpm",
        context->mod_player.module.song_length,
        context->mod_player.module.pattern_count,
        context->mod_player.bpm,
        0UL
    );

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

    stop_channel(context, 0U);
    release_sound_chip_data(context);
    openvn_8svx_free(&context->sample);
    if (!openvn_8svx_load_file(&context->sample, path)) {
        return 0;
    }

    context->sound_chip_data = (unsigned char *)AllocMem(
        context->sample.data_size,
        MEMF_CHIP | MEMF_PUBLIC
    );
    if (context->sound_chip_data == 0) {
        audio_trace("SOUND failed: chip sample allocation");
        return 0;
    }
    context->sound_chip_size = context->sample.data_size;
    CopyMem(
        context->sample.data,
        context->sound_chip_data,
        context->sample.data_size
    );

    request = context->channel_requests[0];
    request->ioa_Request.io_Command = CMD_WRITE;
    request->ioa_Request.io_Flags = ADIOF_PERVOL;
    request->ioa_Data = context->sound_chip_data;
    request->ioa_Length = context->sample.data_size;
    request->ioa_Period = PAL_CLOCK / context->sample.sample_rate;
    request->ioa_Volume = 64U;
    request->ioa_Cycles = 1U;
    BeginIO((struct IORequest *)request);
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
    audio_trace("MUSIC stopped");
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
    request->ioa_Request.io_Flags = ADIOF_PERVOL;
    request->ioa_Data = (unsigned char *)voice->data;
    request->ioa_Length = voice->length;
    request->ioa_Period = period_to_audio_period(voice->period);
    request->ioa_Volume = voice->volume;
    request->ioa_Cycles = 1U;
    BeginIO((struct IORequest *)request);
    context->channel_pending[channel] = 1;

    if (voice->loop_data != 0 && voice->loop_length > 2UL) {
        loop_request = context->loop_requests[channel];
        loop_request->ioa_Request.io_Command = CMD_WRITE;
        loop_request->ioa_Request.io_Flags = ADIOF_PERVOL;
        loop_request->ioa_Data = (unsigned char *)voice->loop_data;
        loop_request->ioa_Length = voice->loop_length;
        loop_request->ioa_Period = period_to_audio_period(voice->period);
        loop_request->ioa_Volume = voice->volume;
        loop_request->ioa_Cycles = 0U;
        BeginIO((struct IORequest *)loop_request);
        context->loop_pending[channel] = 1;
    }

    if (context->diagnostic_voices < OPENVN_AUDIO_DIAGNOSTIC_TICKS) {
        audio_trace_values(
            "VOICE channel/length/period/volume",
            channel,
            voice->length,
            voice->period,
            voice->volume
        );
        context->diagnostic_voices++;
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

        unsigned int row_start;

        row_start = context->mod_player.tick == 0U;
        openvn_mod_player_tick(&context->mod_player);
        if (row_start) {
            trigger_row(context);
        }
        if (context->diagnostic_ticks < OPENVN_AUDIO_DIAGNOSTIC_TICKS) {
            audio_trace_values(
                "TICK count/order/row/tick",
                context->diagnostic_ticks,
                context->mod_player.order,
                context->mod_player.row,
                context->mod_player.tick
            );
            context->diagnostic_ticks++;
        }
        schedule_tick(context);
    }

    return 1;
}

static unsigned long amiga_signal_mask(OpenVNAudioService *service) {
    OpenVNAmigaAudioContext *context;

    context = (OpenVNAmigaAudioContext *)service->context;
    if (context == 0 || context->timer_port == 0) {
        return 0UL;
    }

    return 1UL << context->timer_port->mp_SigBit;
}

static const OpenVNAudioVTable AMIGA_VTABLE = {
    amiga_open,
    amiga_close,
    amiga_music,
    amiga_sound,
    amiga_stop_music,
    amiga_update,
    amiga_signal_mask
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
