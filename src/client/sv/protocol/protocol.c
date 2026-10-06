#include <sys/time.h>
#include <limits.h>
#ifdef WIN32
#include <winsock.h>
#include <windows.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../../common/angband.h"
#include "../../../common/sockbuf.h"
#include "../../../common/pack.h"
#include "protocol/hp-update.h"
#include "protocol/message-update.h"
#include "protocol/key-request.h"
#include "protocol/protocol.h"
static void queue_bytes(SvProtocol *protocol, const unsigned char *bytes, size_t size);
struct SvProtocol {
    version_type version;
    sockbuf_t input, output;
    int previous_type;
    uint32_t ping_id;
    SvPingClock clock;
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
    return SV_OK;
}
SvProtocol *sv_protocol_create(const int version[6])
{
    SvProtocol *p;
    if (!version || version[0] <= 0) return NULL;
    p = calloc(1, sizeof(*p));
    if (!p) return NULL;
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
SvResult sv_protocol_next(SvProtocol *p, SvChange *change)
{
    sockbuf_t *in = &p->input;
    if (!in->len) return SV_WAITING;
    unsigned char type = (unsigned char)*in->ptr;
    switch (type) {
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
