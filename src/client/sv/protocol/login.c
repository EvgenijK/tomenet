#include "protocol/login.h"
#include "../../common/pack.h"
#define CLIENT_SIDE
#define USE_SDL3
#ifndef MINGW
#define LINUX
#endif
#include "../../../common/defines.h"
#include "protocol/resource-report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define SV_LOGIN_INPUT_CAPACITY (64 * 1024)
#define SV_LOGIN_OUTPUT_CAPACITY (32 * 1024)
#include "../../../temporary/sv/stubs/play-profile.h"
struct SvLogin {
    int version[6];
    unsigned char options[SV_SETTINGS_OPTION_COUNT];
    SvResourceReport report;
    SvVisualDefinitions visuals;
    SvLoginState state;
    unsigned char input[SV_LOGIN_INPUT_CAPACITY];
    size_t input_size;
    unsigned char output[SV_LOGIN_OUTPUT_CAPACITY];
    size_t output_size;
    uint32_t flags[4]; /* sflags3, sflags2, sflags1, sflags0 */
    SvLoginCharacter characters[SV_LOGIN_MAX_CHARACTERS];
    SvLoginCharacter creation;
    size_t count;
    size_t selected_slot;
    bool has_selected_slot, creating_character, reincarnation_requested;
    bool play_stream_complete, play_wire_configured;
    SvStartupPrefixConfig play_wire;
    bool flags_received;
    SvLoginTransfer transfer;
    char reason[256];
    SvLoginPingTelemetry ping;
};

static uint32_t get32(const unsigned char *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}
static void receive_pong(SvLogin *login, const unsigned char *packet)
{
    /* Startup has not sent a lagometer ping: baseline ping_id is still zero.
     * Future/older correlation IDs must never index the bounded sample array. */
    uint32_t index = 0u - get32(packet + 2);
    if (index >= 60) return;
    int32_t seconds, microseconds;
#ifdef WIN32
    DWORD ticks = GetTickCount();
    seconds = (int32_t)(ticks / 1000);
    microseconds = (int32_t)((ticks % 1000) * 1000);
#else
    struct timeval now;
    if (gettimeofday(&now, NULL)) return;
    seconds = (int32_t)now.tv_sec;
    microseconds = (int32_t)now.tv_usec;
#endif
    int64_t elapsed = ((int64_t)seconds - (int32_t)get32(packet + 6)) * 1000 +
        ((int64_t)microseconds - (int32_t)get32(packet + 10)) / 1000;
    if (!elapsed) elapsed = 1;
    if (elapsed > INT_MAX) elapsed = INT_MAX;
    if (elapsed < INT_MIN) elapsed = INT_MIN;
    int rtt = (int)elapsed;
    SvLoginPingTelemetry *ping = &login->ping;
    ping->samples[index] = ping->latest_ms = rtt;
    if ((int64_t)rtt < (int64_t)ping->average_ms * 7 / 10) {
        ping->average_ms = rtt;
        ping->average_count = 1;
    }
    if ((int64_t)rtt <= (int64_t)ping->average_ms * 13 / 10 ||
        ping->average_count < 10) {
        if (ping->average_count == 600)
            ping->average_ms = (int)(((int64_t)ping->average_ms * 599 + rtt) / 600);
        else {
            ping->average_ms = (int)(((int64_t)ping->average_ms * ping->average_count + rtt) /
                (ping->average_count + 1));
            ++ping->average_count;
        }
    }
}
SvLoginPingTelemetry sv_login_ping(const SvLogin *login) { return login->ping; }
static int16_t get16(const unsigned char *p)
{
    return (int16_t)(((unsigned)p[0] << 8) | p[1]);
}
static bool newer(const int v[6], const int threshold[6])
{
    for (int i = 0; i < 6; ++i)
        if (v[i] != threshold[i]) return v[i] > threshold[i];
    return false;
}
static bool atleast(const int v[6], const int threshold[6])
{
    return !newer(threshold, v);
}
static void consume(SvLogin *login, size_t size)
{
    login->input_size -= size;
    memmove(login->input, login->input + size, login->input_size);
}
static SvResult append_output(SvLogin *login, const void *bytes, size_t size)
{
    if (size > sizeof(login->output) - login->output_size) return SV_BACKPRESSURE;
    memcpy(login->output + login->output_size, bytes, size);
    login->output_size += size;
    return SV_OK;
}
static SvResult drain_transfer(SvLogin *login)
{
    if (!login->transfer.take_output) return SV_OK;
    SvOutput output = login->transfer.take_output(login->transfer.context,
        login->output + login->output_size, sizeof(login->output) - login->output_size);
    if (output.result == SV_WAITING) return SV_OK;
    if (output.result == SV_OUTPUT_TOO_SMALL) return SV_BACKPRESSURE;
    if (output.result == SV_OK) login->output_size += output.size;
    return output.result;
}
SvResult sv_login_attach_transfer(SvLogin *login, SvLoginTransfer transfer)
{
    if (!login || login->transfer.context || !transfer.context || !transfer.flags ||
        !transfer.receive || !transfer.pump || !transfer.take_output ||
        login->flags_received) return SV_INVALID;
    login->transfer = transfer;
    return SV_OK;
}
void sv_login_detach_transfer(SvLogin *login)
{
    if (login) login->transfer = (SvLoginTransfer){0};
}
SvTransferSummary sv_login_transfer_summary(const SvLogin *login)
{
    return login && login->transfer.summary ?
        login->transfer.summary(login->transfer.context) : (SvTransferSummary){0};
}
SvLuaResult sv_login_lua_begin_result(const SvLogin *login)
{
    return login && login->transfer.begin_result ?
        login->transfer.begin_result(login->transfer.context) :
        (SvLuaResult){.status = SV_LUA_NOT_READY};
}
SvResult sv_login_upload(SvLogin *login, const char *filename)
{
    if (!login || !login->flags_received || !login->transfer.upload) return SV_INVALID;
    return login->transfer.upload(login->transfer.context, filename);
}
SvResult sv_login_tick(SvLogin *login)
{
    if (!login) return SV_INVALID;
    if (login->state == SV_LOGIN_REJECTED || login->state == SV_LOGIN_PEER_QUIT)
        return SV_OK;
    SvResult resumed = sv_login_receive(login, NULL, 0);
    if (resumed != SV_OK) return resumed;
    if (login->transfer.pump) {
        SvResult pumped = login->transfer.pump(login->transfer.context);
        if (pumped != SV_OK && pumped != SV_WAITING) return pumped;
    }
    return drain_transfer(login);
}
/* A split NUL-terminated wire field must not publish a partial row. */
static int string(const unsigned char *bytes, size_t size, size_t *at,
                  char *out, size_t capacity)
{
    if (*at >= size) return 0;
    size_t available = size - *at;
    const unsigned char *end = memchr(bytes + *at, 0,
                                     available < capacity ? available : capacity);
    if (!end) return available >= capacity ? -1 : 0;
    size_t length = (size_t)(end - bytes) - *at;
    memcpy(out, bytes + *at, length);
    out[length] = 0;
    *at += length + 1;
    return 1;
}
SvLogin *sv_login_create(const int version[6], const unsigned char iaddr[6])
{
    if (!version || version[0] <= 0 || !iaddr) return NULL;
    SvLogin *login = calloc(1, sizeof(*login));
    if (!login) return NULL;
    memcpy(login->version, version, sizeof(login->version));
    sv_report_init(&login->report);
    SvSettingsOptions defaults;
    sv_settings_options_defaults(&defaults);
    memcpy(login->options, defaults.values, sizeof(login->options));
    login->state = SV_LOGIN_WAIT_FLAGS;
    for (size_t i = 0; i < 60; ++i) login->ping.samples[i] = -1;
    unsigned char start[8] = {PKT_LOGIN, 0};
    size_t start_size = 2;
    const int iaddr_since[6] = {4, 9, 2, 1, 0, 2};
    if (atleast(version, iaddr_since)) {
        memcpy(start + 2, iaddr, 6);
        start_size += 6;
    }
    (void)append_output(login, start, start_size);
    return login;
}
static size_t option_count(const SvLogin *login)
{
    const int all_options_since[6] = {4, 9, 1, 0, 0, 0};
    const int options_154_since[6] = {4, 5, 8, 1, 0, 1};
    const int options_128_since[6] = {4, 5, 5, 0, 0, 0};
    return newer(login->version, all_options_since) ? 199u :
        newer(login->version, options_154_since) ? 154u :
        newer(login->version, options_128_since) ? 128u : 96u;
}
SvResult sv_login_set_options(SvLogin *login, const SvSettingsOptions *options)
{
    if (!login || !options || login->state == SV_LOGIN_WAIT_PLAY ||
        login->state == SV_LOGIN_PLAY_ACKNOWLEDGED) return SV_INVALID;
    memcpy(login->options, options->values, sizeof(login->options));
    const int geometry_since[6] = {4, 4, 9, 1, 0, 1};
    if (!newer(login->version, geometry_since)) login->options[7] = 0;
    return SV_OK;
}
SvResult sv_login_send_options(SvLogin *login)
{
    if (!login || login->state != SV_LOGIN_SELECTED) return SV_INVALID;
    unsigned char packet[1 + SV_SETTINGS_OPTION_COUNT] = {PKT_OPTIONS};
    size_t count = option_count(login);
    memcpy(packet + 1, login->options, count);
    return append_output(login, packet, count + 1);
}
void sv_login_destroy(SvLogin *login)
{
    if (!login) return;
    volatile unsigned char *p = (void *)login;
    for (size_t i = 0; i < sizeof(*login); ++i) p[i] = 0;
    free(login);
}
SvOutput sv_login_take_output(SvLogin *login, void *bytes, size_t capacity)
{
    if (!login || !login->output_size) return (SvOutput){SV_WAITING, 0};
    if (capacity < login->output_size) return (SvOutput){SV_OUTPUT_TOO_SMALL, login->output_size};
    if (!bytes) return (SvOutput){SV_INVALID, login->output_size};
    size_t size = login->output_size;
    memcpy(bytes, login->output, size);
    login->output_size = 0;
    return (SvOutput){SV_OK, size};
}
SvLoginState sv_login_state(const SvLogin *login) { return login->state; }
bool sv_login_play_stream_complete(const SvLogin *login)
{
    return login && login->play_stream_complete;
}
SvOutput sv_login_take_remaining(SvLogin *login, void *bytes, size_t capacity)
{
    if (!login || !login->play_stream_complete) return (SvOutput){SV_INVALID, 0};
    if (capacity < login->input_size)
        return (SvOutput){SV_OUTPUT_TOO_SMALL, login->input_size};
    if (login->input_size && !bytes) return (SvOutput){SV_INVALID, login->input_size};
    size_t size = login->input_size;
    if (size) memcpy(bytes, login->input, size);
    login->input_size = 0;
    return (SvOutput){SV_OK, size};
}
size_t sv_login_count(const SvLogin *login) { return login->count; }
const SvLoginCharacter *sv_login_character(const SvLogin *login, size_t slot)
{
    return login && slot < login->count ? &login->characters[slot] : NULL;
}
const SvLoginCharacter *sv_login_selected_character(const SvLogin *login)
{
    if (login && login->creating_character) return &login->creation;
    return login && login->has_selected_slot && login->selected_slot < login->count ?
        &login->characters[login->selected_slot] : NULL;
}
bool sv_login_creating_character(const SvLogin *login)
{
    return login && login->creating_character;
}
bool sv_login_reincarnation_requested(const SvLogin *login)
{
    return login && login->creating_character && login->reincarnation_requested;
}
const uint32_t *sv_login_flags(const SvLogin *login) { return login->flags; }
const int *sv_login_version(const SvLogin *login) { return login->version; }
const char *sv_login_reason(const SvLogin *login) { return login->reason; }
SvResult sv_login_choose(SvLogin *login, size_t slot)
{
    if (!login || login->state != SV_LOGIN_OVERVIEW || slot >= login->count)
        return SV_INVALID;
    size_t length = strlen(login->characters[slot].name);
    unsigned char packet[SV_LOGIN_NAME_CAPACITY + 2] = {PKT_LOGIN};
    memcpy(packet + 1, login->characters[slot].name, length + 1);
    SvResult queued = append_output(login, packet, length + 2);
    if (queued != SV_OK) return queued;
    login->selected_slot = slot;
    login->has_selected_slot = true;
    login->state = SV_LOGIN_WAIT_STATUS;
    return SV_OK;
}
SvResult sv_login_create_character(SvLogin *login, const char *name,
                                   bool reincarnation_requested)
{
    if (!login || login->state != SV_LOGIN_OVERVIEW || !name || !name[0])
        return SV_INVALID;
    size_t length = strlen(name);
    if (length >= SV_LOGIN_NAME_CAPACITY) return SV_INVALID;
    unsigned char packet[SV_LOGIN_NAME_CAPACITY + 2] = {PKT_LOGIN};
    memcpy(packet + 1, name, length + 1);
    SvResult queued = append_output(login, packet, length + 2);
    if (queued != SV_OK) return queued;
    memset(&login->creation, 0, sizeof(login->creation));
    memcpy(login->creation.name, name, length + 1);
    login->creating_character = true;
    login->reincarnation_requested = reincarnation_requested;
    login->has_selected_slot = false;
    login->state = SV_LOGIN_WAIT_STATUS;
    return SV_OK;
}
static void put16(unsigned char **at, int16_t value)
{
    *(*at)++ = (unsigned char)((uint16_t)value >> 8);
    *(*at)++ = (unsigned char)value;
}
SvResult sv_login_set_audio(SvLogin *login, const SvSettingsAudioSnapshot *audio)
{
    return login && sv_report_audio(&login->report, audio) ? SV_OK : SV_INVALID;
}
SvResult sv_login_send_audio(SvLogin *login)
{
    if (!login || login->state != SV_LOGIN_PLAY_ACKNOWLEDGED) return SV_INVALID;
    unsigned char packet[1024];
    size_t size = sv_report_audio_packet(&login->report, login->version, packet);
    return size ? append_output(login, packet, size) : SV_OK;
}
SvResult sv_login_send_font(SvLogin *login)
{
    if (!login || login->state != SV_LOGIN_PLAY_ACKNOWLEDGED) return SV_INVALID;
    unsigned char packet[1024];
    size_t size = sv_report_font(&login->report, login->version, packet);
    return size ? append_output(login, packet, size) : SV_OK;
}
SvResult sv_login_set_visuals(SvLogin *login,const SvVisualDefinitions *visuals,const char *font)
{
    return login ? sv_login_set_rendering(login,visuals,login->report.graphics,
        login->report.tiles,font) : SV_INVALID;
}
SvResult sv_login_update_visuals(SvLogin *login,const SvVisualDefinitions *visuals,const char *font)
{
    return login ? sv_login_update_rendering(login,visuals,login->report.graphics,
        login->report.tiles,font) : SV_INVALID;
}
SvResult sv_login_set_rendering(SvLogin *login,const SvVisualDefinitions *visuals,
    int graphics,const char *tiles,const char *font)
{
    if (!login || !visuals || !font || !tiles || strlen(font)>1023 ||
        strlen(tiles)>1023 || graphics<0 || graphics>2) return SV_INVALID;
    /* Use a candidate because a compatibility wrapper may pass report.tiles. */
    SvResourceReport candidate=login->report;
    candidate.graphics=(int16_t)graphics;
    sv_report_copy(candidate.tiles,tiles);sv_report_copy(candidate.font,font);
    login->visuals=*visuals;login->report=candidate;return SV_OK;
}
SvResult sv_login_update_rendering(SvLogin *login,const SvVisualDefinitions *visuals,
    int graphics,const char *tiles,const char *font)
{
    if (!login || !visuals || !font || !tiles || strlen(font)>1023 ||
        strlen(tiles)>1023 || graphics<0 || graphics>2 ||
        login->state!=SV_LOGIN_PLAY_ACKNOWLEDGED) return SV_INVALID;
    unsigned char packet[20*1024],*at=packet;
    const int setup_since[6]={4,6,1,2,0,1};
    if (sv_visual_atleast(login->version,setup_since)) {
        *at++=PKT_CLIENT_SETUP;
        at+=sv_visual_encode(visuals,login->version,false,at);
    }
    SvResourceReport candidate=login->report;
    candidate.graphics=(int16_t)graphics;
    sv_report_copy(candidate.tiles,tiles);
    sv_report_copy(candidate.font,font);
    at+=sv_report_font(&candidate,login->version,at);
    SvResult result=append_output(login,packet,(size_t)(at-packet));
    if (result==SV_OK) { login->report=candidate;login->visuals=*visuals; }
    return result;
}
SvResult sv_login_start_play(SvLogin *login, int16_t mode, int16_t race,
                              int16_t class_id, int16_t trait,
                              const int16_t stats[6], const char *font)
{
    if (!login || login->state != SV_LOGIN_SELECTED ||
        !stats || !font || strlen(font) > 1023 || race < 0 || class_id < 0 || trait < 0)
        return SV_INVALID;
    /* Baseline Net_start wire order and strict/inclusive version gates. */
    unsigned char packet[32 * 1024], *at = packet;
    *at++ = PKT_PLAY;
    put16(&at, mode); put16(&at, race); put16(&at, class_id);
    const int traits_since[6] = {4, 4, 5, 10, 0, 0};
    const int resources_since[6] = {4, 8, 1, 2, 0, 0};
    if (newer(login->version, traits_since)) {
        put16(&at, trait);
        put16(&at, login->report.sfx); put16(&at, login->report.music);
    }
    if (atleast(login->version, resources_since)) {
        put16(&at, login->report.graphics);
        sv_report_string(&at, login->report.tiles);
        char reported_font[80]; sv_report_copy(reported_font, font);
        sv_report_string(&at, reported_font);
    }
    for (unsigned i = 0; i < 6; ++i) put16(&at, stats[i]);
    size_t count = option_count(login);
    memcpy(at, login->options, count); at += count;
    /* Map viewport, not UI window pixels (baseline SCREEN_WID/SCREEN_HGT). */
    const int geometry_since[6] = {4, 4, 9, 1, 0, 1};
    if (newer(login->version, geometry_since)) {
        /* Saved SV layout controls normal/big viewport independently of OPT
         * file aliases (SCREEN_WID66, SCREEN_HGT22, MAX_SCREEN_HGT44). */
        const unsigned char geometry[] = {0, 0, 0, 66, 0, 0, 0,
            login->options[7] ? 44 : 22};
        memcpy(at, geometry, sizeof(geometry));
        at += sizeof(geometry);
    }
    /* Zero entries retain server identity. Actual PRF definitions override
     * them through the same semantic table used by the font map consumer. */
    at+=sv_visual_encode(&login->visuals,login->version,true,at);
    SvResult queued = append_output(login, packet, (size_t)(at - packet));
    if (queued == SV_OK) {
        login->play_wire=sv_login_startup_prefix_config(login);
        login->play_wire_configured=true;
        sv_report_copy(login->report.font, font);
        login->state = SV_LOGIN_WAIT_PLAY;
    }
    return queued;
}
SvResult sv_login_keepalive(SvLogin *login)
{
    if (!login || login->state == SV_LOGIN_REJECTED) return SV_CLOSED;
    const unsigned char packet = PKT_KEEPALIVE;
    return append_output(login, &packet, 1);
}
SvResult sv_login_receive(SvLogin *login, const void *bytes, size_t size)
{
    if (!login || (size && !bytes) || size > sizeof(login->input) - login->input_size)
        return SV_INPUT_OVERFLOW;
    if (login->state == SV_LOGIN_REJECTED) return SV_INVALID;
    if (login->state == SV_LOGIN_PEER_QUIT) return SV_CLOSED;
    if (size) memcpy(login->input + login->input_size, bytes, size);
    login->input_size += size;
    for (;;) {
        SvResult drained = drain_transfer(login);
        if (drained != SV_OK) return drained;
        /* END belongs to startup; the following bytes belong to the live
         * session, even when TCP coalesces both into the same read. Retain
         * them for its owner instead of parsing them as login packets. */
        if (login->play_stream_complete) return SV_OK;
        if (!login->input_size) return SV_OK;
        if (login->input[0] == PKT_FILE) {
            if (!login->flags_received || !login->transfer.receive) return SV_DECODE_ERROR;
            size_t consumed = 0;
            SvResult received = login->transfer.receive(login->transfer.context,
                login->input, login->input_size, &consumed);
            if (received == SV_WAITING) return SV_OK;
            if (received != SV_OK) return received;
            if (!consumed || consumed > login->input_size) return SV_DECODE_ERROR;
            consume(login, consumed);
            continue;
        }
        /* The server terminates the successful play reply stream with END. */
        if (login->state == SV_LOGIN_PLAY_ACKNOWLEDGED &&
            login->input[0] == PKT_END) {
            consume(login, 1);
            login->play_stream_complete = true;
            return SV_OK;
        }
        if (login->state == SV_LOGIN_PLAY_ACKNOWLEDGED && login->input[0] == PKT_VERSION) {
            unsigned char packet[1024];
            size_t count = sv_report_version(&login->report, login->version, login->ping.samples, packet);
            SvResult queued = count ? append_output(login, packet, count) : SV_OK;
            if (queued != SV_OK) return queued;
            consume(login, 1);
            continue;
        }
        if (login->input[0] == PKT_KEEPALIVE) {
            consume(login, 1);
            continue;
        }
        if (login->input[0] == PKT_PING) {
            if (login->input_size < 15) return SV_OK;
            size_t available = login->input_size - 14;
            if (available > 1024) available = 1024;
            const unsigned char *end = memchr(login->input + 14, 0, available);
            if (!end) return available == 1024 ? SV_DECODE_ERROR : SV_OK;
            size_t packet_size = (size_t)(end - login->input) + 1;
            if (!login->input[1]) {
                unsigned char reply[1024];
                if (packet_size > sizeof(reply)) return SV_DECODE_ERROR;
                memcpy(reply, login->input, packet_size);
                reply[1] = 1;
                SvResult queued = append_output(login, reply, packet_size);
                if (queued != SV_OK) return queued;
            } else receive_pong(login, login->input);
            consume(login, packet_size);
            continue;
        }
        if (login->input[0] == PKT_QUIT) {
            size_t at = 1;
            int parsed = string(login->input, login->input_size, &at,
                                login->reason, sizeof(login->reason));
            if (parsed < 0) return SV_DECODE_ERROR;
            if (!parsed) return SV_OK;
            consume(login, at);
            login->state = (login->state == SV_LOGIN_WAIT_PLAY ||
                            login->state == SV_LOGIN_PLAY_ACKNOWLEDGED) ?
                           SV_LOGIN_PEER_QUIT : SV_LOGIN_REJECTED;
            return SV_OK;
        }
        if (login->state == SV_LOGIN_WAIT_PLAY && login->input[0] == PKT_REPLY) {
            if (login->input_size < 3) return SV_OK;
            if (login->input[1] != PKT_PLAY) {
                snprintf(login->reason, sizeof(login->reason),
                         "Can't receive reply packet after play");
                return SV_DECODE_ERROR;
            }
            unsigned status = login->input[2];
            consume(login, 3);
            if (status != PKT_SUCCESS) {
                snprintf(login->reason, sizeof(login->reason),
                         "Start play not allowed (%u)", status);
                login->state = SV_LOGIN_REJECTED;
                return SV_OK;
            }
            login->state = SV_LOGIN_PLAY_ACKNOWLEDGED;
            continue;
        }
        if ((login->state == SV_LOGIN_OVERVIEW || login->state == SV_LOGIN_SELECTED ||
             login->state == SV_LOGIN_WAIT_PLAY ||
             login->state == SV_LOGIN_PLAY_ACKNOWLEDGED) &&
            login->input[0] == PKT_SFLAGS) {
            if (login->input_size < 17) return SV_OK;
            for (int i = 0; i < 4; ++i)
                login->flags[3 - i] = get32(login->input + 1 + 4 * i);
            consume(login, 17);
        } else if (login->state == SV_LOGIN_WAIT_FLAGS) {
            if (login->input[0] != PKT_SERVERDETAILS) return SV_DECODE_ERROR;
            if (login->input_size < 17) return SV_OK;
            for (int i = 0; i < 4; ++i) login->flags[i] = get32(login->input + 1 + 4 * i);
            consume(login, 17);
            login->flags_received = true;
            if (login->transfer.flags) {
                SvResult flags = login->transfer.flags(login->transfer.context, login->flags);
                if (flags != SV_OK) return flags;
            }
            login->state = SV_LOGIN_WAIT_LIST;
        } else if (login->state == SV_LOGIN_WAIT_LIST) {
            if (login->input[0] != PKT_LOGIN) return SV_DECODE_ERROR;
            size_t at = 1;
            const int mode_since[6] = {4, 4, 9, 2, 0, 0};
            const int location_since[6] = {4, 5, 7, 0, 0, 0};
            SvLoginCharacter row = {0};
            if (newer(login->version, mode_since)) {
                if (login->input_size - at < 2) return SV_OK;
                row.mode = get16(login->input + at); at += 2;
            }
            int parsed = string(login->input, login->input_size, &at,
                                row.colour, sizeof(row.colour));
            if (parsed < 0) return SV_DECODE_ERROR;
            if (!parsed) return SV_OK;
            parsed = string(login->input, login->input_size, &at,
                            row.name, sizeof(row.name));
            if (parsed < 0) return SV_DECODE_ERROR;
            if (!parsed) return SV_OK;
            if (login->input_size - at < 6) return SV_OK;
            row.level = get16(login->input + at); at += 2;
            row.race = get16(login->input + at); at += 2;
            row.class_id = get16(login->input + at); at += 2;
            if (newer(login->version, location_since)) {
                parsed = string(login->input, login->input_size, &at,
                                row.location, sizeof(row.location));
                if (parsed < 0) return SV_DECODE_ERROR;
                if (!parsed) return SV_OK;
            }
            consume(login, at);
            if (!row.name[0]) {
                login->state = SV_LOGIN_OVERVIEW;
                continue;
            }
            if (login->count >= SV_LOGIN_MAX_CHARACTERS) return SV_INPUT_OVERFLOW;
            login->characters[login->count++] = row;
        } else if (login->state == SV_LOGIN_WAIT_STATUS) {
            unsigned char status = login->input[0];
            consume(login, 1);
            /* Receive_login sends E_NEED_INFO for an accepted new name;
             * SUCCESS is the existing-character branch, not the only success. */
            if (status != 0 && status != E_NEED_INFO) {
                snprintf(login->reason, sizeof(login->reason),
                         "Server rejected character selection (status %u).", status);
                login->state = SV_LOGIN_REJECTED;
                return SV_OK;
            }
            if (status == E_NEED_INFO && !login->creating_character) {
                /* A listed character can disappear before selection. Follow
                 * the server's birth request while retaining its chosen name. */
                const SvLoginCharacter *selected = sv_login_selected_character(login);
                if (!selected) return SV_DECODE_ERROR;
                login->creation = *selected;
                login->creating_character = true;
                login->has_selected_slot = false;
            }
            login->state = SV_LOGIN_SELECTED;
            continue;
        } else {
            if (login->state == SV_LOGIN_WAIT_PLAY)
                snprintf(login->reason, sizeof(login->reason),
                         "Not a reply packet after play (%u)", login->input[0]);
            return SV_DECODE_ERROR;
        }
    }
}

SvStartupPrefixConfig sv_login_startup_prefix_config(const SvLogin *login)
{
    SvStartupPrefixConfig config={0};
    if(!login)return config;
    if(login->play_wire_configured)return login->play_wire;
    memcpy(config.version,login->version,sizeof(config.version));
    config.graphics=(unsigned)login->report.graphics;
    const int wide[6]={4,8,1,0,0,0}, expanded[6]={4,6,1,2,0,1};
    const int feature_since[6]={4,9,1,2,0,3};
    size_t counts[4]={127,sv_visual_atleast(login->version,feature_since)?512:256,
        sv_visual_atleast(login->version,expanded)?1280:1024,
        sv_visual_atleast(login->version,expanded)?1280:1152};
    const SvVisualDefinition *tables[4]={login->visuals.unknown,login->visuals.features,
        login->visuals.kinds,login->visuals.monsters};
    uint32_t maximum=0;
    bool wide_glyphs=sv_visual_atleast(login->version,wide);
    for(unsigned table=0;table<4;++table)for(size_t i=0;i<counts[table];++i) {
        uint32_t glyph=tables[table][i].character;
        if(!wide_glyphs)glyph&=0xff;
        if(glyph>maximum)maximum=glyph;
    }
    for(;maximum;maximum>>=8)++config.char_transfer_bytes;
    return config;
}

size_t sv_login_output_capacity(const SvLogin *login)
{ return login?sizeof(login->output)-login->output_size:0; }
SvResult sv_login_queue_prepared_output(SvLogin *login,const void *bytes,size_t size)
{
    if(!login || !login->play_stream_complete || (size&&!bytes))return SV_INVALID;
    return size?append_output(login,bytes,size):SV_OK;
}
