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
    SvHud hud;
    SvStatusEffect status_effects[SV_STATUS_EFFECT_CAPACITY];
    size_t status_effect_head, status_effect_count;
    uint64_t status_effect_sequence;
    SvKeyRequest request;
    uint64_t request_sequence;
    SvMessage pending[SV_MESSAGE_PENDING];
    size_t head, count;
    uint64_t sequence;
    SvMessages messages;
    player_type player; /* Existing HP owner; hud.hp is its coherent read projection. */
    unsigned char drain;
    bool bar, boosted;
    uint64_t revision;
    SvControlState controls;
    SvContactSetup *character_setup;
    SvPingTelemetry ping;
    SvCharacterIdentity identity;
    char identity_name[SV_CONTACT_NAME_CAPACITY];
};
SvSession *sv_session_create(void)
{
    SvSession *session = calloc(1, sizeof(SvSession));
    if (session) for (size_t i = 0; i < 60; ++i) session->ping.samples[i] = -1;
    return session;
}
void sv_session_destroy(SvSession *s)
{
    if (!s) return;
    free(s->character_setup);
    free(s);
}
SvResult sv_session_set_character_setup(SvSession *s, const SvContactSetup *setup)
{
    if (!s || !setup || s->character_setup ||
        setup->motd_size > SV_CONTACT_MOTD_CAPACITY) return SV_INVALID;
    SvContactSetup *copy = malloc(sizeof(*copy));
    if (!copy) return SV_NO_MEMORY;
    memcpy(copy, setup, sizeof(*copy));
    s->character_setup = copy;
    return SV_OK;
}
const SvContactSetup *sv_session_character_setup(const SvSession *s)
{
    return s ? s->character_setup : NULL;
}
SvResult sv_session_set_identity_name(SvSession *s, const char *name)
{
    if (!s || !name || strlen(name) >= sizeof(s->identity_name)) return SV_INVALID;
    memcpy(s->identity_name, name, strlen(name) + 1);
    return SV_OK;
}
SvCharacterIdentity sv_session_identity(const SvSession *s)
{ return s ? s->identity : (SvCharacterIdentity){0}; }
SvStatus sv_session_status(const SvSession *s)
{
    SvStatus view = {s->player.mhp, s->player.chp, s->drain,
                    s->bar, s->boosted, s->revision != 0, s->revision};
    return view;
}
SvHud sv_session_hud(const SvSession *s) { return s ? s->hud : (SvHud){0}; }
SvResult sv_session_take_status_effect(SvSession *s, SvStatusEffect *out)
{
    if (!s || !out) return SV_INVALID;
    if (!s->status_effect_count) return SV_WAITING;
    *out = s->status_effects[s->status_effect_head];
    memset(&s->status_effects[s->status_effect_head], 0, sizeof(*out));
    s->status_effect_head = (s->status_effect_head + 1) % SV_STATUS_EFFECT_CAPACITY;
    --s->status_effect_count;
    return SV_OK;
}
static SvResult apply_hud(SvSession *s, const SvHudUpdate *update)
{
    if ((unsigned)update->field >= SV_HUD_FIELD_COUNT || update->field == SV_HUD_HP)
        return SV_INVALID;
    const SvHud *v = &update->values;
    if ((update->field == SV_HUD_DEMOGRAPHICS &&
         !memchr(v->demographics.body, 0, sizeof(v->demographics.body))) ||
        (update->field == SV_HUD_SANITY &&
         !memchr(v->sanity.label, 0, sizeof(v->sanity.label))) ||
        (update->field == SV_HUD_BLOWS &&
         !memchr(v->blows.label, 0, sizeof(v->blows.label))) ||
        (update->field == SV_HUD_EXTRA &&
         !memchr(v->extra, 0, sizeof(v->extra))) ||
        (update->field == SV_HUD_DEPTH &&
         (!memchr(v->depth.location, 0, sizeof(v->depth.location)) ||
          !memchr(v->depth.location_secondary, 0, sizeof(v->depth.location_secondary)) ||
          !memchr(v->depth.preposition, 0, sizeof(v->depth.preposition))))) return SV_INVALID;
    bool body = update->field == SV_HUD_DEMOGRAPHICS &&
        strcmp(s->hud.demographics.body, v->demographics.body) != 0;
    bool idle = update->field == SV_HUD_IDLE;
    if (s->hud.revision == UINT64_MAX ||
        ((body || idle) && (s->status_effect_count == SV_STATUS_EFFECT_CAPACITY ||
         s->status_effect_sequence == UINT64_MAX))) return SV_EVENT_OVERFLOW;
    /* Validate the complete input before appending an occurrence or mutating state. */
    if (update->field == SV_HUD_STATS &&
        (update->stat_index < 0 || update->stat_index >= 6)) return SV_INVALID;
    switch (update->field) {
#define FIELD(id, member) case id: s->hud.member = v->member; break
        FIELD(SV_HUD_MANA, mana);
        FIELD(SV_HUD_STAMINA, stamina);
        FIELD(SV_HUD_SANITY, sanity);
        case SV_HUD_STATS: s->hud.stats[update->stat_index] = v->stats[update->stat_index]; break;
        FIELD(SV_HUD_ARMOUR, armour);
        FIELD(SV_HUD_DEMOGRAPHICS, demographics);
        FIELD(SV_HUD_COMBAT, combat);
        FIELD(SV_HUD_EXPERIENCE, experience);
        FIELD(SV_HUD_CURRENCY, currency);
        FIELD(SV_HUD_CONFUSION, confusion);
        FIELD(SV_HUD_POISON, poison);
        FIELD(SV_HUD_STUDY, study);
        FIELD(SV_HUD_FOOD, food);
        FIELD(SV_HUD_FEAR, fear);
        case SV_HUD_SPEED: s->hud.speed = v->speed; s->hud.speed_boosted = v->speed_boosted; break;
        FIELD(SV_HUD_CUT, cut);
        FIELD(SV_HUD_BLIND, blind);
        FIELD(SV_HUD_STUN, stun);
        FIELD(SV_HUD_BLOWS, blows);
        FIELD(SV_HUD_MONSTER, monster);
        FIELD(SV_HUD_AFK, afk);
        FIELD(SV_HUD_MARTYR, martyr);
        FIELD(SV_HUD_IDLE, idle);
        case SV_HUD_ENCUMBRANCE: memcpy(s->hud.encumbrance, v->encumbrance, sizeof(v->encumbrance)); break;
        case SV_HUD_EXTRA: memcpy(s->hud.extra, v->extra, sizeof(v->extra)); break;
        FIELD(SV_HUD_INDICATORS, indicators);
        FIELD(SV_HUD_DEPTH, depth);
#undef FIELD
        default: return SV_INVALID;
    }
    s->hud.available |= UINT64_C(1) << update->field;
    ++s->hud.revision;
    if (body || idle) {
        SvStatusEffect effect = {.sequence = ++s->status_effect_sequence,
            .kind = body ? SV_STATUS_BODY_EFFECT : SV_STATUS_IDLE_EFFECT};
        if (body) memcpy(effect.body, v->demographics.body, sizeof(effect.body));
        else effect.idle = v->idle;
        s->status_effects[(s->status_effect_head + s->status_effect_count) % SV_STATUS_EFFECT_CAPACITY] = effect;
        ++s->status_effect_count;
    }
    return SV_OK;
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
    s->hud.hp = (SvHudResource){hp.maximum, hp.current, hp.bar, hp.boosted, hp.drain};
    s->hud.available |= UINT64_C(1) << SV_HUD_HP;
    ++s->hud.revision;
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
    case SV_CHANGE_IDENTITY: {
        if (!sv_character_identity_validate(&change->identity, s->character_setup)) {
            applied.result = SV_DECODE_ERROR;
            break;
        }
        SvCharacterIdentity identity = change->identity;
        identity.revision = s->identity.revision + 1;
        if (!identity.name_available)
            memcpy(identity.name, s->identity_name, sizeof(identity.name));
        s->identity = identity;
        break;
    }
    case SV_CHANGE_HUD: applied.result = apply_hud(s, &change->hud); break;
    case SV_CHANGE_HP:
        if (s->revision == UINT64_MAX || s->hud.revision == UINT64_MAX)
            applied.result = SV_EVENT_OVERFLOW;
        else applied.status = apply_hp(s, change->hp);
        break;
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
    case SV_CHANGE_CONFIRM:
    case SV_CHANGE_NOOP:
    case SV_CHANGE_PAUSE:
    case SV_CHANGE_FLUSH: break;
    case SV_CHANGE_SERVER_FLAGS:
        memcpy(s->controls.server_flags, change->server_flags, sizeof(change->server_flags));
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
SvResult sv_session_complete_request(SvSession *s, uint64_t sequence)
{
    if (!s->request.pending || s->request.sequence != sequence) return SV_STALE;
    s->request = (SvKeyRequest){0};
    return SV_OK;
}
