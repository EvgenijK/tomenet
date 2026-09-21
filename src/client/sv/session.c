#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../common/angband.h"
#include "../../common/sockbuf.h"
#include "../hp-update.h"
#include "session.h"
struct SvSession {
    SvKeyRequest request;
    uint64_t request_sequence;
    SvMessage pending[SV_MESSAGE_PENDING];
    size_t head, count;
    uint64_t sequence;
    SvMessages messages;
    player_type player; /* Sole HP value storage. */
    unsigned char drain;
    bool bar, boosted;
    uint64_t revision;
};
SvSession *sv_session_create(void) { return calloc(1, sizeof(SvSession)); }
void sv_session_destroy(SvSession *s) { free(s); }
SvStatus sv_session_status(const SvSession *s)
{
    SvStatus view = {s->player.mhp, s->player.chp, s->drain,
                    s->bar, s->boosted, s->revision != 0, s->revision};
    return view;
}
static SvStatusChange apply_hp(SvSession *s, SvHpUpdate update)
{
    SvStatusChange change;
    ClientHpUpdate hp = {update.maximum, update.current, update.bar,
                         update.boosted, update.drain};
    change.before = sv_session_status(s);
    client_apply_hp(&s->player, &hp);
    s->bar = hp.bar; s->boosted = hp.boosted; s->drain = hp.drain;
    ++s->revision;
    change.after = sv_session_status(s);
    return change;
}

static SvResult apply_message(SvSession *s, const SvMessage *message)
{
    if (!message || message->length >= SV_MESSAGE_BYTES || message->bytes[message->length] ||
        memchr(message->bytes, 0, message->length)) return SV_INVALID;
    if (s->count == SV_MESSAGE_PENDING || s->sequence == UINT64_MAX) return SV_EVENT_OVERFLOW;
    SvMessage next = *message;
    next.sequence = ++s->sequence;
    s->pending[(s->head + s->count) % SV_MESSAGE_PENDING] = next;
    ++s->count;
    if (next.length == 1 && next.bytes[0] == 255) {
        s->messages = (SvMessages){.revision = next.sequence}; /* Clear-topline sentinel. */
        return SV_OK;
    }
    s->messages.revision = next.sequence;
    /* The current feed is state, independent of pending one-time delivery. */
    if (s->messages.count == SV_MESSAGE_LINES) {
        memmove(s->messages.lines, s->messages.lines + 1,
                (SV_MESSAGE_LINES - 1) * sizeof(SvMessage));
        --s->messages.count;
    }
    s->messages.lines[s->messages.count++] = next;
    return SV_OK;
}
SvResult sv_session_take_message(SvSession *s, SvMessage *message)
{
    if (!message) return SV_INVALID;
    if (!s->count) return SV_WAITING;
    *message = s->pending[s->head];
    memset(&s->pending[s->head], 0, sizeof(SvMessage));
    s->head = (s->head + 1) % SV_MESSAGE_PENDING;
    --s->count;
    return SV_OK;
}
SvMessages sv_session_messages(const SvSession *s) { return s->messages; }

SvSessionChange sv_session_apply(SvSession *s, const SvChange *change)
{
    SvStatus status = sv_session_status(s);
    SvSessionChange applied = {SV_OK, {status, status}};
    switch (change->kind) {
    case SV_CHANGE_HP: applied.status = apply_hp(s, change->hp); break;
    case SV_CHANGE_MESSAGE: applied.result = apply_message(s, &change->message); break;
    case SV_CHANGE_KEY_REQUEST:
        if (s->request.pending || s->request_sequence == UINT64_MAX) {
            applied.result = SV_EVENT_OVERFLOW; break;
        }
        s->request = change->request;
        s->request.sequence = ++s->request_sequence;
        s->request.pending = 1; s->request.aborted = 0;
        break;
    case SV_CHANGE_REQUEST_ABORT:
        if (s->request.pending) s->request.aborted = 1;
        break;
    default: applied.result = SV_INVALID; break;
    }
    return applied;
}

SvKeyRequest sv_session_request(const SvSession *s) { return s->request; }
SvResult sv_session_complete_request(SvSession *s, uint64_t sequence)
{
    if (!s->request.pending || s->request.sequence != sequence) return SV_STALE;
    s->request = (SvKeyRequest){0};
    return SV_OK;
}
