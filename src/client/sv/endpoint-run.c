#include "endpoint-run.h"
#include "app.h"
#include "input/native-endpoint.h"
#include "input/credentials.h"
#include "input/metaserver.h"
#include "input/login-interaction.h"
#include "input/native-login.h"
#include "ui/endpoint-scene.h"
#include "protocol/contact-socket.h"
#include "protocol/login.h"
#include "protocol/login-identity.h"
#include "session/login-view.h"
#include "session/pregame.h"
#include "../../common/pack.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

static void wipe_password(char *password, size_t size)
{
    volatile char *bytes = password;
    while (size--) *bytes++ = 0;
}

/* The endpoint scene only drives the private input interaction and drawing. */
static int enter_credentials(SDL_Renderer *renderer, SvFont *font, SvEndpoint *endpoint,
                             SvEndpointInput *input, int frame_limit,
                             char account[80], char password[80])
{
    SvCredentialInput credentials;
    sv_credentials_begin(&credentials, endpoint, account, password);
    int outcome = 0, frames = 0;
    for (;;) {
        char status[180];
        sv_credentials_poll(&credentials);
        sv_credentials_status(&credentials, status);
        input->contact_status = status;
        input->text_error = credentials.text_error;
        input->clipboard_unavailable = credentials.clipboard_unavailable;
        if (!sv_endpoint_draw(renderer, font, endpoint, input)) {
            outcome = -1;
            break;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            SvCredentialAction action = sv_credentials_event(&credentials, &event);
            if (action != SV_CREDENTIAL_EDITING) {
                outcome = action == SV_CREDENTIAL_ACCEPTED ? 1 : 0;
                goto done;
            }
        }
        if (frame_limit && ++frames >= frame_limit) break;
        SDL_Delay(16);
    }
done:
    sv_credentials_end(&credentials);
    return outcome;
}

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
        SDL_snprintf(status, status_size, "%s Escape exits.",
                     contact_rejection_reason(rejection));
        return status;
    case SV_SOCKET_VERIFY_ERROR: return "Verification failed. R retries; Escape exits.";
    case SV_SOCKET_SETUP_ERROR: return "Server setup failed. R retries; Escape exits.";
    }
    return "Contact stopped.";
}

typedef struct {
    unsigned char sending[SV_CONTACT_OUTPUT_CAPACITY];
    size_t size, sent;
    unsigned char *remaining;
    size_t remaining_size, remaining_at;
} SvSessionWire;

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
    unsigned char incoming[4096];
    for (int i = 0; i < 4; ++i) {
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
        } else if (sv_login_state(login) == SV_LOGIN_REJECTED) break;
        else received = sv_contact_socket_read(connection, incoming, sizeof(incoming));
        if (received.result == SV_WAITING) break;
        if (received.result != SV_OK) return SV_SOCKET_CLOSED;
        *failure = sv_login_receive(login, incoming, received.size);
        if (*failure != SV_OK)
            return SV_SOCKET_PROTOCOL_ERROR;
    }
    return SV_SOCKET_READY;
}

static void publish_outcome(const SvPregame *pregame, SvEndpointOutcome *outcome,
                            uint64_t prior_generation,
                            SvCredentialSaveState credential_save)
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
                                   .credential_save = credential_save};
    memcpy(outcome->server_flags, pregame->server_flags,
           sizeof(outcome->server_flags));
    memcpy(outcome->selected_character, pregame->selected_character,
           sizeof(outcome->selected_character));
    memcpy(outcome->reason, pregame->reason, sizeof(outcome->reason));
}

static int run_contact(SDL_Window *window, SDL_Renderer *renderer, SvFont *font,
                       SvEndpoint *endpoint, SvEndpointInput *input, SvEndpointOptions options,
                       const char *profile_root, uint64_t generation,
                       uint64_t prior_generation)
{
    unsigned char iaddr[6];
    if (!sv_login_identity(profile_root, iaddr)) {
        SDL_SetError("Cannot derive versioned login identity");
        return 1;
    }
    SvContactIdentity identity = {options.real_name ? options.real_name : "PLAYER",
                                  options.account, "localhost", options.password};
    if (endpoint->protocol >= 2 && strchr(options.password, '*')) {
        SDL_SetError("Password contains '*' and cannot round-trip through this server protocol");
        return 1;
    }
    SvContactSocket *connection = sv_contact_socket_start(endpoint->host, endpoint->port,
                                                           endpoint->protocol, &identity);
    if (!connection) {
        SDL_SetError("Cannot start contact (invalid identity or unavailable socket)");
        return 1;
    }
    SvSocketState state = SV_SOCKET_RESOLVING;
    SvLogin *login = NULL;
    SvLoginInteraction login_input = {0};
    SvLoginView login_view = {0};
    SvPregame pregame;
    SvVaultRequest *save = NULL;
    sv_pregame_begin(&pregame, generation);
    SvSessionWire wire = {0};
    SvResult session_error = SV_OK;
    unsigned rejection = 0;
    int frames = 0, result = 1;
    bool quit = false, reached_ready = false, reported_failure = false;
    bool save_started = false, save_failed = false;
    SvCredentialSaveState credential_save = SV_CREDENTIAL_SAVE_NOT_STARTED;
    bool selected_reported = false;
    bool handoff_reported = false;
    const char *save_message = NULL;
    char failure_status[384] = {0};
    while (!quit) {
        if (connection) {
            state = sv_contact_socket_poll(connection);
            rejection = sv_contact_socket_rejection(connection);
        }
        if (state == SV_SOCKET_READY && !login) {
            login = sv_login_create(sv_contact_socket_version(connection), iaddr);
            if (!login) { session_error = SV_NO_MEMORY; state = SV_SOCKET_PROTOCOL_ERROR; }
            else {
                if (sv_pregame_contact_ready(&pregame, generation) != SV_OK) {
                    session_error = SV_INVALID;
                    state = SV_SOCKET_PROTOCOL_ERROR;
                    continue;
                }
                sv_login_interaction_begin(&login_input, login, options.character,
                                           options.skip_motd);
                if ((session_error = handoff_contact_bytes(connection, &wire)) != SV_OK)
                    state = SV_SOCKET_PROTOCOL_ERROR;
            }
        }
        if (state == SV_SOCKET_READY && login)
            state = pump_login(connection, login, &wire, &session_error);
        if (state == SV_SOCKET_READY && login)
            sv_native_login_clear_previous_keys(sv_login_interaction_sync(&login_input));
        if (state == SV_SOCKET_READY && login) {
            SvResult applied = sv_pregame_sync_login(&pregame, generation, login,
                sv_contact_socket_setup(connection), login_input.motd_complete);
            if (applied != SV_OK) { session_error = applied; state = SV_SOCKET_PROTOCOL_ERROR; }
        }
        if (login && sv_login_state(login) == SV_LOGIN_REJECTED) {
            SDL_snprintf(failure_status, sizeof(failure_status), "%s  Press R to retry or Escape to exit.",
                         sv_login_reason(login));
            input->contact_status = failure_status;
            state = SV_SOCKET_REJECTED;
        } else if (state == SV_SOCKET_READY && login) {
            SvLoginState phase = sv_login_state(login);
            if (phase == SV_LOGIN_OVERVIEW) {
                input->contact_status = save_message ? save_message :
                    "Account confirmed. Select an existing character by letter; Q quits.";
            } else if (phase == SV_LOGIN_WAIT_STATUS)
                input->contact_status = "Waiting for selected character status...";
            else if (phase == SV_LOGIN_SELECTED) {
                if (!selected_reported) {
                    puts("SV character selected; peer MOTD acknowledgement pending");
                    selected_reported = true;
                }
                input->contact_status = save_failed ?
                    (login_input.motd_complete ?
                     "MOTD complete. Password is session only. Waiting for live-session handoff." :
                     "Character confirmed. Password is session only. Read MOTD; press a key.") :
                    (login_input.motd_complete ?
                     "MOTD complete. Waiting for live-session handoff; Escape exits." :
                     "Character confirmed. Read MOTD; press a key to continue.");
                if (sv_login_interaction_complete(&login_input) &&
                    pregame.phase == SV_PREGAME_LIVE_HANDOFF) {
                    result = 0;
                    if (!handoff_reported) {
                        printf("SV startup phase=%s character=%s generation=%llu\n",
                               sv_pregame_phase_name(pregame.phase),
                               pregame.selected_character,
                               (unsigned long long)pregame.generation);
                        handoff_reported = true;
                    }
                }
            } else input->contact_status = contact_status(state, rejection,
                                                           failure_status,
                                                           sizeof(failure_status));
        } else input->contact_status = session_error != SV_OK ?
            sv_result_text(session_error) : contact_status(state, rejection,
                                                           failure_status,
                                                           sizeof(failure_status));
        if (state == SV_SOCKET_READY && pregame.authenticated && !save_started) {
            char key[SV_VAULT_KEY_CAPACITY];
            save_started = true;
            credential_save = SV_CREDENTIAL_SAVE_PENDING;
            if (sv_vault_key(key, endpoint->host, strlen(endpoint->host), endpoint->port,
                             options.account, strlen(options.account)))
                save = sv_vault_store(key, generation, options.password,
                                      strlen(options.password));
            if (!save) {
                save_failed = true;
                credential_save = SV_CREDENTIAL_SAVE_SESSION_ONLY;
                save_message = "Account confirmed. Password is session only; choose a character.";
            }
        }
        if (save) {
            SvVaultResult saved = sv_vault_poll(&save, generation, NULL, 0, NULL);
            if (saved == SV_VAULT_SAVED) {
                credential_save = SV_CREDENTIAL_SAVE_SAVED;
                save_message = "Account confirmed. Password saved; choose a character.";
            }
            else if (saved != SV_VAULT_PENDING) {
                save_failed = true;
                credential_save = SV_CREDENTIAL_SAVE_SESSION_ONLY;
                save_message = "Account confirmed. Password is session only; choose a character.";
            }
        }
        input->login_view = NULL;
        if (login && connection) {
            sv_login_view_prepare(&login_view, &pregame);
            input->login_view = &login_view;
        }
        if (!sv_endpoint_draw(renderer, font, endpoint, input)) break;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                quit = true;
                break;
            }
            if (state >= SV_SOCKET_DNS_ERROR && event.type == SDL_EVENT_KEY_DOWN &&
                !event.key.repeat && event.key.key == SDLK_R) {
                result = 2;
                quit = true;
                break;
            }
            if (state == SV_SOCKET_READY && login) {
                SvLoginCommand command;
                SvLoginInputResult handled = sv_native_login_command(&event, &command) ?
                    sv_login_interaction_command(&login_input, command) :
                    SV_LOGIN_INPUT_IGNORED;
                if (handled == SV_LOGIN_INPUT_QUIT) { result = 0; quit = true; break; }
                if (handled == SV_LOGIN_INPUT_HANDLED) {
                    SvResult applied = sv_pregame_sync_login(&pregame, generation, login,
                        sv_contact_socket_setup(connection), login_input.motd_complete);
                    if (applied != SV_OK) {
                        session_error = applied;
                        state = SV_SOCKET_PROTOCOL_ERROR;
                    } else if (sv_login_interaction_complete(&login_input) &&
                               pregame.phase == SV_PREGAME_LIVE_HANDOFF) {
                        result = 0;
                        if (!handoff_reported) {
                            printf("SV startup phase=%s character=%s generation=%llu\n",
                                   sv_pregame_phase_name(pregame.phase),
                                   pregame.selected_character,
                                   (unsigned long long)pregame.generation);
                            handoff_reported = true;
                        }
                    }
                    continue;
                }
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                quit = true;
                break;
            }
        }
        if (state == SV_SOCKET_READY) {
            if (!reached_ready) {
                const int *version = sv_contact_socket_version(connection);
                const SvContactSetup *setup = sv_contact_socket_setup(connection);
                printf("SV contact ready host=%s port=%u server=%d.%d.%d.%d.%d.%d races=%u classes=%u traits=%u motd=%u\n",
                       endpoint->host, (unsigned)endpoint->port, version[0], version[1],
                       version[2], version[3], version[4], version[5],
                       (unsigned)setup->race_count, (unsigned)setup->class_count,
                       (unsigned)setup->trait_count, (unsigned)setup->motd_size);
            }
            reached_ready = true;
        } else if (state >= SV_SOCKET_DNS_ERROR) {
            if (result != 2) result = 1;
            if (pregame.phase != SV_PREGAME_FAILED) {
                if (state == SV_SOCKET_REJECTED)
                    (void)sv_pregame_fail(&pregame, generation,
                                          contact_rejection_reason(rejection));
                else
                    (void)sv_pregame_disconnect(&pregame, generation,
                                                input->contact_status);
            }
            if (!reported_failure) fprintf(stderr, "SV contact failed: %s (status=%u)\n",
                                           input->contact_status, rejection);
            reported_failure = true;
            if (connection) { sv_contact_socket_stop(connection); connection = NULL; }
            if (options.frames) break;
        }
        if (options.frames && ++frames >= options.frames) break;
        SDL_Delay(16);
    }
    if (save) {
        sv_vault_cancel(&save);
        if (credential_save == SV_CREDENTIAL_SAVE_PENDING)
            credential_save = SV_CREDENTIAL_SAVE_SESSION_ONLY;
    }
    publish_outcome(&pregame, options.outcome, prior_generation,
                    credential_save);
    sv_contact_socket_stop(connection);
    sv_login_destroy(login);
    SDL_free(wire.remaining);
    input->login_view = NULL;
    (void)window;
    return result;
}

static bool load_list(SvEndpoint *endpoint, const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) return SDL_SetError("Cannot read selected server list");
    SvServer servers[SV_SERVER_COUNT] = {{0}};
    size_t count = 0;
    char line[512];
    bool ok = true;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (count == SV_SERVER_COUNT || !strchr(line, '\n')) { ok = false; break; }
        unsigned port;
        int parsed = sscanf(line, "%79s %u %95[^\n]", servers[count].host,
                            &port, servers[count].label);
        if (parsed < 2 || port < 1 || port > 65535) { ok = false; break; }
        servers[count].port = (uint16_t)port;
        servers[count].ping_ms = -1;
        ++count;
    }
    if (ferror(file)) ok = false;
    fclose(file);
    if (!ok || !sv_endpoint_servers(endpoint, servers, count))
        return SDL_SetError("Invalid server list row or capacity");
    return true;
}

int sv_endpoint_run(SvEndpointOptions options)
{
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint, (uint16_t)(options.port ? options.port : 18348));
    if (options.server && !sv_endpoint_argument(&endpoint, options.server)) {
        fprintf(stderr, "Invalid server address\n");
        return 2;
    }
    if (options.server_list && !load_list(&endpoint, options.server_list)) {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 2;
    }
    const char *root = options.root;
    char *owned_root = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SvFont *font = NULL;
    SvMetaserver *provider = NULL;
    int result = 1;
    if (!root) {
        root = SDL_getenv("TOMENET_SDL3_USER_PATH");
        if (!root || !*root) {
            owned_root = SDL_GetPrefPath("TomenetGame", "tomenet");
            root = owned_root;
        }
    }
    if (!root || !SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) goto done;
    if (!options.server && !options.server_list && !options.source_poll) {
        SvEndpoint meta;
        sv_endpoint_begin(&meta, 8801);
        if (!sv_endpoint_argument(&meta, options.metaserver ? options.metaserver : "meta.tomenet.eu")) {
            SDL_SetError("Invalid metaserver address"); goto done;
        }
        provider = sv_metaserver_start(meta.host, meta.port);
        if (!provider) goto done;
    }
    const char *library = options.library ? options.library : SDL_getenv("TOMENET_PATH");
    char adjacent[4096];
    if (!library || !*library) {
        const char *base = SDL_GetBasePath();
        if (!base || SDL_snprintf(adjacent,sizeof(adjacent),"%s/lib",base) >= (int)sizeof(adjacent)) goto done;
        library = adjacent;
    }
    font = sv_font_open(root, library);
    if (!font) goto done;
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (!options.windowed) flags |= SDL_WINDOW_FULLSCREEN;
    window = SDL_CreateWindow("TomeNET SV - Server",options.width,options.height,flags);
    if (!window) goto done;
    float window_scale = SDL_GetWindowDisplayScale(window) / SDL_GetWindowPixelDensity(window);
    if (window_scale <= 0 || !SDL_SetWindowMinimumSize(window,
            (int)SDL_ceilf(1024 * window_scale), (int)SDL_ceilf(768 * window_scale))) goto done;
    if (options.windowed && !SDL_SetWindowSize(window,
            (int)SDL_roundf(options.width * window_scale),
            (int)SDL_roundf(options.height * window_scale))) goto done;
    renderer = SDL_CreateRenderer(window,NULL);
    if (!renderer) goto done;
    int window_count = 0;
    SDL_Window **windows = SDL_GetWindows(&window_count);
    SDL_free(windows);
    if (window_count != 1) { SDL_SetError("SV requires exactly one system window"); goto done; }
    SvEndpointInput input;
    sv_endpoint_input_begin(&input);
    if (!sv_endpoint_draw(renderer,font,&endpoint,&input) ||
        !sv_endpoint_render(renderer,font,&endpoint,&input)) goto done;
    SDL_Rect sample = {0,0,1,1};
    SDL_Surface *ready = SDL_RenderReadPixels(renderer,&sample);
    if (!ready) goto done;
    SDL_DestroySurface(ready);
    if (!SDL_RenderPresent(renderer) || !SDL_StartTextInput(window)) goto done;
    sv_endpoint_input_begin(&input);
    int frames = 0;
    bool quit = endpoint.phase == SV_ENDPOINT_SELECTED;
    while (!quit) {
        bool failed = false;
        bool changed = options.source_poll ? options.source_poll(options.source_context, &endpoint, &failed) :
                       provider && sv_metaserver_poll(provider, &endpoint, &failed);
        if (changed && failed &&
            endpoint.phase == SV_ENDPOINT_LIST) sv_endpoint_manual(&endpoint);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                quit = true; break;
            }
            (void)sv_endpoint_event(&input,&endpoint,&event,NULL,NULL);
            if (endpoint.phase == SV_ENDPOINT_CANCELLED ||
                endpoint.phase == SV_ENDPOINT_SELECTED) { quit = true; break; }
        }
        if (quit) break;
        if (!sv_endpoint_draw(renderer,font,&endpoint,&input)) goto done;
        if (options.frames && ++frames >= options.frames) break;
        SDL_Delay(16);
    }
    if (endpoint.phase == SV_ENDPOINT_SELECTED) {
        printf("SV endpoint selected host=%s port=%u\n",
               endpoint.host,(unsigned)endpoint.port);
        if (!options.selected_endpoint) {
            char account[80] = {0}, password[80] = {0};
            int entered = 1;
            bool interactive_credentials = !options.account || !options.password;
            if (interactive_credentials) {
                entered = enter_credentials(renderer, font, &endpoint, &input,
                                            options.frames, account, password);
                options.account = account;
                options.password = password;
            }
            uint64_t generation = SDL_GetTicksNS();
            if (!generation) generation = 1;
            uint64_t prior_generation = 0;
            while (entered > 0) {
                result = run_contact(window, renderer, font, &endpoint, &input,
                                     options, root, generation, prior_generation);
                if (result != 2) break;
                prior_generation = generation;
                generation = SDL_GetTicksNS();
                if (!generation || generation == prior_generation)
                    generation = prior_generation + 1;
                if (!generation) generation = 1;
                input.login_view = NULL;
                input.contact_status = "Retrying account authentication.";
                if (interactive_credentials) {
                    wipe_password(password, sizeof(password));
                    entered = enter_credentials(renderer, font, &endpoint, &input,
                                                options.frames, account, password);
                }
            }
            if (entered <= 0) result = entered < 0 ? 1 : 0;
            wipe_password(password, sizeof(password));
            goto done;
        }
    }
    else if (endpoint.phase == SV_ENDPOINT_CANCELLED)
        puts("SV endpoint selection cancelled; no contact attempted");
    else puts("SV endpoint selection incomplete; no contact attempted");
    if (options.selected_endpoint) {
        *options.selected_endpoint = (SvEndpointChoice){.phase = endpoint.phase,
            .port = endpoint.port, .protocol = endpoint.protocol};
        memcpy(options.selected_endpoint->host, endpoint.host, sizeof(endpoint.host));
    }
    result = 0;
done:
    sv_metaserver_stop(provider);
    if (result) fprintf(stderr,"SV endpoint startup failed: %s\n",SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    sv_font_close(font);
    TTF_Quit();
    SDL_Quit();
    SDL_free(owned_root);
    return result;
}
