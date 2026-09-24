#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../../common/angband.h"
#include "../../../common/sockbuf.h"
#include "protocol/hp-update.h"
#include "session/session.h"
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
    SvControlState controls;
    unsigned char confirmations[32];
    size_t confirmation_head, confirmation_count;
    SvPingTelemetry ping;
};
SvSession *sv_session_create(void)
{
    SvSession *session = calloc(1, sizeof(SvSession));
    if (session) for (size_t i = 0; i < 60; ++i) session->ping.samples[i] = -1;
    return session;
}
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
    SvDecodedHp hp = {update.maximum, update.current, update.bar,
                      update.boosted, update.drain};
    change.before = sv_session_status(s);
    sv_apply_hp(&s->player, &hp);
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
    SvSessionChange applied = {.result = SV_OK, .status = {status, status}};
    switch (change->kind) {
    case SV_CHANGE_HP: applied.status = apply_hp(s, change->hp); break;
    case SV_CHANGE_MESSAGE:
        applied.result = apply_message(s, &change->message);
        applied.message_occurrence = s->messages.count ? s->messages.revision : 0;
        /* Baseline c_message_add: leading 253 routes explicit chat. */
        applied.message_chat = change->message.length && change->message.bytes[0] == 253;
        break;
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
    case SV_CHANGE_NOOP:
    case SV_CHANGE_PAUSE:
    case SV_CHANGE_FLUSH: break;
    case SV_CHANGE_SERVER_FLAGS:
        memcpy(s->controls.server_flags, change->server_flags, sizeof(change->server_flags));
        break;
    case SV_CHANGE_CONFIRM:
        if (s->confirmation_count == sizeof(s->confirmations)) {
            applied.result = SV_EVENT_OVERFLOW; break;
        }
        s->confirmations[(s->confirmation_head + s->confirmation_count) % sizeof(s->confirmations)] =
            change->confirmed_command;
        ++s->confirmation_count;
        break;
    case SV_CHANGE_PING:
        if (change->ping.index < 60) {
            int rtt = change->ping.rtt_ms;
            SvPingTelemetry *telemetry = &s->ping;
            telemetry->samples[change->ping.index] = rtt;
            telemetry->latest_ms = rtt;
            if ((int64_t)rtt < (int64_t)telemetry->average_ms * 7 / 10) {
                telemetry->average_ms = rtt;
                telemetry->average_count = 1;
            }
            if ((int64_t)rtt <= (int64_t)telemetry->average_ms * 13 / 10 ||
                telemetry->average_count < 10) {
                if (telemetry->average_count == 600)
                    telemetry->average_ms = (int)(((int64_t)telemetry->average_ms * 599 + rtt) / 600);
                else {
                    telemetry->average_ms =
                        (int)(((int64_t)telemetry->average_ms * telemetry->average_count + rtt) /
                        (telemetry->average_count + 1));
                    ++telemetry->average_count;
                }
            }
        }
        break;
    default: applied.result = SV_INVALID; break;
    }
    return applied;
}

SvKeyRequest sv_session_request(const SvSession *s) { return s->request; }
SvControlState sv_session_controls(const SvSession *s) { return s->controls; }
void sv_session_ping_sent(SvSession *s)
{
    memmove(s->ping.samples + 1, s->ping.samples, 59 * sizeof(int));
    s->ping.samples[0] = -1;
}
SvPingTelemetry sv_session_ping(const SvSession *s) { return s->ping; }
SvResult sv_session_take_confirmation(SvSession *s, unsigned char *command)
{
    if (!command) return SV_INVALID;
    if (!s->confirmation_count) return SV_WAITING;
    *command = s->confirmations[s->confirmation_head];
    s->confirmations[s->confirmation_head] = 0;
    s->confirmation_head = (s->confirmation_head + 1) % sizeof(s->confirmations);
    --s->confirmation_count;
    return SV_OK;
}
SvResult sv_session_complete_request(SvSession *s, uint64_t sequence)
{
    if (!s->request.pending || s->request.sequence != sequence) return SV_STALE;
    s->request = (SvKeyRequest){0};
    return SV_OK;
}
