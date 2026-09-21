#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../common/angband.h"
#include "../../common/sockbuf.h"
#include "../../common/pack.h"
#include "../hp-update.h"
#include "../message-update.h"
#include "protocol.h"
struct SvProtocol {
    version_type version;
    sockbuf_t input, output;
    int previous_type;
};
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
        ClientHpUpdate hp;
        int decoded = client_decode_hp(in, &p->version, &hp);
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
        int decoded = client_decode_message(in, raw);
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
    default:
        /* Unknown + redraw is one recovery. Never partially enqueue it. */
        if (p->output.size - p->output.len < 11) return SV_OUTPUT_OVERFLOW;
        if (Packet_printf(&p->output, "%c%d%d", PKT_UNKNOWNPACKET, (int)type,
                          p->previous_type) <= 0 ||
            Packet_printf(&p->output, "%c%c", PKT_REDRAW, 0) <= 0) return SV_OUTPUT_OVERFLOW;
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
