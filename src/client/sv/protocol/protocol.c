#include <sys/time.h>
#include <limits.h>
#ifdef WIN32
#include <winsock.h>
#include <windows.h>
#undef GF_INERTIA
#endif
#define CLIENT
#define CLIENT_SIDE
#include "../../../common/angband.h"
#include "../../../common/sockbuf.h"
#include "../../../common/pack.h"
#include "protocol/hp-update.h"
#include "protocol/message-update.h"
#include "protocol/key-request.h"
#include "protocol/protocol.h"
#include "protocol/resource-report.h"
#include "protocol/character-identity.h"
#include "protocol/status-update.h"
static int identity_compare(const int version[6], const int threshold[6])
{
    for (unsigned i = 0; i < 6; ++i)
        if (version[i] != threshold[i]) return version[i] > threshold[i] ? 1 : -1;
    return 0;
}
static uint16_t identity_get16(const unsigned char *p)
{ return (uint16_t)((uint16_t)p[0] << 8 | p[1]); }
SvResult sv_character_identity_decode(const int version[6], const void *bytes,
    size_t available, SvCharacterIdentity *out, size_t *consumed)
{
    if (!consumed) return SV_INVALID;
    *consumed = 0;
    if (!version || !out || (available && !bytes)) return SV_INVALID;
    if (!available) return SV_WAITING;
    const unsigned char *p = bytes;
    if (p[0] != PKT_CHAR_INFO) return SV_DECODE_ERROR;
    const int modern_since[6] = {4,9,2,1,0,1}, lives_since[6] = {4,7,3,0,0,0};
    const int name_after[6] = {4,5,2,0,0,0}, trait_after[6] = {4,4,5,10,0,0};
    bool modern = identity_compare(version, modern_since) >= 0;
    SvCharacterIdentity next = {.available = true, .lives = -1,
        .trait_available = identity_compare(version, trait_after) > 0,
        .lives_available = identity_compare(version, lives_since) >= 0,
        .name_available = identity_compare(version, name_after) > 0};
    size_t fixed = 1 + 2 * (next.trait_available ? 5u : 4u) +
        (modern ? 2u : 0u) + (next.lives_available ? 2u : 0u);
    if (available < fixed) return SV_WAITING;
    size_t at = 1;
    next.race = (int16_t)identity_get16(p + at); at += 2;
    next.class_id = (int16_t)identity_get16(p + at); at += 2;
    if (next.trait_available) { next.trait = (int16_t)identity_get16(p + at); at += 2; }
    next.sex = (int16_t)identity_get16(p + at); at += 2;
    next.wire_mode = identity_get16(p + at); at += 2;
    if (modern) { next.wire_mode = next.wire_mode << 16 | identity_get16(p + at); at += 2; }
    if (next.lives_available) { next.lives = (int16_t)identity_get16(p + at); at += 2; }
    if (next.name_available) {
        size_t count = available - at;
        if (count > sizeof(next.name)) count = sizeof(next.name);
        const unsigned char *end = memchr(p + at, 0, count);
        if (!end) return count == sizeof(next.name) ? SV_DECODE_ERROR : SV_WAITING;
        size_t length = (size_t)(end - (p + at));
        memcpy(next.name, p + at, length);
        at += length + 1;
    }
    uint32_t mode = next.wire_mode;
    next.mode = modern ? mode : mode & 0xff;
    next.admin_wiz = (mode & (modern ? MODE_ADMIN_WIZ : 0x0400)) != 0;
    next.admin_dm = (mode & (modern ? MODE_ADMIN_DM : 0x0800)) != 0;
    next.privileged = (mode & (modern ? MODE_VPRIVILEGED : 0x2000)) ? 2 :
        (mode & (modern ? MODE_PRIVILEGED : 0x1000)) ? 1 : 0;
    next.restricted = (mode & (modern ? MODE_VRESTRICTED : 0x8000)) ? 2 :
        (mode & (modern ? MODE_RESTRICTED : 0x4000)) ? 1 : 0;
    next.fruit_bat = (mode & MODE_FRUIT_BAT) != 0;
    if (!sv_character_identity_validate(&next, NULL)) return SV_DECODE_ERROR;
    *out = next;
    *consumed = at;
    return SV_OK;
}

static void queue_bytes(SvProtocol *protocol, const unsigned char *bytes, size_t size);
struct SvProtocol {
    version_type version;
    sockbuf_t input, output;
    int previous_type;
    uint32_t ping_id;
    SvPingClock clock;
    SvResourceReport report;
    int report_version[6], ping_samples[60];
};
static void ping_now(SvProtocol *protocol, int32_t *seconds, int32_t *microseconds)
{
    if (protocol->clock.now) {
        protocol->clock.now(protocol->clock.context, seconds, microseconds);
        return;
    }
#ifdef WIN32
    DWORD ticks = GetTickCount();
    *seconds = (int32_t)(ticks / 1000);
    *microseconds = (int32_t)((ticks % 1000) * 1000);
#else
    struct timeval time;
    gettimeofday(&time, NULL);
    *seconds = (int32_t)time.tv_sec;
    *microseconds = (int32_t)time.tv_usec;
#endif
}
SvResult sv_protocol_set_ping_clock(SvProtocol *protocol, SvPingClock clock)
{
    if (!clock.now) return SV_INVALID;
    protocol->clock = clock;
    return SV_OK;
}
SvResult sv_protocol_send_ping(SvProtocol *protocol)
{
    if (sv_protocol_output_capacity(protocol) < 15) return SV_BACKPRESSURE;
    int32_t seconds, microseconds;
    ping_now(protocol, &seconds, &microseconds);
    uint32_t id = protocol->ping_id + 1;
    unsigned char bytes[15] = {PKT_PING, 0};
    const uint32_t values[3] = {id, (uint32_t)seconds, (uint32_t)microseconds};
    for (int i = 0; i < 3; ++i) {
        bytes[2 + 4*i] = (unsigned char)(values[i] >> 24);
        bytes[3 + 4*i] = (unsigned char)(values[i] >> 16);
        bytes[4 + 4*i] = (unsigned char)(values[i] >> 8);
        bytes[5 + 4*i] = (unsigned char)values[i];
    }
    queue_bytes(protocol, bytes, sizeof(bytes));
    protocol->ping_id = id;
    memmove(protocol->ping_samples + 1, protocol->ping_samples, 59 * sizeof(int));
    protocol->ping_samples[0] = -1;
    return SV_OK;
}
SvProtocol *sv_protocol_create(const int version[6])
{
    SvProtocol *p;
    if (!version || version[0] <= 0) return NULL;
    p = calloc(1, sizeof(*p));
    if (!p) return NULL;
    sv_report_init(&p->report);
    memcpy(p->report_version, version, sizeof(p->report_version));
    for (unsigned i = 0; i < 60; ++i) p->ping_samples[i] = -1;
    p->version.major = version[0]; p->version.minor = version[1];
    p->version.patch = version[2]; p->version.extra = version[3];
    p->version.branch = version[4]; p->version.build = version[5];
    if (Sockbuf_init(&p->input, -1, SV_PROTOCOL_CAPACITY, SOCKBUF_READ | SOCKBUF_LOCK) < 0 ||
        Sockbuf_init(&p->output, -1, SV_PROTOCOL_CAPACITY, SOCKBUF_WRITE | SOCKBUF_LOCK) < 0) {
        sv_protocol_destroy(p); return NULL;
    }
    return p;
}
void sv_protocol_destroy(SvProtocol *p)
{
    if (!p) return;
    Sockbuf_cleanup(&p->input); Sockbuf_cleanup(&p->output); free(p);
}
size_t sv_protocol_pending(const SvProtocol *p) { return (size_t)p->input.len; }
size_t sv_protocol_capacity(const SvProtocol *p) { return (size_t)(p->input.size - p->input.len); }
size_t sv_protocol_output_capacity(const SvProtocol *p) { return (size_t)(p->output.size - p->output.len); }
static void queue_bytes(SvProtocol *p, const unsigned char *bytes, size_t size)
{
    memcpy(p->output.buf + p->output.len, bytes, size);
    p->output.len += (int)size;
}
SvResult sv_protocol_receive(SvProtocol *p, const void *bytes, size_t size)
{
    if (size && !bytes) return SV_INVALID;
    if (size > SV_PROTOCOL_CAPACITY) return SV_INPUT_OVERFLOW;
    if (size > sv_protocol_capacity(p)) return SV_BACKPRESSURE;
    if (size) memcpy(p->input.buf + p->input.len, bytes, size);
    p->input.len += size;
    return SV_OK;
}
bool sv_protocol_supports_packet(unsigned char type)
{
    if (sv_status_packet_supported(type)) return true;
    switch (type) {
    case PKT_VERSION: case PKT_HP: case PKT_MESSAGE: case PKT_REQUEST_KEY:
    case PKT_REQUEST_ABORT: case PKT_KEEPALIVE: case PKT_END: case PKT_KEYPRESS:
    case PKT_PAUSE: case PKT_FLUSH: case PKT_CONFIRM: case PKT_SFLAGS:
    case PKT_PING: case PKT_CHAR_INFO: return true;
    default: return false;
    }
}
SvResult sv_protocol_redraw(SvProtocol *p)
{
    if (!p) return SV_INVALID;
    if (sv_protocol_output_capacity(p) < 2) return SV_BACKPRESSURE;
    const unsigned char bytes[2] = {PKT_REDRAW, 0};
    queue_bytes(p, bytes, sizeof(bytes));
    return SV_OK;
}
SvResult sv_protocol_next(SvProtocol *p, SvChange *change)
{
    sockbuf_t *in = &p->input;
    if (!in->len) return SV_WAITING;
    unsigned char type = (unsigned char)*in->ptr;
    if (sv_status_packet_supported(type)) {
        SvHudUpdate update;
        size_t consumed = 0;
        SvResult result = sv_status_decode(p->report_version, in->ptr,
            (size_t)(in->len - (in->ptr - in->buf)), &update, &consumed);
        if (result != SV_OK) return result;
        change->kind = SV_CHANGE_HUD;
        change->hud = update;
        p->previous_type = type;
        Sockbuf_advance(in, (int)consumed);
        return SV_OK;
    }
    switch (type) {
    case PKT_CHAR_INFO: {
        size_t consumed = 0;
        SvCharacterIdentity identity;
        SvResult result = sv_character_identity_decode(p->report_version, in->ptr,
            (size_t)(in->len - (in->ptr - in->buf)), &identity, &consumed);
        if (result != SV_OK) return result;
        change->kind = SV_CHANGE_IDENTITY;
        change->identity = identity;
        p->previous_type = type;
        Sockbuf_advance(in, (int)consumed);
        return SV_OK;
    }
    case PKT_VERSION: {
        unsigned char packet[1024];
        size_t size = sv_report_version(&p->report, p->report_version, p->ping_samples, packet);
        if (sv_protocol_output_capacity(p) < size) return SV_BACKPRESSURE;
        if (size) queue_bytes(p, packet, size);
        change->kind = SV_CHANGE_NOOP;
        p->previous_type = type;
        Sockbuf_advance(in, 1);
        return SV_OK;
    }
    case PKT_HP: {
        SvDecodedHp hp;
        int decoded = sv_decode_hp(in, &p->version, &hp);
        if (decoded < 0) return SV_DECODE_ERROR;
        if (!decoded) return SV_WAITING;
        change->kind = SV_CHANGE_HP;
        change->hp = (SvHpUpdate){hp.maximum, hp.current, hp.drain, hp.bar, hp.boosted};
        p->previous_type = type;
        Sockbuf_advance(in, (int)(in->ptr - in->buf));
        return SV_OK;
    }
    case PKT_MESSAGE: {
        char raw[MSG_LEN];
        int decoded = sv_decode_message(in, raw);
        if (decoded < 0) return SV_DECODE_ERROR;
        if (!decoded) return SV_WAITING;
        change->kind = SV_CHANGE_MESSAGE;
        change->message = (SvMessage){0};
        change->message.length = strlen(raw);
        memcpy(change->message.bytes, raw, MSG_LEN);
        p->previous_type = type;
        Sockbuf_advance(in, (int)(in->ptr - in->buf));
        return SV_OK;
    }
    case PKT_REQUEST_KEY: {
        SvKeyRequest request = {0};
        int decoded = sv_decode_key_request(in, &request.id, (char *)request.prompt);
        if (decoded < 0) return SV_DECODE_ERROR;
        if (!decoded) return SV_WAITING;
        change->kind = SV_CHANGE_KEY_REQUEST;
        change->request = request;
        p->previous_type = type;
        Sockbuf_advance(in, (int)(in->ptr - in->buf));
        return SV_OK;
    }
    case PKT_REQUEST_ABORT:
        change->kind = SV_CHANGE_REQUEST_ABORT;
        p->previous_type = type;
        Sockbuf_advance(in, 1);
        return SV_OK;
    case PKT_KEEPALIVE:
    case PKT_END:
    case PKT_KEYPRESS:
        change->kind = SV_CHANGE_NOOP;
        p->previous_type = type;
        Sockbuf_advance(in, 1);
        return SV_OK;
    case PKT_PAUSE:
    case PKT_FLUSH:
        change->kind = type == PKT_PAUSE ? SV_CHANGE_PAUSE : SV_CHANGE_FLUSH;
        p->previous_type = type;
        Sockbuf_advance(in, 1);
        return SV_OK;
    case PKT_CONFIRM:
        if (in->len < 2) return SV_WAITING;
        change->kind = SV_CHANGE_CONFIRM;
        change->confirmed_command = (unsigned char)in->ptr[1];
        p->previous_type = type;
        Sockbuf_advance(in, 2);
        return SV_OK;
    case PKT_SFLAGS: {
        if (in->len < 17) return SV_WAITING;
        uint32_t words[4];
        for (int i = 0; i < 4; ++i) {
            const unsigned char *field = (const unsigned char *)in->ptr + 1 + 4 * i;
            words[i] = ((uint32_t)field[0] << 24) | ((uint32_t)field[1] << 16) |
                       ((uint32_t)field[2] << 8) | field[3];
        }
        change->kind = SV_CHANGE_SERVER_FLAGS;
        memcpy(change->server_flags, words, sizeof(words));
        p->previous_type = type;
        Sockbuf_advance(in, 17);
        return SV_OK;
    }
    case PKT_PING: {
        size_t available = (size_t)(in->len - (in->ptr - in->buf));
        if (available < 15) return SV_WAITING;
        size_t body = available - 14;
        if (body > MSG_LEN) body = MSG_LEN;
        const char *end = memchr(in->ptr + 14, 0, body);
        if (!end) return body == MSG_LEN ? SV_DECODE_ERROR : SV_WAITING;
        size_t size = (size_t)(end - in->ptr) + 1;
        if (!in->ptr[1]) {
            if (sv_protocol_output_capacity(p) < size) return SV_BACKPRESSURE;
            char *reply = p->output.buf + p->output.len;
            memcpy(reply, in->ptr, size);
            reply[1] = 1;
            p->output.len += (int)size;
            change->kind = SV_CHANGE_NOOP;
        } else {
            const unsigned char *fields = (const unsigned char *)in->ptr + 2;
            uint32_t id = ((uint32_t)fields[0] << 24) | ((uint32_t)fields[1] << 16) |
                          ((uint32_t)fields[2] << 8) | fields[3];
            uint32_t index = p->ping_id - id;
            int32_t seconds, microseconds;
            ping_now(p, &seconds, &microseconds);
            int32_t sent_seconds = (int32_t)(((uint32_t)fields[4] << 24) |
                ((uint32_t)fields[5] << 16) | ((uint32_t)fields[6] << 8) | fields[7]);
            int32_t sent_microseconds = (int32_t)(((uint32_t)fields[8] << 24) |
                ((uint32_t)fields[9] << 16) | ((uint32_t)fields[10] << 8) | fields[11]);
            int64_t elapsed = ((int64_t)seconds - sent_seconds) * 1000 +
                              ((int64_t)microseconds - sent_microseconds) / 1000;
            if (!elapsed) elapsed = 1;
            if (elapsed > INT_MAX) elapsed = INT_MAX;
            if (elapsed < INT_MIN) elapsed = INT_MIN;
            if (id && id <= p->ping_id && index < 60) p->ping_samples[index] = (int)elapsed;
            change->kind = SV_CHANGE_PING;
            change->ping = (SvPingUpdate){(int32_t)elapsed,
                id && id <= p->ping_id && index < 60 ? (unsigned char)index : 255};
        }
        p->previous_type = type;
        Sockbuf_advance(in, (int)size);
        return SV_OK;
    }
    default:
        /* Unknown + redraw is one recovery. Never partially enqueue it. */
        if (sv_protocol_output_capacity(p) < 11) return SV_BACKPRESSURE;
        const unsigned char reply[11] = {PKT_UNKNOWNPACKET,
            0, 0, 0, type,
            0, 0, 0, (unsigned char)p->previous_type,
            PKT_REDRAW, 0};
        /* Baseline recovery also reports the dropped type to the player.
         * Publish it with the successful recovery, never on a capacity retry. */
        change->kind = SV_CHANGE_MESSAGE;
        change->message = (SvMessage){0};
        change->message.length = (size_t)snprintf((char *)change->message.bytes,
            sizeof(change->message.bytes),
            "\377RReceived unknown packet type (%d, %d), dropping",
            type, p->previous_type);
        queue_bytes(p, reply, sizeof(reply));
        Sockbuf_clear(in); /* Baseline recovery discards this input batch. */
        return SV_RECOVERED;
    }
}
SvOutput sv_protocol_output(SvProtocol *p, void *bytes, size_t capacity)
{
    size_t count = (size_t)p->output.len;
    if (!count) return (SvOutput){SV_WAITING, 0};
    if (capacity < count) return (SvOutput){SV_OUTPUT_TOO_SMALL, count};
    if (!bytes) return (SvOutput){SV_INVALID, count};
    memcpy(bytes, p->output.buf, count);
    Sockbuf_clear(&p->output);
    return (SvOutput){SV_OK, count};
}

SvResult sv_protocol_key_reply(SvProtocol *p, int id, unsigned char key)
{
    if (sv_protocol_output_capacity(p) < 6) return SV_BACKPRESSURE;
    uint32_t number = (uint32_t)id;
    const unsigned char packet[6] = {PKT_REQUEST_KEY, (unsigned char)(number >> 24),
        (unsigned char)(number >> 16), (unsigned char)(number >> 8), (unsigned char)number, key};
    queue_bytes(p, packet, sizeof(packet));
    return SV_OK;
}

SvResult sv_protocol_raw_key(SvProtocol *p, unsigned char key)
{
    if (sv_protocol_output_capacity(p) < 2) return SV_BACKPRESSURE;
    const unsigned char packet[2] = {PKT_RAW_KEY, key};
    queue_bytes(p, packet, sizeof(packet));
    return SV_OK;
}
SvResult sv_protocol_keepalive(SvProtocol *p)
{
    if (sv_protocol_output_capacity(p) < 1) return SV_BACKPRESSURE;
    const unsigned char packet = PKT_KEEPALIVE;
    queue_bytes(p, &packet, 1);
    return SV_OK;
}

SvResult sv_protocol_set_reporting_audio(SvProtocol *p, const SvSettingsAudioSnapshot *audio)
{
    return p && sv_report_audio(&p->report, audio) ? SV_OK : SV_INVALID;
}
SvResult sv_protocol_send_font(SvProtocol *p)
{
    if (!p) return SV_INVALID;
    unsigned char packet[1024];
    size_t size = sv_report_font(&p->report, p->report_version, packet);
    if (sv_protocol_output_capacity(p) < size) return SV_BACKPRESSURE;
    if (size) queue_bytes(p, packet, size);
    return SV_OK;
}
