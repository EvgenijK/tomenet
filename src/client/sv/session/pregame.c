#include "session/pregame.h"
#include <string.h>

static void copy_text(char *out, size_t capacity, const char *text)
{
    size_t length = text ? strlen(text) : 0;
    if (length >= capacity) length = capacity - 1;
    if (length) memcpy(out, text, length);
    out[length] = 0;
}

static SvResult current(SvPregame *pregame, uint64_t generation)
{
    if (!pregame || !generation) return SV_INVALID;
    return pregame->generation == generation ? SV_OK : SV_STALE;
}

static void transition(SvPregame *pregame, SvPregamePhase phase, bool content_changed)
{
    if (pregame->phase != phase) {
        pregame->phase = phase;
        ++pregame->revision;
    } else if (content_changed) ++pregame->revision;
}

void sv_pregame_begin(SvPregame *pregame, uint64_t generation)
{
    if (!pregame) return;
    *pregame = (SvPregame){.generation = generation, .revision = 1,
                           .phase = SV_PREGAME_CONTACT};
}

SvResult sv_pregame_contact_ready(SvPregame *pregame, uint64_t generation)
{
    SvResult result = current(pregame, generation);
    if (result != SV_OK) return result;
    if (pregame->phase != SV_PREGAME_CONTACT) return SV_INVALID;
    transition(pregame, SV_PREGAME_AUTHENTICATING, false);
    return SV_OK;
}

static bool publish_overview(SvPregame *pregame, const SvLogin *login,
                             const SvContactSetup *setup)
{
    SvPregameCharacter characters[SV_LOGIN_MAX_CHARACTERS] = {{0}};
    size_t count = sv_login_count(login);
    if (count > SV_LOGIN_MAX_CHARACTERS) count = SV_LOGIN_MAX_CHARACTERS;
    for (size_t i = 0; i < count; ++i) {
        const SvLoginCharacter *source = sv_login_character(login, i);
        SvPregameCharacter *target = &characters[i];
        *target = (SvPregameCharacter){.level = source->level, .mode = source->mode};
        copy_text(target->name, sizeof(target->name), source->name);
        copy_text(target->location, sizeof(target->location), source->location);
        copy_text(target->race, sizeof(target->race),
                  source->race >= 0 && source->race < setup->race_count ?
                  setup->races[source->race].title : "?");
        copy_text(target->class_title, sizeof(target->class_title),
                  source->class_id >= 0 && source->class_id < setup->class_count ?
                  setup->classes[source->class_id].title : "?");
    }
    const uint32_t *flags = sv_login_flags(login);
    bool changed = pregame->character_count != count ||
        memcmp(pregame->characters, characters, sizeof(characters)) ||
        memcmp(pregame->server_flags, flags, sizeof(pregame->server_flags)) ||
        pregame->creation_flags != setup->creation_flags || !pregame->authenticated;
    pregame->character_count = count;
    memcpy(pregame->characters, characters, sizeof(characters));
    memcpy(pregame->server_flags, flags, sizeof(pregame->server_flags));
    pregame->creation_flags = setup->creation_flags;
    pregame->authenticated = true;
    return changed;
}

SvResult sv_pregame_sync_login(SvPregame *pregame, uint64_t generation,
                               const SvLogin *login, const SvContactSetup *setup,
                               bool motd_complete)
{
    SvResult result = current(pregame, generation);
    if (result != SV_OK) return result;
    if (!login || !setup) return SV_INVALID;
    if (pregame->phase == SV_PREGAME_FAILED ||
        pregame->phase == SV_PREGAME_DISCONNECTED)
        return SV_CLOSED;
    SvLoginState state = sv_login_state(login);
    if (state == SV_LOGIN_REJECTED) {
        bool changed = strcmp(pregame->reason, sv_login_reason(login)) != 0;
        copy_text(pregame->reason, sizeof(pregame->reason), sv_login_reason(login));
        transition(pregame, SV_PREGAME_FAILED, changed);
        return SV_OK;
    }
    if (state == SV_LOGIN_WAIT_FLAGS || state == SV_LOGIN_WAIT_LIST) {
        transition(pregame, SV_PREGAME_AUTHENTICATING, false);
        return SV_OK;
    }
    if (state == SV_LOGIN_OVERVIEW) {
        bool changed = publish_overview(pregame, login, setup);
        transition(pregame, SV_PREGAME_OVERVIEW, changed);
        return SV_OK;
    }
    if (state == SV_LOGIN_WAIT_STATUS) {
        bool changed = publish_overview(pregame, login, setup);
        const SvLoginCharacter *selected = sv_login_selected_character(login);
        if (selected) {
            changed = changed || strcmp(pregame->selected_character, selected->name) != 0;
            copy_text(pregame->selected_character,
                      sizeof(pregame->selected_character), selected->name);
        }
        transition(pregame, SV_PREGAME_CHARACTER_PENDING, changed);
        return SV_OK;
    }
    if (state == SV_LOGIN_SELECTED) {
        bool changed = publish_overview(pregame, login, setup);
        const SvLoginCharacter *selected = sv_login_selected_character(login);
        if (!selected) return SV_INVALID;
        changed = changed || strcmp(pregame->selected_character, selected->name) != 0;
        copy_text(pregame->selected_character, sizeof(pregame->selected_character),
                  selected->name);
        if (setup->motd_size > sizeof(pregame->motd) - 1) return SV_INVALID;
        changed = changed || pregame->motd_size != setup->motd_size ||
            memcmp(pregame->motd, setup->motd, setup->motd_size);
        pregame->motd_size = setup->motd_size;
        memcpy(pregame->motd, setup->motd, setup->motd_size);
        pregame->motd[setup->motd_size] = 0;
        transition(pregame, motd_complete ? SV_PREGAME_LIVE_HANDOFF : SV_PREGAME_MOTD,
                   changed);
        return SV_OK;
    }
    return SV_INVALID;
}

static SvResult terminal_transition(SvPregame *pregame, uint64_t generation,
                                    const char *reason, SvPregamePhase destination)
{
    SvResult result = current(pregame, generation);
    if (result != SV_OK) return result;
    if (pregame->phase == SV_PREGAME_FAILED ||
        pregame->phase == SV_PREGAME_DISCONNECTED)
        return SV_CLOSED;
    bool changed = strcmp(pregame->reason, reason ? reason : "") != 0;
    copy_text(pregame->reason, sizeof(pregame->reason), reason);
    transition(pregame, destination, changed);
    return SV_OK;
}

SvResult sv_pregame_fail(SvPregame *pregame, uint64_t generation,
                         const char *reason)
{
    return terminal_transition(pregame, generation, reason, SV_PREGAME_FAILED);
}

SvResult sv_pregame_disconnect(SvPregame *pregame, uint64_t generation,
                               const char *reason)
{
    return terminal_transition(pregame, generation, reason,
                               SV_PREGAME_DISCONNECTED);
}

const char *sv_pregame_phase_name(SvPregamePhase phase)
{
    switch (phase) {
    case SV_PREGAME_CONTACT: return "contact";
    case SV_PREGAME_AUTHENTICATING: return "authenticating";
    case SV_PREGAME_OVERVIEW: return "overview";
    case SV_PREGAME_CHARACTER_PENDING: return "character-pending";
    case SV_PREGAME_MOTD: return "motd";
    case SV_PREGAME_LIVE_HANDOFF: return "live-session-handoff";
    case SV_PREGAME_FAILED: return "failed";
    case SV_PREGAME_DISCONNECTED: return "disconnected";
    }
    return "unknown";
}
