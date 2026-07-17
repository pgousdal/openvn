#include "openvn_mod_player.h"

#include <string.h>

static const unsigned short PERIODS[] = {
    1712U,1616U,1524U,1440U,1356U,1280U,1208U,1140U,1076U,1016U,960U,906U,
    856U,808U,762U,720U,678U,640U,604U,570U,538U,508U,480U,453U,
    428U,404U,381U,360U,339U,320U,302U,285U,269U,254U,240U,226U,
    214U,202U,190U,180U,170U,160U,151U,143U,135U,127U,120U,113U
};
#define PERIOD_COUNT (sizeof(PERIODS) / sizeof(PERIODS[0]))

static unsigned char clamp_volume(int value) {
    if (value < 0) return 0U;
    if (value > 64) return 64U;
    return (unsigned char)value;
}

static unsigned short clamp_period(int value) {
    if (value < 113) return 113U;
    if (value > 1712) return 1712U;
    return (unsigned short)value;
}

static int triangle(unsigned char position) {
    unsigned int phase = position & 63U;
    if (phase < 16U) return (int)phase;
    if (phase < 48U) return 32 - (int)phase;
    return (int)phase - 64;
}

static unsigned short transpose_period(unsigned short period, unsigned int semitones) {
    unsigned int i, nearest = 0U;
    unsigned long best = 65535UL;
    if (period == 0U) return 0U;
    for (i = 0U; i < PERIOD_COUNT; i++) {
        unsigned long d = PERIODS[i] > period ? PERIODS[i] - period : period - PERIODS[i];
        if (d < best) { best = d; nearest = i; }
    }
    nearest += semitones;
    if (nearest >= PERIOD_COUNT) nearest = PERIOD_COUNT - 1U;
    return PERIODS[nearest];
}

void openvn_mod_player_reset(OpenVNMODPlayer *player) {
    if (player != 0) { memset(player, 0, sizeof(*player)); player->speed = 6U; player->bpm = 125U; }
}
void openvn_mod_player_free(OpenVNMODPlayer *player) { if (player != 0) { openvn_mod_free(&player->module); openvn_mod_player_reset(player); } }
int openvn_mod_player_load(OpenVNMODPlayer *player, const char *path) { if (player == 0 || path == 0) return 0; openvn_mod_player_free(player); return openvn_mod_load_file(&player->module, path); }
int openvn_mod_player_start(OpenVNMODPlayer *player, int loop) {
    if (player == 0 || player->module.patterns == 0) return 0;
    memset(player->channels, 0, sizeof(player->channels)); player->order=0U; player->row=0U; player->tick=0U; player->speed=6U; player->bpm=125U; player->pattern_delay_rows=0U; player->position_pending=0; player->playing=1; player->loop=loop; return 1;
}
void openvn_mod_player_stop(OpenVNMODPlayer *player) { if (player != 0) player->playing = 0; }
static void schedule_position(OpenVNMODPlayer *p, unsigned int order, unsigned int row) { p->next_order=order; p->next_row=row<OPENVN_MOD_ROWS?row:0U; p->position_pending=1; }

static void volume_slide(OpenVNMODChannelState *s, unsigned char parameter) {
    unsigned int up = parameter >> 4, down = parameter & 15U;
    s->volume = clamp_volume((int)s->volume + (up ? (int)up : -(int)down));
}
static void tone_portamento(OpenVNMODChannelState *s) {
    if (s->target_period == 0U || s->porta_speed == 0U) return;
    if (s->base_period < s->target_period) {
        unsigned int n = s->base_period + s->porta_speed;
        s->base_period = (unsigned short)(n > s->target_period ? s->target_period : n);
    } else if (s->base_period > s->target_period) {
        int n = (int)s->base_period - (int)s->porta_speed;
        s->base_period = (unsigned short)(n < (int)s->target_period ? s->target_period : n);
    }
    s->period = s->base_period;
}
static void vibrato(OpenVNMODChannelState *s) {
    int delta = triangle(s->vibrato_position) * (int)s->vibrato_depth / 8;
    s->period = clamp_period((int)s->base_period + delta);
    s->vibrato_position = (unsigned char)(s->vibrato_position + s->vibrato_speed);
}
static void tremolo(OpenVNMODChannelState *s) {
    int delta = triangle(s->tremolo_position) * (int)s->tremolo_depth / 16;
    s->volume = clamp_volume((int)s->volume + delta);
    s->tremolo_position = (unsigned char)(s->tremolo_position + s->tremolo_speed);
}

static void extended_row(OpenVNMODPlayer *p, OpenVNMODChannelState *s, const OpenVNMODNote *n) {
    unsigned int sub = n->parameter >> 4, x = n->parameter & 15U;
    switch (sub) {
        case 0x1U: s->base_period = s->period = clamp_period((int)s->base_period - (int)x); break;
        case 0x2U: s->base_period = s->period = clamp_period((int)s->base_period + (int)x); break;
        case 0x6U:
            if (x == 0U) s->pattern_loop_row = p->row;
            else if (s->pattern_loop_count == 0U) { s->pattern_loop_count = x; schedule_position(p, p->order, s->pattern_loop_row); }
            else if (--s->pattern_loop_count > 0U) schedule_position(p, p->order, s->pattern_loop_row);
            break;
        case 0xAU: s->volume = clamp_volume((int)s->volume + (int)x); break;
        case 0xBU: s->volume = clamp_volume((int)s->volume - (int)x); break;
        case 0xCU: s->note_cut_tick = (unsigned char)x; break;
        case 0xDU: s->note_delay_tick = (unsigned char)x; s->note_delayed = 1; s->triggered = 0; break;
        case 0xEU: p->pattern_delay_rows = x; break;
        default: break;
    }
}

static void begin_row(OpenVNMODPlayer *p) {
    unsigned int c;
    for (c=0U;c<OPENVN_MOD_CHANNELS;c++) {
        const OpenVNMODNote *n = openvn_mod_player_channel(p,c); OpenVNMODChannelState *s=&p->channels[c];
        s->triggered=0; s->note_delayed=0; s->note_delay_tick=0U; s->note_cut_tick=0U; s->retrigger_interval=0U;
        if (n==0) continue;
        s->effect=n->effect; s->parameter=n->parameter;
        if (n->sample>0U && n->sample<=OPENVN_MOD_SAMPLE_COUNT) { s->sample=n->sample; s->pending_sample=n->sample; s->volume=p->module.samples[n->sample-1U].volume; }
        if (n->effect==0x03U || n->effect==0x05U) {
            if (n->period>0U) s->target_period=n->period;
        } else if (n->period>0U) { s->base_period=n->period; s->period=n->period; s->triggered=1; }
        switch(n->effect) {
            case 0x03U: if(n->parameter) s->porta_speed=n->parameter; break;
            case 0x04U: if(n->parameter>>4) s->vibrato_speed=n->parameter>>4; if(n->parameter&15U) s->vibrato_depth=n->parameter&15U; break;
            case 0x07U: if(n->parameter>>4) s->tremolo_speed=n->parameter>>4; if(n->parameter&15U) s->tremolo_depth=n->parameter&15U; break;
            case 0x09U: if(n->parameter) s->sample_offset=(unsigned int)n->parameter*256U; break;
            case 0x0BU: schedule_position(p,n->parameter,0U); break;
            case 0x0CU: s->volume=clamp_volume(n->parameter); break;
            case 0x0DU: schedule_position(p,p->order+1U,(n->parameter>>4)*10U+(n->parameter&15U)); break;
            case 0x0EU: extended_row(p,s,n); break;
            case 0x0FU: if(n->parameter>=32U)p->bpm=n->parameter; else if(n->parameter>0U)p->speed=n->parameter; break;
            default: break;
        }
    }
}

static void apply_tick_effects(OpenVNMODPlayer *p) {
    unsigned int c;
    for(c=0U;c<OPENVN_MOD_CHANNELS;c++) {
        OpenVNMODChannelState *s=&p->channels[c]; unsigned int amount, sub=s->parameter>>4, x=s->parameter&15U; s->triggered=0;
        if(s->note_delayed && p->tick==s->note_delay_tick){s->note_delayed=0;s->triggered=1;}
        if(s->note_cut_tick>0U && p->tick==s->note_cut_tick)s->volume=0U;
        if(s->effect==0x0EU && sub==0x9U && x>0U && p->tick%x==0U)s->triggered=1;
        switch(s->effect){
            case 0x00U: if(s->parameter){unsigned int step=p->tick%3U,semi=step==1U?s->parameter>>4:step==2U?s->parameter&15U:0U;s->period=transpose_period(s->base_period,semi);} break;
            case 0x01U: amount=s->parameter;s->base_period=s->period=clamp_period((int)s->base_period-(int)amount);break;
            case 0x02U: amount=s->parameter;s->base_period=s->period=clamp_period((int)s->base_period+(int)amount);break;
            case 0x03U: tone_portamento(s);break;
            case 0x04U: vibrato(s);break;
            case 0x05U: tone_portamento(s);volume_slide(s,s->parameter);break;
            case 0x06U: vibrato(s);volume_slide(s,s->parameter);break;
            case 0x07U: tremolo(s);break;
            case 0x0AU: volume_slide(s,s->parameter);break;
            default:break;
        }
    }
}

static int normalize_position(OpenVNMODPlayer *p){if(p->order<p->module.song_length)return 1;if(!p->loop){p->playing=0;return 0;}p->order=p->module.restart_position;if(p->order>=p->module.song_length)p->order=0U;return 1;}
int openvn_mod_player_tick(OpenVNMODPlayer *p){
    if(p==0||!p->playing)return 0;
    if(p->tick==0U){if(p->pattern_delay_rows==0U)begin_row(p);}else apply_tick_effects(p);
    p->tick++; if(p->tick<p->speed)return 1; p->tick=0U;
    if(p->pattern_delay_rows>0U){p->pattern_delay_rows--;if(p->pattern_delay_rows>0U)return 1;}
    if(p->position_pending){p->order=p->next_order;p->row=p->next_row;p->position_pending=0;normalize_position(p);return 1;}
    p->row++;if(p->row<OPENVN_MOD_ROWS)return 1;p->row=0U;p->order++;normalize_position(p);return 1;
}
const OpenVNMODNote *openvn_mod_player_channel(const OpenVNMODPlayer *p,unsigned int c){unsigned int pattern;if(p==0||p->module.song_length==0U||p->order>=p->module.song_length)return 0;pattern=p->module.pattern_table[p->order];return openvn_mod_note(&p->module,pattern,p->row,c);}
const OpenVNMODChannelState *openvn_mod_player_channel_state(const OpenVNMODPlayer *p,unsigned int c){if(p==0||c>=OPENVN_MOD_CHANNELS)return 0;return &p->channels[c];}
