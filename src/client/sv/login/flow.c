#include "login/flow-internal.h"
#include "protocol/login-identity.h"
#include "../../../temporary/sv/stubs/pregame-ui.h"
#include "../../../common/pack.h"
#include <stdio.h>
#include <string.h>
static const char *contact_rejection_reason(unsigned rejection)
{
    switch (rejection) {
    case E_VERSION_OLD: return "Server rejected contact: client version too old.";
    case E_VERSION_UNKNOWN: return "Server rejected contact: incompatible version.";
    case E_GAME_FULL: return "Server rejected contact: game is full.";
    case E_TWO_PLAYERS: return "Server rejected contact: another character is online.";
    case E_PASSWORD: return "Server rejected contact: invalid password.";
    case E_IN_USE_DUP: return "Server rejected contact: duplicate login.";
    case E_LETTER: return "Server rejected contact: invalid account name.";
    case E_IN_USE: return "Server rejected contact: account in use from another address.";
    case E_SOCKET: return "Server rejected contact: server socket error.";
    case E_INVAL: return "Server rejected contact: invalid identity.";
    case E_INVITE: return "Server rejected contact: members only.";
    case E_BANNED: return "Server rejected contact: temporarily banned.";
    case E_LENGTH: return "Server rejected contact: account name too short.";
    case E_IN_USE_PC: return "Server rejected contact: account in use on this PC.";
    case E_CLOSED: return "Server rejected contact: server closing.";
    }
    return "Server rejected contact.";
}

static const char *contact_status(SvSocketState state, unsigned rejection,
                                  const SvEndpoint *endpoint, SvResult failure,
                                  char *status, size_t status_size)
{
    switch (state) {
    case SV_SOCKET_RESOLVING: return "Resolving server address...";
    case SV_SOCKET_CONNECTING: return "Connecting to server...";
    case SV_SOCKET_NEGOTIATING: return "Negotiating version and setup...";
    case SV_SOCKET_READY: return "Contact established; waiting for account overview.";
    case SV_SOCKET_DNS_ERROR: return "Cannot resolve server address. R retries; Escape exits.";
    case SV_SOCKET_CONNECT_ERROR: return "Cannot open server socket. R retries; Escape exits.";
    case SV_SOCKET_TIMEOUT: return "Server timed out. R retries; Escape exits.";
    case SV_SOCKET_CLOSED: return "Server closed the connection. R retries; Escape exits.";
    case SV_SOCKET_PROTOCOL_ERROR: return "Invalid contact or network packet. R retries; Escape exits.";
    case SV_SOCKET_REJECTED:
        SDL_snprintf(status,status_size,"%s Escape exits.",contact_rejection_reason(rejection));
        return status;
    case SV_SOCKET_VERIFY_ERROR: return "Verification failed. R retries; Escape exits.";
    case SV_SOCKET_SETUP_ERROR: {
        const char *reason=failure==SV_CLOSED?
            "The server closed the connection before its configuration was received.":
            failure==SV_INPUT_OVERFLOW?
            "The server configuration exceeds this client's supported size.":
            "The server sent configuration this client cannot read.";
        SDL_snprintf(status,status_size,"%s:%u: %s Retry (R) or cancel (Esc).",
                     endpoint->host,(unsigned)endpoint->port,reason);
        return status;
    }
    }
    return "Contact stopped.";
}

static SvResult handoff_contact_bytes(SvContactSocket *connection, SvSessionWire *wire)
{
    SvOutput needed = sv_contact_socket_take_remaining(connection, NULL, 0);
    if (needed.result != SV_OK && needed.result != SV_OUTPUT_TOO_SMALL) return needed.result;
    wire->remaining = SDL_malloc(needed.size ? needed.size : 1);
    if (!wire->remaining) return SV_NO_MEMORY;
    SvOutput extra = sv_contact_socket_take_remaining(connection, wire->remaining, needed.size);
    if (extra.result != SV_OK) return extra.result;
    wire->remaining_size = extra.size;
    return SV_OK;
}

static SvSocketState pump_login(SvContactSocket *connection, SvLogin *login,
                                SvSessionWire *wire, SvResult *failure)
{
    SvResult progress = sv_login_tick(login);
    if (progress != SV_OK && progress != SV_BACKPRESSURE) {
        *failure = progress;
        return SV_SOCKET_PROTOCOL_ERROR;
    }
    if (wire->sent == wire->size &&
        SDL_GetTicks() - sv_contact_socket_last_sent(connection) >= 2000u) {
        SvResult keepalive = sv_login_keepalive(login);
        if (keepalive != SV_OK && keepalive != SV_BACKPRESSURE) {
            *failure = keepalive;
            return SV_SOCKET_PROTOCOL_ERROR;
        }
    }
    if (wire->sent == wire->size) {
        SvOutput output = sv_login_take_output(login, wire->sending,
                                                sizeof(wire->sending));
        if (output.result == SV_OK) { wire->size = output.size; wire->sent = 0; }
        else if (output.result != SV_WAITING) {
            *failure = output.result;
            return SV_SOCKET_PROTOCOL_ERROR;
        }
    }
    if (wire->sent < wire->size) {
        SvOutput sent = sv_contact_socket_write(connection, wire->sending + wire->sent,
                                                 wire->size - wire->sent);
        if (sent.result == SV_OK) wire->sent += sent.size;
        else if (sent.result != SV_WAITING) return SV_SOCKET_CLOSED;
    }
    /* Resume already owned input after output drain, without appending the same
     * socket bytes again. Backpressure is progress waiting, not malformed input. */
    progress = sv_login_tick(login);
    if (progress == SV_BACKPRESSURE) return SV_SOCKET_READY;
    if (progress != SV_OK) {
        *failure = progress;
        return SV_SOCKET_PROTOCOL_ERROR;
    }
    unsigned char incoming[4096];
    for (int i = 0; i < 4; ++i) {
        if (sv_login_play_stream_complete(login)) break;
        SvOutput received;
        if (wire->remaining_at < wire->remaining_size) {
            size_t count = wire->remaining_size - wire->remaining_at;
            if (count > sizeof(incoming)) count = sizeof(incoming);
            memcpy(incoming, wire->remaining + wire->remaining_at, count);
            wire->remaining_at += count;
            received = (SvOutput){SV_OK, count};
            if (wire->remaining_at == wire->remaining_size) {
                SDL_free(wire->remaining);
                wire->remaining = NULL;
                wire->remaining_size = wire->remaining_at = 0;
            }
        } else if (sv_login_state(login) == SV_LOGIN_REJECTED ||
                 sv_login_state(login) == SV_LOGIN_PEER_QUIT) break;
        else received = sv_contact_socket_read(connection, incoming, sizeof(incoming));
        if (received.result == SV_WAITING) break;
        if (received.result != SV_OK) return SV_SOCKET_CLOSED;
        *failure = sv_login_receive(login, incoming, received.size);
        if (*failure == SV_BACKPRESSURE) { *failure = SV_OK; return SV_SOCKET_READY; }
        if (*failure != SV_OK)
            return SV_SOCKET_PROTOCOL_ERROR;
    }
    return SV_SOCKET_READY;
}

static void publish_outcome(const SvPregame *pregame, SvEndpointOutcome *outcome,
                            uint64_t prior_generation,
                            SvCredentialSaveState credential_save,
                            const SvOrdinaryNameEvidence *name,
                            const SvNameRecovery *recovery, const SvBirthSnapshot *birth,
                            const SvBirthPlaceholderEvidence *placeholder)
{
    if (!pregame || !outcome) return;
    *outcome = (SvEndpointOutcome){.generation = pregame->generation,
                                   .prior_generation = prior_generation,
                                   .revision = pregame->revision,
                                   .phase = pregame->phase,
                                   .authenticated = pregame->authenticated,
                                   .character_count = pregame->character_count,
                                   .creation_flags = pregame->creation_flags,
                                   .credential_save_started = credential_save !=
                                       SV_CREDENTIAL_SAVE_NOT_STARTED,
                                   .credential_saved = credential_save ==
                                       SV_CREDENTIAL_SAVE_SAVED,
                                   .credential_save = credential_save,
                                   .ordinary_name_started = name && name->started &&
                                       !name->dedicated,
                                   .ordinary_name_cancelled = name && name->cancelled &&
                                       !name->dedicated,
                                   .exclusive_name_started = name && name->started &&
                                       name->dedicated,
                                   .exclusive_name_cancelled = name && name->cancelled &&
                                       name->dedicated,
                                   .dedicated_creation = pregame->dedicated_creation,
                                   .dedicated_mode_flags = pregame->dedicated_mode_flags,
                                   .name_submitted = name && name->submitted,
                                   .random_name_suggested = name && name->random_suggested,
                                   .name_input_truncated = name && name->input_truncated,
                                   .name_input_method_used = name && name->input_method_used,
                                   .name_keyboard_edited = name && name->keyboard_edited,
                                   .name_mouse_selected = name && name->mouse_selected,
                                   .name_paste_used = name && name->paste_used,
                                   .name_maximum_draft_bytes = name ?
                                       name->maximum_draft_bytes : 0,
                                   .reincarnation_requested = name &&
                                       name->reincarnation_requested,
                                   .name_submit_phase = name ? name->submit_phase :
                                       SV_PREGAME_CONTACT,
                                   .name_entry_generation = name ? name->entry_generation : 0,
                                   .name_cancel_generation = name ? name->cancel_generation : 0,
                                   .name_entry_overview_revision = name ? name->entry_revision : 0,
                                   .name_cancel_overview_revision = name ? name->cancel_revision : 0,
                                   .name_rejected = recovery && recovery->rejected,
                                   .name_recovery_started = recovery && recovery->started,
                                   .name_recovery_draft_preserved = recovery &&
                                       recovery->draft_preserved,
                                   .name_rejected_generation = recovery ?
                                       recovery->rejected_generation : 0,
                                   .name_recovery_generation = recovery ?
                                       recovery->recovery_generation : 0,
                                   .birth_entered = birth && birth->entered,
                                   .birth_sex_confirmed = birth && birth->confirmed,
                                   .birth_keyboard_navigated = birth &&
                                       birth->keyboard_navigated,
                                   .birth_mouse_navigated = birth && birth->mouse_navigated,
                                   .birth_random_used = birth && birth->random_used,
                                   .birth_dna_used = birth && birth->dna_used,
                                   .birth_illegal_ignored = birth && birth->illegal_ignored,
                                   .birth_auto_reincarnation = birth &&
                                       birth->auto_reincarnation,
                                   .birth_dna_available = birth &&
                                       birth->dna_available,
                                   .birth_quit = birth && birth->quit,
                                   .birth_active = birth && birth->active,
                                   .birth_closed = birth && birth->closed,
                                   .birth_class_confirmed = birth &&
                                       birth->class_confirmed,
                                   .birth_class_random_used = birth &&
                                       birth->class_random_used,
                                   .birth_class_dna_used = birth &&
                                       birth->class_dna_used,
                                   .birth_race_confirmed = birth &&
                                       birth->race_confirmed,
                                   .birth_race_random_used = birth &&
                                       birth->race_random_used,
                                   .birth_race_dna_used = birth &&
                                       birth->race_dna_used,
                                   .birth_trait_confirmed = birth &&
                                       birth->trait_confirmed,
                                   .birth_trait_random_used = birth &&
                                       birth->trait_random_used,
                                   .birth_trait_dna_used = birth &&
                                       birth->trait_dna_used,
                                   .birth_trait_skipped = birth &&
                                       birth->trait_skipped,
                                   .birth_body_confirmed = birth &&
                                       birth->body_confirmed,
                                   .birth_body_random_used = birth &&
                                       birth->body_random_used,
                                   .birth_body_dna_used = birth &&
                                       birth->body_dna_used,
                                   .birth_body_skipped = birth &&
                                       birth->body_skipped,
                                   .birth_stats_confirmed = birth &&
                                       birth->stats_confirmed,
                                   .birth_stats_random_used = birth &&
                                       birth->stats_random_used,
                                   .birth_stats_dna_used = birth &&
                                       birth->stats_dna_used,
                                   .birth_stats_point_buy = birth &&
                                       birth->stats_point_buy,
                                   .birth_stats_confirmation_pending = birth &&
                                       birth->stats_confirmation_pending,
                                   .birth_mode_confirmed = birth &&
                                       birth->mode_confirmed,
                                   .birth_mode_random_used = birth &&
                                       birth->mode_random_used,
                                   .birth_mode_dna_used = birth && birth->mode_dna_used,
                                   .birth_mode_ready = birth && birth->mode_ready,
                                   .birth_owner = birth ? birth->owner : SV_BIRTH_NONE,
                                   .birth_highlighted_sex = birth ?
                                       birth->highlighted_sex : SV_BIRTH_SEX_NONE,
                                   .birth_sex = birth ? birth->chosen_sex :
                                       SV_BIRTH_SEX_NONE,
                                   .birth_highlighted_class = birth ?
                                       birth->highlighted_class : SV_BIRTH_CLASS_NONE,
                                   .birth_class = birth ? birth->chosen_class :
                                       SV_BIRTH_CLASS_NONE,
                                   .birth_visible_class_count = birth ?
                                       birth->visible_class_count : 0,
                                   .birth_highlighted_race = birth ?
                                       birth->highlighted_race : SV_BIRTH_RACE_NONE,
                                   .birth_race = birth ? birth->chosen_race :
                                       SV_BIRTH_RACE_NONE,
                                   .birth_legal_class_count = birth ?
                                       birth->legal_class_count : 0,
                                   .birth_legal_race_count = birth ?
                                       birth->legal_race_count : 0,
                                   .birth_race_count = birth ? birth->race_count : 0,
                                   .birth_highlighted_trait = birth ?
                                       birth->highlighted_trait : SV_BIRTH_TRAIT_NONE,
                                   .birth_trait = birth ? birth->chosen_trait :
                                       SV_BIRTH_TRAIT_NONE,
                                   .birth_legal_trait_count = birth ?
                                       birth->legal_trait_count : 0,
                                   .birth_trait_count = birth ? birth->trait_count : 0,
                                   .birth_highlighted_body = birth ?
                                       birth->highlighted_body : SV_BIRTH_BODY_NONE,
                                   .birth_body = birth ? birth->chosen_body :
                                       SV_BIRTH_BODY_NONE,
                                   .birth_legal_body_count = birth ?
                                       birth->legal_body_count : 0,
                                   .birth_body_count = birth && !birth->body_skipped ?
                                       2 : 0,
                                   .birth_highlighted_stat = birth ?
                                       birth->highlighted_stat : -1,
                                   .birth_legal_stat_count = birth ?
                                       birth->legal_stat_count : 0,
                                   .birth_stats_order_count = birth ?
                                       birth->stats_order_count : 0,
                                   .birth_stats_remaining = birth ?
                                       birth->stats_remaining : 0,
                                   .birth_highlighted_mode = birth ?
                                       birth->highlighted_mode : SV_BIRTH_MODE_NONE,
                                   .birth_mode = birth ? birth->chosen_mode :
                                       SV_BIRTH_MODE_NONE,
                                   .birth_legal_mode_count = birth ?
                                       birth->legal_mode_count : 0,
                                   .birth_placeholder_open_count = placeholder ?
                                       placeholder->open_count : 0,
                                   .birth_placeholder_close_count = placeholder ?
                                       placeholder->close_count : 0,
                                   .birth_placeholder_owner_mask = placeholder ?
                                       placeholder->owner_mask : 0,
                                   .birth_placeholder_draft_preserved = placeholder &&
                                       placeholder->draft_preserved,
                                   .birth_generation = birth ? birth->generation : 0};
    if (birth) {
        memcpy(outcome->birth_visible_classes, birth->visible_classes,
               birth->visible_class_count);
        memcpy(outcome->birth_legal_classes, birth->legal_classes,
               birth->legal_class_count);
        memcpy(outcome->birth_legal_races, birth->legal_races,
               birth->legal_race_count);
        memcpy(outcome->birth_legal_traits, birth->legal_traits,
               birth->legal_trait_count);
        memcpy(outcome->birth_legal_bodies, birth->legal_bodies,
               birth->legal_body_count * sizeof(birth->legal_bodies[0]));
        memcpy(outcome->birth_stats,
               birth->stats_confirmed ? birth->committed_stats : birth->stats,
               sizeof(outcome->birth_stats));
        memcpy(outcome->birth_committed_stats, birth->committed_stats,
               sizeof(birth->committed_stats));
        memcpy(outcome->birth_legal_stats, birth->legal_stats,
               birth->legal_stat_count);
        memcpy(outcome->birth_legal_modes, birth->legal_modes,
               birth->legal_mode_count * sizeof(birth->legal_modes[0]));
    }
    if (placeholder)
        memcpy(outcome->birth_placeholder_text, placeholder->text,
               sizeof(outcome->birth_placeholder_text));
    memcpy(outcome->server_flags, pregame->server_flags,
           sizeof(outcome->server_flags));
    memcpy(outcome->selected_character, pregame->selected_character,
           sizeof(outcome->selected_character));
    memcpy(outcome->reason, pregame->reason, sizeof(outcome->reason));
    if (recovery)
        memcpy(outcome->name_rejection_reason, recovery->reason,
               sizeof(outcome->name_rejection_reason));
}

bool sv_login_flow_begin(SvLoginFlow *flow, const char *profile_root,
                          bool first_run, const char *font_resource)
{
    /* Every connection gets fresh attempt state, including after Back. */
    if (flow->attempt.connection || flow->attempt.birth || flow->attempt.preferences) return false;
    memset(&flow->attempt, 0, sizeof(flow->attempt));
    flow->attempt.lua_begin_result.status = SV_LUA_NOT_READY;
    flow->profile_root = profile_root;
    flow->first_run = first_run;
    flow->font_resource = font_resource;
    unsigned char iaddr[6];
    if (!sv_login_identity(profile_root, iaddr))
        return SDL_SetError("Cannot derive versioned login identity");
    if (flow->endpoint.protocol >= 2 && strchr(flow->options.password, '*'))
        return SDL_SetError("Password contains '*' and cannot round-trip through this server protocol");
    SvContactIdentity identity = {flow->options.real_name ? flow->options.real_name : "PLAYER",
        flow->options.account, "localhost", flow->options.password};
    flow->attempt.connection = sv_contact_socket_start(flow->endpoint.host, flow->endpoint.port,
                                               flow->endpoint.protocol, &identity);
    if (!flow->attempt.connection)
        return SDL_SetError("Cannot start contact (invalid identity or unavailable socket)");
    flow->attempt.birth = sv_birth_create();
    if (!flow->attempt.birth) { sv_contact_socket_stop(flow->attempt.connection); flow->attempt.connection = NULL; return false; }
    flow->attempt.state = SV_SOCKET_RESOLVING;
    flow->attempt.connection_failure = flow->attempt.session_error = SV_OK;
    flow->attempt.result = 1;
    flow->attempt.birth_placeholder.draft_preserved = true;
    flow->attempt.credential_save = SV_CREDENTIAL_SAVE_NOT_STARTED;
    flow->generation = SDL_GetTicksNS();
    if (!flow->generation || flow->generation == flow->prior_generation)
        flow->generation = flow->prior_generation + 1;
    if (!flow->generation) flow->generation = 1;
    memcpy(flow->attempt.iaddr, iaddr, sizeof(iaddr));
    sv_pregame_begin(&flow->attempt.pregame, flow->generation, first_run);
    return true;
}
void sv_login_flow_poll(SvLoginFlow *flow)
{
    SvBirthSnapshot birth_snapshot;
    sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
        if (flow->attempt.connection) {
            flow->attempt.state = sv_contact_socket_poll(flow->attempt.connection);
            flow->attempt.connection_failure = sv_contact_socket_failure_result(flow->attempt.connection);
            flow->attempt.rejection = sv_contact_socket_rejection(flow->attempt.connection);
        }
        if (flow->attempt.state == SV_SOCKET_READY && !flow->attempt.login) {
            flow->attempt.login = sv_login_create(sv_contact_socket_version(flow->attempt.connection), flow->attempt.iaddr);
            if (!flow->attempt.login) { flow->attempt.session_error = SV_NO_MEMORY; flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR; }
            else {
                if (flow->options.transfer_factory.create) {
                    if (!flow->options.transfer_factory.create(
                            flow->options.transfer_factory.context, flow->generation,
                            sv_login_version(flow->attempt.login), &flow->attempt.transfer) ||
                        sv_login_attach_transfer(flow->attempt.login, flow->attempt.transfer) != SV_OK) {
                        flow->attempt.session_error = SV_NO_MEMORY;
                        flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                        return;
                    }
                }
                if (flow->options.audio)
                    sv_login_set_audio(flow->attempt.login, flow->options.audio);
                if (flow->options.map_font)
                    sv_login_set_rendering(flow->attempt.login,
                        &flow->options.map_font->definitions,flow->options.graphics_mode,
                        flow->options.graphics_tiles?flow->options.graphics_tiles:"NO_GRAPHICS",
                        flow->options.map_font->effective);
                if (sv_pregame_contact_ready(&flow->attempt.pregame, flow->generation) != SV_OK) {
                    flow->attempt.session_error = SV_INVALID;
                    flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                    return;
                }
                sv_login_interaction_begin(&flow->attempt.login_input, flow->attempt.login, flow->options.character,
                    flow->options.skip_motd, flow->attempt.pregame.first_run,
                    sv_contact_socket_setup(flow->attempt.connection)->creation_flags);
                if (flow->options.auto_reincarnation && flow->options.character &&
                    !sv_login_interaction_request_reincarnation(
                        &flow->attempt.login_input, flow->options.character)) {
                    flow->attempt.session_error = SV_INVALID;
                    flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                    return;
                }
                if ((flow->attempt.session_error = handoff_contact_bytes(flow->attempt.connection, &flow->attempt.wire)) != SV_OK)
                    flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
            }
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login && flow->attempt.birth_started &&
            sv_login_state(flow->attempt.login) == SV_LOGIN_SELECTED) {
            SvBirthPlayDraft draft;
            if (sv_birth_committed_play(flow->attempt.birth, flow->generation, &draft)) {
                flow->attempt.session_error = sv_login_start_play(flow->attempt.login, draft.mode, draft.race,
                    draft.class_id, draft.trait, draft.stats,
                    flow->options.map_font?flow->options.map_font->effective:"");
                if (flow->attempt.session_error != SV_OK) flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                else {
                    flow->attempt.play_started = true;
                    if (!flow->attempt.dna_saved) {
                        const SvLoginCharacter *character=sv_login_selected_character(flow->attempt.login);
                        SvDna dna={.mode=draft.mode,.class_id=draft.class_id,.race=draft.race,.trait=draft.trait};
                        memcpy(dna.stats,draft.stats,sizeof(dna.stats));
                        if (character && sv_dna_save(&dna,flow->options.root,character->name)!=SV_FILE_OK)
                            fprintf(stderr,"SV DNA: saving birth data failed\n");
                        flow->attempt.dna_saved=true;
                    }
                }
            }
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login)
            flow->attempt.state = pump_login(flow->attempt.connection, flow->attempt.login, &flow->attempt.wire, &flow->attempt.session_error);
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login &&
            sv_login_state(flow->attempt.login) == SV_LOGIN_SELECTED &&
            !flow->attempt.settings_applied && (flow->options.settings || flow->options.preferences)) {
            const SvLoginCharacter *character = sv_login_selected_character(flow->attempt.login);
            SvSettingsOptions current;
            if (!character) {
                flow->attempt.session_error = SV_INVALID;
                flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
            } else {
                if (flow->options.preferences) {
                    SvPreferenceContext context = sv_preferences_default_context();
                    SvPreferenceResult result;
                    context.generation = flow->generation;
                    flow->attempt.preferences = sv_preferences_character(
                        flow->options.preferences, character->name, &context, &result);
                    if (!flow->attempt.preferences || result.status == SV_PREF_FATAL) {
                        flow->attempt.session_error = flow->attempt.preferences ? SV_INPUT_OVERFLOW : SV_NO_MEMORY;
                        flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                        return;
                    }
                    for (size_t i = 0; i < sv_preferences_diagnostic_count(flow->attempt.preferences); ++i) {
                        const SvPreferenceDiagnostic *diagnostic = sv_preferences_diagnostic(flow->attempt.preferences, i);
                        fprintf(stderr, "SV character preferences %s:%u: %s\n", diagnostic->source.path,
                            diagnostic->line, diagnostic->message);
                    }
                    size_t messages = 0, actions = 0;
                    for (size_t i = 0; i < sv_preferences_effect_count(flow->attempt.preferences); ++i) {
                        const SvPreferenceEffect *effect = sv_preferences_effect(flow->attempt.preferences, i);
                        messages += effect->kind == SV_PREF_MESSAGE;
                        actions += effect->kind == SV_PREF_ACTION;
                    }
                    if (messages || actions)
                        fprintf(stderr, "SV character preferences generation=%llu: retained %zu messages and %zu actions; message presentation and macro runtime pending\n",
                            (unsigned long long)flow->generation, messages, actions);
                    current = *sv_preferences_options(flow->attempt.preferences);
                } else if (!sv_settings_options_character(&current, flow->options.settings,
                               flow->profile_root, flow->options.library, character->name))
                    fprintf(stderr, "SV character options: invalid/unavailable records; retaining usable values\n");
                /* big_map is a projection of the approved SV layout, not a
                 * competing setting inherited from a legacy OPT layer. */
                sv_settings_option_set(&current, "big_map", flow->options.settings_wide);
                if (flow->attempt.preferences)
                    *sv_preferences_options_mutable(flow->attempt.preferences) = current;
                if(flow->options.map_font && flow->attempt.preferences) {
                    SvVisualDefinitions definitions;
                    sv_map_font_definitions(&definitions,flow->attempt.preferences);
                    sv_login_set_visuals(flow->attempt.login,&definitions,
                        flow->options.map_font->effective);
                }
                /* Packet generation follows the character overlay, before birth
                 * or play setup. Retry gets a fresh copy of the global snapshot. */
                flow->attempt.session_error = sv_login_set_options(flow->attempt.login, &current);
                /* The pre-PLAY server dispatcher accepts PLAY, not standalone
                 * OPTIONS. Retain effective values for the options embedded in
                 * that common serializer (baseline Net_start clears its queue). */
                if (flow->attempt.session_error != SV_OK)
                    flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                else flow->attempt.settings_applied = true;
            }
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login &&
            flow->attempt.settings_applied && !flow->attempt.play_started &&
            !sv_login_creating_character(flow->attempt.login) &&
            sv_login_state(flow->attempt.login)==SV_LOGIN_SELECTED &&
            sv_login_interaction_complete(&flow->attempt.login_input)) {
            const SvLoginCharacter *selected=sv_login_selected_character(flow->attempt.login);
            const int16_t stats[6]={0};
            flow->attempt.session_error=sv_login_start_play(flow->attempt.login,
                selected->mode,selected->race,selected->class_id,0,stats,
                flow->options.map_font?flow->options.map_font->effective:"");
            if(flow->attempt.session_error==SV_OK)flow->attempt.play_started=true;
            else if(flow->attempt.session_error!=SV_BACKPRESSURE)flow->attempt.state=SV_SOCKET_PROTOCOL_ERROR;
        }
        if(flow->attempt.settings_applied && !flow->preparation && flow->attempt.login) {
            flow->preparation=sv_preparation_create(flow);
            if(!flow->preparation){flow->attempt.session_error=SV_NO_MEMORY;flow->attempt.state=SV_SOCKET_PROTOCOL_ERROR;}
        }
        if(flow->preparation && flow->attempt.state==SV_SOCKET_READY) {
            SvResult prepared=sv_preparation_poll(flow->preparation);
            if(prepared!=SV_OK && prepared!=SV_WAITING && prepared!=SV_BACKPRESSURE) {
                flow->attempt.session_error=prepared;flow->attempt.state=prepared==SV_CLOSED?SV_SOCKET_CLOSED:SV_SOCKET_PROTOCOL_ERROR;
            }
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login) {
            if (sv_login_state(flow->attempt.login) == SV_LOGIN_PLAY_ACKNOWLEDGED &&
                flow->options.audio && !flow->attempt.audio_reported) {
                flow->attempt.session_error = sv_login_send_audio(flow->attempt.login);
                if (flow->attempt.session_error != SV_OK) {
                    flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                    return;
                }
                flow->attempt.audio_reported = true;
            }
            SvLoginTransition transition = sv_login_interaction_sync(&flow->attempt.login_input);
            if (transition != SV_LOGIN_TRANSITION_NONE)
                flow->input_epoch++;
            if (transition == SV_LOGIN_TRANSITION_OVERVIEW &&
                flow->recovery.pending &&
                sv_login_interaction_restore_name(&flow->attempt.login_input, flow->recovery.draft,
                    flow->recovery.dedicated) ==
                    SV_LOGIN_INPUT_NAME_STARTED) {
                flow->recovery.pending = false;
                flow->recovery.started = true;
                flow->recovery.recovery_generation = flow->generation;
                flow->recovery.draft_preserved =
                    !strcmp(flow->attempt.login_input.character_name.bytes, flow->recovery.draft);
                flow->attempt.name_evidence.started = true;
                flow->attempt.name_evidence.dedicated = flow->attempt.login_input.dedicated_creation;
                flow->attempt.name_evidence.entry_generation = flow->generation;
                flow->attempt.name_evidence.entry_revision = flow->attempt.pregame.revision;
            }
        }
        if (flow->attempt.state==SV_SOCKET_READY && flow->attempt.login &&
            sv_login_state(flow->attempt.login)==SV_LOGIN_PLAY_ACKNOWLEDGED && !flow->attempt.font_reported) {
            SvResult sent=sv_login_send_font(flow->attempt.login);
            if (sent==SV_OK) flow->attempt.font_reported=true;
            else { flow->attempt.session_error=sent; flow->attempt.state=SV_SOCKET_PROTOCOL_ERROR; }
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login) {
            SvResult applied = sv_pregame_sync_login(&flow->attempt.pregame, flow->generation, flow->attempt.login,
                sv_contact_socket_setup(flow->attempt.connection), flow->attempt.login_input.motd_complete);
            if (applied != SV_OK) { flow->attempt.session_error = applied; flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR; }
            else if (flow->attempt.pregame.phase == SV_PREGAME_BIRTH && !flow->attempt.birth_started) {
                SvEndpointOptions birth_options=flow->options;
                SvDna dna;
                const SvLoginCharacter *created=sv_login_selected_character(flow->attempt.login);
                const char *previous=sv_login_reincarnation_requested(flow->attempt.login)?flow->previous_character:NULL;
                SvFileResult loaded=created?sv_dna_load(&dna,flow->options.root,flow->options.library,
                    created->name,previous):SV_FILE_MISSING;
                if (loaded==SV_FILE_OK) {
                    birth_options.birth_dna_available=true;
                    birth_options.birth_dna_sex=(dna.mode&0x100)?SV_BIRTH_SEX_MALE:SV_BIRTH_SEX_FEMALE;
                    birth_options.birth_dna_body=(dna.mode&0x200)?SV_BIRTH_BODY_FRUIT_BAT:SV_BIRTH_BODY_NORMAL;
                    birth_options.birth_dna_mode=(SvBirthMode)(dna.mode&~0x300);
                    birth_options.birth_dna_class=dna.class_id; birth_options.birth_dna_race=dna.race;
                    birth_options.birth_dna_trait=dna.trait;
                    memcpy(birth_options.birth_dna_stats,dna.stats,sizeof(dna.stats));
                    birth_options.birth_dna_class_available=birth_options.birth_dna_race_available=true;
                    birth_options.birth_dna_trait_available=birth_options.birth_dna_body_available=true;
                    birth_options.birth_dna_stats_available=birth_options.birth_dna_mode_available=true;
                } else if (loaded!=SV_FILE_MISSING) fprintf(stderr,"SV DNA: birth data unavailable or invalid\n");
                flow->attempt.session_error = sv_birth_begin(flow->attempt.birth, flow->generation,
                    sv_contact_socket_setup(flow->attempt.connection), flow->attempt.pregame.server_flags,
                    flow->attempt.pregame.first_run, flow->attempt.pregame.dedicated_mode_flags,
                    birth_options.birth_dna_available,
                    birth_options.birth_dna_sex, birth_options.birth_dna_class_available,
                    birth_options.birth_dna_class, birth_options.birth_dna_race_available,
                    birth_options.birth_dna_race, birth_options.birth_dna_trait_available,
                    birth_options.birth_dna_trait, birth_options.birth_dna_body_available,
                    birth_options.birth_dna_body, birth_options.birth_dna_stats_available,
                    birth_options.birth_dna_stats, birth_options.birth_dna_mode_available,
                    birth_options.birth_dna_mode);
                if (flow->attempt.session_error != SV_OK) flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                else {
                    flow->input_epoch++;
                    flow->attempt.birth_started = true;
                    flow->attempt.birth_input_since_ns = SDL_GetTicksNS();
                }
            }
        }
        if (flow->attempt.login && (sv_login_state(flow->attempt.login) == SV_LOGIN_REJECTED ||
                      sv_login_state(flow->attempt.login) == SV_LOGIN_PEER_QUIT)) {
            if (!flow->attempt.play_started && sv_login_creating_character(flow->attempt.login) && !flow->recovery.pending) {
                flow->attempt.character_name_rejected = true;
                flow->recovery.pending = true;
                flow->recovery.rejected = true;
                flow->recovery.rejected_generation = flow->generation;
                flow->recovery.dedicated = flow->attempt.login_input.dedicated_creation;
                SDL_strlcpy(flow->recovery.draft, flow->attempt.login_input.character_name.bytes,
                            sizeof(flow->recovery.draft));
                SDL_strlcpy(flow->recovery.reason, sv_login_reason(flow->attempt.login),
                            sizeof(flow->recovery.reason));
            }
            SDL_snprintf(flow->attempt.failure_status, sizeof(flow->attempt.failure_status),
                         flow->attempt.play_started ? "%s  Escape exits." :
                         flow->options.retry_login_disabled && flow->attempt.character_name_rejected ?
                         "%s  Retry login is disabled; exiting." :
                         "%s  Press R to retry or Escape to exit.",
                         sv_login_reason(flow->attempt.login));
            flow->attempt.status = flow->attempt.failure_status;
            flow->attempt.state = SV_SOCKET_REJECTED;
        } else if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login) {
            SvLoginState phase = sv_login_state(flow->attempt.login);
            if (phase == SV_LOGIN_OVERVIEW) {
                flow->attempt.status = flow->attempt.pregame.reason[0] ? flow->attempt.pregame.reason :
                    flow->attempt.login_input.ordinary_name_active ?
                    "New character name: Escape cancels and returns to the overview." :
                    (flow->attempt.save_message ? flow->attempt.save_message :
                     (sv_login_interaction_exclusive_modes(&flow->attempt.login_input) ?
                      "Account confirmed. Select a character, N creates, E creates exclusive, Q quits." :
                      "Account confirmed. Select a character, N creates, Q quits."));
            } else if (phase == SV_LOGIN_WAIT_STATUS)
                flow->attempt.status = "Waiting for selected character status...";
            else if (phase == SV_LOGIN_WAIT_PLAY)
                flow->attempt.status = "Waiting for authoritative play acknowledgement...";
            else if (phase == SV_LOGIN_SELECTED || phase == SV_LOGIN_PLAY_ACKNOWLEDGED) {
                if (phase == SV_LOGIN_SELECTED && sv_login_creating_character(flow->attempt.login)) {
                    flow->attempt.status =
                        "Character name accepted; handing off to character creation.";
                } else {
                    if (!flow->attempt.selected_reported) {
                        puts("SV character selected; peer MOTD acknowledgement pending");
                        flow->attempt.selected_reported = true;
                    }
                    flow->attempt.status = flow->attempt.save_failed ?
                        (flow->attempt.login_input.motd_complete ?
                         SV_STUB_LIVE_HANDOFF_SESSION_PASSWORD_STATUS :
                         "Character confirmed. Password is session only. Read MOTD; press a key.") :
                        (flow->attempt.login_input.motd_complete ?
                         SV_STUB_LIVE_HANDOFF_STATUS :
                         "Character confirmed. Read MOTD; press a key to continue.");
                    if (sv_login_interaction_complete(&flow->attempt.login_input) &&
                        flow->attempt.pregame.phase == SV_PREGAME_LIVE_HANDOFF) {
                        flow->attempt.result = 0;
                        if (!flow->attempt.handoff_reported) {
                            printf("SV startup phase=%s character=%s generation=%llu\n",
                                   sv_pregame_phase_name(flow->attempt.pregame.phase),
                                   flow->attempt.pregame.selected_character,
                                   (unsigned long long)flow->attempt.pregame.generation);
                            flow->attempt.handoff_reported = true;
                        }
                    }
                }
            } else flow->attempt.status = contact_status(flow->attempt.state, flow->attempt.rejection, &flow->endpoint, flow->attempt.connection_failure,
                                                           flow->attempt.failure_status,
                                                           sizeof(flow->attempt.failure_status));
        } else if (flow->attempt.play_started && flow->attempt.state == SV_SOCKET_CLOSED)
            flow->attempt.status = flow->preparation && flow->preparation->view.failure_reason[0]?
                flow->preparation->view.failure_reason:"Error reading play reply";
        else if(flow->preparation && flow->preparation->view.failure_reason[0])
            flow->attempt.status=flow->preparation->view.failure_reason;
        else flow->attempt.status = flow->attempt.session_error != SV_OK ?
            (flow->attempt.play_started && flow->attempt.login && sv_login_reason(flow->attempt.login)[0] ?
             sv_login_reason(flow->attempt.login) : sv_result_text(flow->attempt.session_error)) : contact_status(flow->attempt.state, flow->attempt.rejection, &flow->endpoint, flow->attempt.connection_failure,
                                                           flow->attempt.failure_status,
                                                           sizeof(flow->attempt.failure_status));
        sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
        if (flow->attempt.birth_started && birth_snapshot.active && flow->attempt.state >= SV_SOCKET_DNS_ERROR) {
            /* A disconnected caller cannot retain its temporary Guide surface. */
            if (flow->attempt.birth_placeholder_open) {
                flow->attempt.birth_placeholder_open = false;
                flow->attempt.temporary_surface_text = NULL;
                if (flow->attempt.birth_placeholder_drawn) flow->attempt.birth_placeholder.close_count++;
            }
            sv_birth_close(flow->attempt.birth, flow->generation);
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.pregame.authenticated && !flow->attempt.history) {
            flow->attempt.history=sv_history_create();
            SvFileResult loaded=flow->attempt.history?sv_history_begin(flow->attempt.history,
                flow->options.root,flow->account):SV_FILE_IO;
            if (loaded!=SV_FILE_OK && loaded!=SV_FILE_MISSING)
                fprintf(stderr,"SV history: account history unavailable or invalid\n");
        }
        if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.pregame.authenticated && !flow->attempt.save_started) {
            char key[SV_VAULT_KEY_CAPACITY];
            flow->attempt.save_started = true;
            flow->attempt.credential_save = SV_CREDENTIAL_SAVE_PENDING;
            if (sv_vault_key(key, flow->endpoint.host, strlen(flow->endpoint.host), flow->endpoint.port,
                             flow->options.account, strlen(flow->options.account)))
                flow->attempt.save = sv_vault_store(key, flow->generation, flow->options.password,
                                      strlen(flow->options.password));
            if (!flow->attempt.save) {
                flow->attempt.save_failed = true;
                flow->attempt.credential_save = SV_CREDENTIAL_SAVE_SESSION_ONLY;
                flow->attempt.save_message = "Account confirmed. Password is session only; choose a character.";
            }
        }
        if (flow->attempt.save) {
            SvVaultResult saved = sv_vault_poll(&flow->attempt.save, flow->generation, NULL, 0, NULL);
            if (saved == SV_VAULT_SAVED) {
                flow->attempt.credential_save = SV_CREDENTIAL_SAVE_SAVED;
                flow->attempt.save_message = "Account confirmed. Password saved; choose a character.";
            }
            else if (saved != SV_VAULT_PENDING) {
                flow->attempt.save_failed = true;
                flow->attempt.credential_save = SV_CREDENTIAL_SAVE_SESSION_ONLY;
                flow->attempt.save_message = "Account confirmed. Password is session only; choose a character.";
            }
        }
}
static void close_transfer_attempt(SvLoginFlow *flow)
{
    if (!flow->attempt.transfer.context) return;
    flow->attempt.transfer_result = sv_login_transfer_summary(flow->attempt.login);
    flow->attempt.lua_begin_result = sv_login_lua_begin_result(flow->attempt.login);
    sv_login_detach_transfer(flow->attempt.login);
    if (flow->options.transfer_factory.destroy)
        flow->options.transfer_factory.destroy(&flow->attempt.transfer);
}
void sv_login_flow_finish_tick(SvLoginFlow *flow)
{
        if (flow->attempt.state == SV_SOCKET_READY) {
            if (!flow->attempt.reached_ready) {
                const int *version = sv_contact_socket_version(flow->attempt.connection);
                const SvContactSetup *setup = sv_contact_socket_setup(flow->attempt.connection);
                printf("SV contact ready host=%s port=%u server=%d.%d.%d.%d.%d.%d races=%u classes=%u traits=%u motd=%u\n",
                       flow->endpoint.host, (unsigned)flow->endpoint.port, version[0], version[1],
                       version[2], version[3], version[4], version[5],
                       (unsigned)setup->race_count, (unsigned)setup->class_count,
                       (unsigned)setup->trait_count, (unsigned)setup->motd_size);
            }
            flow->attempt.reached_ready = true;
        } else if (flow->attempt.state >= SV_SOCKET_DNS_ERROR) {
            if (flow->attempt.result != 2) flow->attempt.result = 1;
            if (flow->attempt.pregame.phase != SV_PREGAME_FAILED) {
                if (flow->attempt.play_started && flow->attempt.session_error != SV_OK)
                    (void)sv_pregame_fail(&flow->attempt.pregame, flow->generation, flow->attempt.status);
                else if (flow->attempt.state == SV_SOCKET_REJECTED)
                    (void)sv_pregame_fail(&flow->attempt.pregame, flow->generation,
                                          contact_rejection_reason(flow->attempt.rejection));
                else
                    (void)sv_pregame_disconnect(&flow->attempt.pregame, flow->generation,
                                                flow->attempt.status);
            }
            if (!flow->attempt.reported_failure) fprintf(stderr, "SV contact failed: %s (status=%u)\n",
                                           flow->attempt.status, flow->attempt.rejection);
            flow->attempt.reported_failure = true;
            close_transfer_attempt(flow);
            if (flow->attempt.connection) { sv_contact_socket_stop(flow->attempt.connection); flow->attempt.connection = NULL; }
            if (flow->options.retry_login_disabled && flow->attempt.character_name_rejected) flow->attempt.finished = true;
            if (flow->options.frames) flow->attempt.finished = true;
        }
}
void sv_login_flow_end_attempt(SvLoginFlow *flow)
{
    if(flow->preparation && flow->preparation->moved)return;
    if(flow->preparation) { SvSessionPreparation *p=flow->preparation;flow->preparation=NULL;p->owner=NULL;sv_session_preparation_destroy(p); }
    SvBirthSnapshot birth_snapshot;
    sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
    if (flow->attempt.save) {
        sv_vault_cancel(&flow->attempt.save);
        if (flow->attempt.credential_save == SV_CREDENTIAL_SAVE_PENDING)
            flow->attempt.credential_save = SV_CREDENTIAL_SAVE_SESSION_ONLY;
    }
    if (flow->attempt.result == SV_CONTACT_SCENE_BACK)
        (void)sv_pregame_disconnect(&flow->attempt.pregame, flow->generation, "Returned to server selection.");
    close_transfer_attempt(flow);
    publish_outcome(&flow->attempt.pregame, flow->options.outcome, flow->prior_generation,
                    flow->attempt.credential_save, &flow->attempt.name_evidence, &flow->recovery,
                    flow->attempt.birth_started ? &birth_snapshot : NULL, &flow->attempt.birth_placeholder);
    if (flow->options.outcome) {
        flow->options.outcome->transfers = flow->attempt.transfer_result;
        flow->options.outcome->lua_begin = flow->attempt.lua_begin_result;
    }
    if (flow->attempt.birth_started) {
        /* Preserve the last interaction owner in the outcome, but report the
         * generation's actual lifetime after endpoint teardown. */
        sv_birth_close(flow->attempt.birth, flow->generation);
        sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
        if (flow->options.outcome) {
            flow->options.outcome->birth_active = birth_snapshot.active;
            flow->options.outcome->birth_closed = birth_snapshot.closed;
        }
    }
    if (flow->attempt.history) {
        if (sv_history_save(flow->attempt.history)!=SV_FILE_OK)
            fprintf(stderr,"SV history: could not save account history\n");
        sv_history_destroy(flow->attempt.history); flow->attempt.history=NULL;
    }
    const SvLoginCharacter *last=sv_login_selected_character(flow->attempt.login);
    if (last && flow->attempt.pregame.phase==SV_PREGAME_LIVE_HANDOFF)
        SDL_strlcpy(flow->previous_character,last->name,sizeof(flow->previous_character));
    sv_contact_socket_stop(flow->attempt.connection);
    flow->attempt.connection = NULL;

    sv_login_destroy(flow->attempt.login);
    flow->attempt.login = NULL;
    sv_birth_destroy(flow->attempt.birth);
    flow->attempt.birth = NULL;
    SDL_free(flow->attempt.wire.remaining);
    flow->attempt.wire.remaining = NULL;
    /* No game-effect consumer is active at the current live placeholder. Close
     * this generation explicitly, so its retained bytes cannot enter a retry. */
    if (flow->attempt.preferences) {
        size_t pending = sv_preferences_effect_count(flow->attempt.preferences);
        if (pending)
            fprintf(stderr, "SV character preferences generation=%llu: cancelling %zu retained effects at attempt close\n",
                (unsigned long long)flow->generation, pending);
        sv_preferences_destroy(flow->attempt.preferences);
        flow->attempt.preferences = NULL;
    }
}

SvLoginFlow *sv_login_flow_create(SvEndpointOptions options)
{
    if (options.transfer_factory.create && !options.transfer_factory.destroy) return NULL;
    SvLoginFlow *flow = SDL_calloc(1, sizeof(*flow));
    if (flow) {
        flow->options = options;
        flow->interactive_credentials = !options.account || !options.password;
        if (options.account) {
            SDL_strlcpy(flow->account, options.account, sizeof(flow->account));
            SDL_strlcpy(flow->initial_account, options.account, sizeof(flow->initial_account));
        }
        if (!flow->interactive_credentials) {
            SDL_strlcpy(flow->password, options.password, sizeof(flow->password));
        }
        flow->options.account = flow->account;
        flow->options.password = flow->password;
        sv_endpoint_begin(&flow->endpoint, (uint16_t)(options.port ? options.port : 18348));
    }
    return flow;
}
void sv_login_flow_destroy(SvLoginFlow *flow)
{
    if (!flow) return;
    if(flow->preparation && flow->preparation->moved)return;
    if (flow->attempt.connection || flow->attempt.birth || flow->attempt.preferences) sv_login_flow_end_attempt(flow);
    volatile char *password = flow->password;
    for (size_t i = 0; i < sizeof(flow->password); ++i) password[i] = 0;
    SDL_free(flow);
}
void sv_login_flow_view(const SvLoginFlow *flow, SvLoginFlowView *view)
{
    *view = (SvLoginFlowView){.generation = flow->generation,
        .revision = flow->attempt.pregame.revision, .phase = flow->attempt.pregame.phase,
        .status = flow->attempt.status, .failed = flow->attempt.state >= SV_SOCKET_DNS_ERROR,
        .retry_available = flow->attempt.state >= SV_SOCKET_DNS_ERROR && !flow->attempt.play_started &&
            !(flow->options.retry_login_disabled && flow->attempt.character_name_rejected),
        .text_input = flow->attempt.state == SV_SOCKET_READY && flow->attempt.login_input.ordinary_name_active,
        .has_login = flow->attempt.login && flow->attempt.connection};
    if (!view->has_login) return;
            sv_login_view_prepare(&view->characters, &flow->attempt.pregame);
            /* Keep creation geometry while the server accepts the name. */
            view->characters.ordinary_name_entry = flow->attempt.login_input.ordinary_name_active ||
                (flow->attempt.name_evidence.submitted && !flow->attempt.birth_started &&
                 sv_login_state(flow->attempt.login) == SV_LOGIN_WAIT_STATUS);
            view->characters.ordinary_creation_available =
                sv_login_interaction_ordinary_available(&flow->attempt.login_input);
            view->characters.exclusive_creation_available=sv_login_interaction_exclusive_modes(&flow->attempt.login_input)!=0;
            view->characters.creation_supported=flow->attempt.login_input.creation_supported;
            view->characters.first_run=flow->attempt.login_input.first_run;
            view->characters.pending=sv_login_state(flow->attempt.login)!=SV_LOGIN_OVERVIEW;
            sv_login_interaction_free_slots(&flow->attempt.login_input,view->characters.free_slots);
            view->characters.name_field=&flow->attempt.login_input.character_name;
            view->characters.character_name_draft = flow->attempt.login_input.ordinary_name_active ?
                flow->attempt.login_input.character_name.bytes :
                (sv_login_selected_character(flow->attempt.login)?sv_login_selected_character(flow->attempt.login)->name:flow->attempt.login_input.character_name.bytes);

    SvBirthSnapshot birth_snapshot;
    sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
    view->has_birth = flow->attempt.birth_started && birth_snapshot.active &&
        (sv_login_state(flow->attempt.login) == SV_LOGIN_SELECTED ||
         sv_login_state(flow->attempt.login) == SV_LOGIN_WAIT_PLAY);
    view->birth_pending = sv_login_state(flow->attempt.login) == SV_LOGIN_WAIT_PLAY;
    if (view->has_birth) sv_birth_view_prepare(&view->birth, flow->attempt.birth);
}

void sv_login_flow_login_result(SvLoginFlow *flow, SvLoginInputResult handled)
{
                if (handled == SV_LOGIN_INPUT_BACK) {
                    flow->attempt.result = SV_CONTACT_SCENE_BACK; flow->attempt.finished = true; return;
                }
                if (handled == SV_LOGIN_INPUT_QUIT) { flow->attempt.result = 0; flow->attempt.finished = true; return; }
                if (handled == SV_LOGIN_INPUT_NAME_STARTED) {
                    flow->input_epoch++;
                    flow->attempt.name_evidence.started = true;
                    flow->attempt.name_evidence.dedicated = flow->attempt.login_input.dedicated_creation;
                    flow->attempt.name_evidence.entry_generation = flow->attempt.pregame.generation;
                    flow->attempt.name_evidence.entry_revision = flow->attempt.pregame.revision;
                    return;
                }
                if (handled == SV_LOGIN_INPUT_NAME_CANCELLED) {

                    flow->input_epoch++;
                    flow->attempt.name_evidence.cancelled = true;
                    flow->attempt.name_evidence.cancel_generation = flow->attempt.pregame.generation;
                    flow->attempt.name_evidence.cancel_revision = flow->attempt.pregame.revision;
                    return;
                }
                if (handled == SV_LOGIN_INPUT_NAME_SUGGESTED) {
                    flow->attempt.name_evidence.random_suggested = true;
                    return;
                }
                if (handled == SV_LOGIN_INPUT_CREATION_UNSUPPORTED) {
                    SvResult applied = sv_pregame_creation_unsupported(
                        &flow->attempt.pregame, flow->generation);
                    if (applied != SV_OK) {
                        flow->attempt.session_error = applied;
                        flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                    }
                    return;
                }
                if (handled == SV_LOGIN_INPUT_NAME_SUBMITTED) {

                    flow->input_epoch++;
                    SvResult applied = sv_pregame_creation_submitted(&flow->attempt.pregame, flow->generation,
                        flow->attempt.login_input.dedicated_creation,
                        flow->attempt.login_input.dedicated_mode_flags);
                    if (applied == SV_OK)
                        applied = sv_pregame_sync_login(&flow->attempt.pregame, flow->generation, flow->attempt.login,
                            sv_contact_socket_setup(flow->attempt.connection),
                            flow->attempt.login_input.motd_complete);
                    if (applied != SV_OK) {
                        flow->attempt.session_error = applied;
                        flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                    } else {
                        flow->attempt.name_evidence.submitted = true;
                        flow->attempt.name_evidence.reincarnation_requested =
                            sv_login_reincarnation_requested(flow->attempt.login);
                        flow->attempt.name_evidence.submit_phase = flow->attempt.pregame.phase;
                        flow->attempt.name_evidence.input_truncated = flow->attempt.login_input.name_input_truncated;
                        flow->attempt.name_evidence.input_method_used = flow->attempt.login_input.name_input_method_used;
                        flow->attempt.name_evidence.keyboard_edited = flow->attempt.login_input.name_keyboard_edited;
                        flow->attempt.name_evidence.mouse_selected = flow->attempt.login_input.name_mouse_selected;
                        flow->attempt.name_evidence.paste_used = flow->attempt.login_input.name_paste_used;
                        flow->attempt.name_evidence.maximum_draft_bytes =
                            flow->attempt.login_input.name_maximum_draft_bytes;
                    }
                    return;
                }
                if (handled == SV_LOGIN_INPUT_HANDLED) {
                    SvResult applied = sv_pregame_sync_login(&flow->attempt.pregame, flow->generation, flow->attempt.login,
                        sv_contact_socket_setup(flow->attempt.connection), flow->attempt.login_input.motd_complete);
                    if (applied != SV_OK) {
                        flow->attempt.session_error = applied;
                        flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                    } else if (sv_login_interaction_complete(&flow->attempt.login_input) &&
                               flow->attempt.pregame.phase == SV_PREGAME_LIVE_HANDOFF) {
                        flow->attempt.result = 0;
                        if (!flow->attempt.handoff_reported) {
                            printf("SV startup phase=%s character=%s generation=%llu\n",
                                   sv_pregame_phase_name(flow->attempt.pregame.phase),
                                   flow->attempt.pregame.selected_character,
                                   (unsigned long long)flow->attempt.pregame.generation);
                            flow->attempt.handoff_reported = true;
                        }
                    }
                    return;
                }
}

void sv_login_flow_birth_result(SvLoginFlow *flow, SvBirthInputResult birth_result)
{
                    if (birth_result == SV_BIRTH_INPUT_BACK_TO_NAME ) {
                        /* The accepted name has moved the peer into PLAY.
                         * Reauthenticate a new generation before editing it;
                         * another LOGIN packet on this socket is not valid. */
                        flow->recovery.pending=true;
                        flow->recovery.dedicated=flow->attempt.login_input.dedicated_creation;
                        SDL_strlcpy(flow->recovery.draft,flow->attempt.login_input.character_name.bytes,
                                    sizeof(flow->recovery.draft));
                        (void)sv_pregame_disconnect(&flow->attempt.pregame,flow->generation,"Returning to character name.");
                        flow->attempt.result=SV_CONTACT_SCENE_EDIT_NAME;flow->attempt.finished=true;return;
                    }
                    if (birth_result == SV_BIRTH_INPUT_QUIT) {
                        flow->attempt.result = 0;
                        flow->attempt.finished = true;
                        return;
                    }
}

static SvLoginFlowStep return_to_servers(SvLoginFlow *flow)
{
    flow->endpoint.phase = SV_ENDPOINT_LIST;
    flow->endpoint.host[0] = 0;
    flow->endpoint.port = 0;
    flow->endpoint.invalid_address = false;
    memset(&flow->recovery, 0, sizeof(flow->recovery));
    flow->generation = flow->prior_generation = 0;
    return SV_LOGIN_FLOW_SERVERS;
}
SvLoginFlowStep sv_login_flow_selected(SvLoginFlow *flow)
{
    if (flow->options.protocol_override)
        flow->endpoint.protocol = flow->options.protocol_override;
    memset(&flow->recovery, 0, sizeof(flow->recovery));
    flow->generation = flow->prior_generation = 0;
    if (flow->interactive_credentials) {
        SDL_strlcpy(flow->account, flow->initial_account, sizeof(flow->account));
        memset(flow->password, 0, sizeof(flow->password));
        return SV_LOGIN_FLOW_CREDENTIALS;
    }
    return SV_LOGIN_FLOW_CONNECT;
}
SvLoginFlowStep sv_login_flow_credentials_result(SvLoginFlow *flow, SvLoginCredentialResult result)
{
    if (result == SV_LOGIN_CREDENTIAL_ACCEPTED) return SV_LOGIN_FLOW_CONNECT;
    if (result == SV_LOGIN_CREDENTIAL_BACK) return return_to_servers(flow);
    flow->attempt.result = result == SV_LOGIN_CREDENTIAL_ERROR ? 1 : 0;
    return SV_LOGIN_FLOW_FINISHED;
}
SvLoginFlowStep sv_login_flow_next(SvLoginFlow *flow)
{
    if (flow->attempt.result == SV_CONTACT_SCENE_BACK) return return_to_servers(flow);
    if (flow->attempt.result != 2 && flow->attempt.result != SV_CONTACT_SCENE_EDIT_NAME)
        return SV_LOGIN_FLOW_FINISHED;
    flow->prior_generation = flow->generation;
    if (flow->interactive_credentials && flow->attempt.result != SV_CONTACT_SCENE_EDIT_NAME) {
        volatile char *password = flow->password;
        for (size_t i = 0; i < sizeof(flow->password); ++i) password[i] = 0;
        return SV_LOGIN_FLOW_CREDENTIALS;
    }
    return SV_LOGIN_FLOW_CONNECT;
}
void sv_login_flow_command(SvLoginFlow *flow, SvLoginFlowCommand command)
{
    switch (command) {
    case SV_LOGIN_FLOW_EXIT: flow->attempt.result = 0; flow->attempt.finished = true; break;
    case SV_LOGIN_FLOW_RETRY:
        if (flow->attempt.state >= SV_SOCKET_DNS_ERROR && !flow->attempt.play_started &&
            !(flow->options.retry_login_disabled && flow->attempt.character_name_rejected)) {
            flow->attempt.result = 2; flow->attempt.finished = true;
        }
        break;
    case SV_LOGIN_FLOW_BACK:
        flow->attempt.result = SV_CONTACT_SCENE_BACK; flow->attempt.finished = true; break;
    case SV_LOGIN_FLOW_EDIT_NAME:
        sv_login_flow_birth_result(flow, SV_BIRTH_INPUT_BACK_TO_NAME); break;
    }
}

const SvSessionPreparation *sv_login_flow_preparation(const SvLoginFlow *flow)
{ return flow?flow->preparation:NULL; }
SvSessionPreparation *sv_login_flow_take_preparation(SvLoginFlow *flow)
{
    if(!flow || !flow->preparation || flow->preparation->moved ||
       flow->attempt.state!=SV_SOCKET_READY || !flow->attempt.connection ||
       !flow->preparation->view.startup_complete)return NULL;
    SvSessionPreparation *p=flow->preparation;p->moved=true;
    SvBirthSnapshot birth_snapshot;
    sv_birth_snapshot_prepare(&birth_snapshot,flow->attempt.birth);
    publish_outcome(&flow->attempt.pregame,flow->options.outcome,flow->prior_generation,
        flow->attempt.credential_save,&flow->attempt.name_evidence,&flow->recovery,
        flow->attempt.birth_started?&birth_snapshot:NULL,&flow->attempt.birth_placeholder);
    if(flow->options.outcome) {
        flow->options.outcome->transfers=sv_login_transfer_summary(flow->attempt.login);
        flow->options.outcome->lua_begin=sv_login_lua_begin_result(flow->attempt.login);
    }
    /* No later authentication operation belongs to this prepared connection. */
    memset(flow->password,0,sizeof(flow->password));
    flow->options.password=NULL;flow->options.account=NULL;flow->options.invocation=NULL;
    flow->options.preferences=p->application;
    flow->options.settings=sv_preferences_options(p->application);
    flow->options.audio=&p->view.audio;
    flow->options.root=p->root;flow->options.library=p->library;flow->profile_root=p->root;
    flow->options.map_font=&p->resources.map_font;
    flow->options.graphics_tiles=p->resources.effective_tiles;
    flow->options.graphics_owner=p->graphics;
    flow->options.client_config=NULL;
    flow->options.effective_text_font=p->resources.effective_text_font;
    flow->options.server=flow->options.server_list=flow->options.metaserver=NULL;
    flow->options.source_poll=NULL;flow->options.source_context=NULL;
    flow->options.selected_endpoint=NULL;
    flow->options.real_name=flow->options.character=NULL;
    flow->options.startup_argv=NULL;flow->options.startup_argc=0;
    flow->options.shell=(SvLoginShellOptions){0};flow->font_resource=NULL;
    flow->options.outcome=NULL;flow->options.preparation=NULL;
    return p;
}
