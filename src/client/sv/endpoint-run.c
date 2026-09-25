#include "endpoint-run.h"
#include "app.h"
#include "input/native-endpoint.h"
#include "input/metaserver.h"
#include "input/text-field.h"
#include "ui/endpoint-scene.h"
#include "protocol/contact-socket.h"
#include "profile.h"
#include "options.h"
#include "resource.h"
#include "../../common/pack.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

static void wipe_password(char *password, size_t size)
{
    volatile char *bytes = password;
    while (size--) *bytes++ = 0;
}

/* The selected endpoint remains visible while credentials are entered. */
static int enter_credentials(SDL_Renderer *renderer, SvFont *font, SvEndpoint *endpoint,
                             SvEndpointInput *input, int frame_limit,
                             char account[80], char password[80], float user_scale)
{
    int stage = 0, frames = 0;
    bool incompatible_password = false;
    char status[180];
    for (;;) {
        char mask[80];
        size_t password_length = strlen(password);
        memset(mask, '*', password_length);
        mask[password_length] = 0;
        SDL_snprintf(status, sizeof(status), incompatible_password ?
            "Password contains '*' (unsupported by server). Edit it; Escape cancels." : stage == 0 ?
            "Account: %s  (Enter continues, Escape cancels)" :
            "Password: %s  (Enter connects, Escape cancels)",
            stage == 0 ? account : mask);
        input->contact_status = status;
        if (!sv_endpoint_draw(renderer, font, endpoint, input, user_scale)) return -1;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                return 0;
            if (event.type == SDL_EVENT_TEXT_INPUT) {
                char *field = stage ? password : account;
                SvTextField editor;
                sv_text_begin(&editor, field, 79, true);
                (void)sv_text_select(&editor, editor.length, editor.length);
                input->text_error = sv_contact_field_insert(&editor, event.text.text);
                memcpy(field, editor.bytes, editor.length + 1);
                incompatible_password = false;
                wipe_password((char *)&editor, sizeof(editor));
            } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                if (event.key.key == SDLK_ESCAPE) return 0;
                char *field = stage ? password : account;
                size_t length = strlen(field);
                if (event.key.key == SDLK_BACKSPACE && length) {
                    field[length - 1] = 0;
                    incompatible_password = false;
                    input->text_error = SV_TEXT_OK;
                }
                else if ((event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER) && length) {
                    if (input->text_error == SV_TEXT_ENCODING_ERROR) continue;
                    if (stage) {
                        if (endpoint->protocol >= 2 && strchr(password, '*')) {
                            incompatible_password = true;
                            continue;
                        }
                        return 1;
                    }
                    stage = 1;
                    input->text_error = SV_TEXT_OK;
                }
            }
        }
        if (frame_limit && ++frames >= frame_limit) return 0;
        SDL_Delay(16);
    }
}

static const char *contact_status(SvSocketState state, unsigned rejection)
{
    switch (state) {
    case SV_SOCKET_RESOLVING: return "Resolving server address...";
    case SV_SOCKET_CONNECTING: return "Connecting to server...";
    case SV_SOCKET_NEGOTIATING: return "Negotiating version and setup...";
    case SV_SOCKET_READY: return "Contact established. Close this window.";
    case SV_SOCKET_DNS_ERROR: return "Cannot resolve server address. Escape exits.";
    case SV_SOCKET_CONNECT_ERROR: return "Cannot open server socket. Escape exits.";
    case SV_SOCKET_TIMEOUT: return "Server timed out. Escape exits.";
    case SV_SOCKET_CLOSED: return "Server closed the connection. Escape exits.";
    case SV_SOCKET_PROTOCOL_ERROR: return "Invalid contact or network packet. Escape exits.";
    case SV_SOCKET_REJECTED:
        switch (rejection) {
        case E_VERSION_OLD: return "Server rejected contact: client version too old. Escape exits.";
        case E_VERSION_UNKNOWN: return "Server rejected contact: incompatible version. Escape exits.";
        case E_GAME_FULL: return "Server rejected contact: game is full. Escape exits.";
        case E_TWO_PLAYERS: return "Server rejected contact: another character is online. Escape exits.";
        case E_PASSWORD: return "Server rejected contact: invalid password. Escape exits.";
        case E_IN_USE_DUP: return "Server rejected contact: duplicate login. Escape exits.";
        case E_LETTER: return "Server rejected contact: invalid account name. Escape exits.";
        case E_IN_USE: return "Server rejected contact: account in use from another address. Escape exits.";
        case E_SOCKET: return "Server rejected contact: server socket error. Escape exits.";
        case E_INVAL: return "Server rejected contact: invalid identity. Escape exits.";
        case E_INVITE: return "Server rejected contact: members only. Escape exits.";
        case E_BANNED: return "Server rejected contact: temporarily banned. Escape exits.";
        case E_LENGTH: return "Server rejected contact: account name too short. Escape exits.";
        case E_IN_USE_PC: return "Server rejected contact: account in use on this PC. Escape exits.";
        case E_CLOSED: return "Server rejected contact: server closing. Escape exits.";
        }
        return "Server rejected contact. Escape exits.";
    case SV_SOCKET_VERIFY_ERROR: return "Verification failed. Escape exits.";
    case SV_SOCKET_SETUP_ERROR: return "Server setup failed. Escape exits.";
    }
    return "Contact stopped.";
}

typedef struct {
    unsigned char sending[SV_PROTOCOL_CAPACITY];
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

static SvSocketState pump_session(SvContactSocket *connection, SvApp *app,
                                  uint64_t generation, SvSessionWire *wire,
                                  SvResult *failure)
{
    uint64_t now_ms = SDL_GetTicks();
    *failure = sv_app_frame(app, generation, now_ms);
    if (*failure != SV_OK) return SV_SOCKET_PROTOCOL_ERROR;
    if (wire->sent == wire->size) {
        (void)sv_app_keepalive(app, generation, now_ms,
                               sv_contact_socket_last_sent(connection));
        SvOutput output = sv_app_take_output(app, generation, wire->sending,
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
        size_t capacity = sv_app_receive_capacity(app);
        if (!capacity) break;
        if (capacity > sizeof(incoming)) capacity = sizeof(incoming);
        SvOutput received;
        if (wire->remaining_at < wire->remaining_size) {
            size_t count = wire->remaining_size - wire->remaining_at;
            if (count > capacity) count = capacity;
            memcpy(incoming, wire->remaining + wire->remaining_at, count);
            wire->remaining_at += count;
            received = (SvOutput){SV_OK, count};
            if (wire->remaining_at == wire->remaining_size) {
                SDL_free(wire->remaining);
                wire->remaining = NULL;
                wire->remaining_size = wire->remaining_at = 0;
            }
        } else received = sv_contact_socket_read(connection, incoming, capacity);
        if (received.result == SV_WAITING) break;
        if (received.result != SV_OK) return SV_SOCKET_CLOSED;
        *failure = sv_app_receive(app, generation, incoming, received.size);
        if (*failure != SV_OK)
            return SV_SOCKET_PROTOCOL_ERROR;
    }
    SvStep step = sv_app_step(app, 64);
    if (step.result != SV_OK && step.result != SV_WAITING &&
        step.result != SV_BACKPRESSURE && step.result != SV_RECOVERED) {
        *failure = step.result;
        return SV_SOCKET_PROTOCOL_ERROR;
    }
    return SV_SOCKET_READY;
}

static int run_contact(SDL_Window *window, SDL_Renderer *renderer, SvFont *font,
                       SvEndpoint *endpoint, SvEndpointInput *input, SvEndpointOptions options,
                       float user_scale)
{
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
    SvApp *app = NULL;
    uint64_t generation = 0;
    SvSessionWire wire = {0};
    SvResult session_error = SV_OK;
    unsigned rejection = 0;
    int frames = 0, result = 1;
    bool quit = false, reached_ready = false, reported_failure = false;
    uint64_t pause_since_ns = 0, observed_pause = 0;
    uint64_t presented_flush = 0;
    while (!quit) {
        if (connection) {
            state = sv_contact_socket_poll(connection);
            rejection = sv_contact_socket_rejection(connection);
        }
        if (state == SV_SOCKET_READY && !app) {
            const SvContactSetup *setup = sv_contact_socket_setup(connection);
            app = sv_app_create((SvAlertSink){0});
            if (!setup || !app) {
                session_error = setup ? SV_NO_MEMORY : SV_INVALID;
                state = SV_SOCKET_PROTOCOL_ERROR;
            } else if ((session_error = sv_app_open(app,
                           sv_contact_socket_version(connection))) != SV_OK)
                state = SV_SOCKET_PROTOCOL_ERROR;
            else {
                generation = sv_app_view(app).generation;
                session_error = sv_app_set_character_setup(app, generation, setup);
                if (session_error == SV_OK)
                    session_error = handoff_contact_bytes(connection, &wire);
                if (session_error != SV_OK)
                    state = SV_SOCKET_PROTOCOL_ERROR;
            }
        }
        if (state == SV_SOCKET_READY && app)
            state = pump_session(connection, app, generation, &wire, &session_error);
        if (state == SV_SOCKET_READY && app) {
            SvAppView view = sv_app_view(app);
            if (view.paused && view.pause_sequence != observed_pause) {
                observed_pause = view.pause_sequence;
                pause_since_ns = SDL_GetTicksNS();
            }
            input->contact_status = view.paused ?
                "Server paused. Press a fresh key to continue." : contact_status(state, rejection);
        } else input->contact_status = session_error != SV_OK ?
            sv_result_text(session_error) : contact_status(state, rejection);
        bool present = true;
        if (state == SV_SOCKET_READY && app) {
            SvAppView view = sv_app_view(app);
            if (view.flush_sequence != presented_flush && SDL_GetTicks() < view.flush_due_ms)
                present = false;
        }
        if (present) {
            if (!sv_endpoint_draw(renderer, font, endpoint, input, user_scale)) break;
            if (state == SV_SOCKET_READY && app)
                presented_flush = sv_app_view(app).flush_sequence;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                quit = true;
                break;
            }
            if (state == SV_SOCKET_READY && app && event.type == SDL_EVENT_KEY_DOWN &&
                !event.key.repeat && event.common.timestamp >= pause_since_ns &&
                sv_app_view(app).paused) {
                (void)sv_app_ack_pause(app, generation, observed_pause);
                SDL_FlushEvents(SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
                continue;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                quit = true;
                break;
            }
        }
        if (state == SV_SOCKET_READY) {
            if (!reached_ready) {
                const int *version = sv_contact_socket_version(connection);
                const SvContactSetup *setup = sv_app_character_setup(app, generation);
                printf("SV contact ready host=%s port=%u server=%d.%d.%d.%d.%d.%d races=%u classes=%u traits=%u motd=%u\n",
                       endpoint->host, (unsigned)endpoint->port, version[0], version[1],
                       version[2], version[3], version[4], version[5],
                       (unsigned)setup->race_count, (unsigned)setup->class_count,
                       (unsigned)setup->trait_count, (unsigned)setup->motd_size);
            }
            reached_ready = true;
            result = 0;
        } else if (state >= SV_SOCKET_DNS_ERROR) {
            result = 1;
            if (!reported_failure) fprintf(stderr, "SV contact failed: %s (status=%u)\n",
                                           input->contact_status, rejection);
            reported_failure = true;
            if (app && sv_app_view(app).active)
                (void)sv_app_close_reason(app,
                    session_error == SV_OK ? SV_CLOSED : session_error);
            if (connection) { sv_contact_socket_stop(connection); connection = NULL; }
            if (options.frames || reached_ready) break;
        }
        if (options.frames && ++frames >= options.frames) break;
        SDL_Delay(16);
    }
    sv_contact_socket_stop(connection);
    SDL_free(wire.remaining);
    (void)sv_app_destroy(app);
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

static void report_resource_source(const char *root, const char *library,
                                   const char *category, const char *requested,
                                   const char *relative, SDL_PathType type)
{
    SvResourceRef resolved;
    bool found = sv_resource_find(root, library, relative, type, &resolved);
    printf("SV resource %s requested=%s source=%s availability=%s\n",
           category, requested, found ? resolved.path : "missing",
           found ? "found" : "missing");
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
    SvProfile profile;
    SvOptions options_snapshot;
    int result = 1;
    if (!root) {
        root = SDL_getenv("TOMENET_SDL3_USER_PATH");
        if (!root || !*root) {
            owned_root = SDL_GetPrefPath("TomenetGame", "tomenet");
            root = owned_root;
        }
    }
    if (!root || !sv_profile_load(&profile, root)) {
        SDL_SetError("Cannot read SV profile"); goto done;
    }
    if (!sv_options_load_base(&options_snapshot, root)) {
        SDL_SetError("Cannot read SV option layers"); goto done;
    }
    if (options.window_override) profile.windowed = options.windowed;
    if (options.ui_scale_override) profile.ui_scale = options.ui_scale_override;
    sv_profile_report(&profile);
    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) goto done;
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
    font = sv_font_open_requested(root, library, profile.text_font);
    if (!font) goto done;
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (!profile.windowed) flags |= SDL_WINDOW_FULLSCREEN;
    window = SDL_CreateWindow("TomeNET SV - Server",options.width,options.height,flags);
    if (!window) goto done;
    float window_scale = SDL_GetWindowDisplayScale(window) / SDL_GetWindowPixelDensity(window);
    if (window_scale <= 0 || !SDL_SetWindowMinimumSize(window,
            (int)SDL_ceilf(1024 * window_scale), (int)SDL_ceilf(768 * window_scale))) goto done;
    if (profile.windowed && !SDL_SetWindowSize(window,
            (int)SDL_roundf(options.width * window_scale),
            (int)SDL_roundf(options.height * window_scale))) goto done;
    renderer = SDL_CreateRenderer(window,NULL);
    if (!renderer) goto done;
    printf("SV resource text requested=%s effective=%s\n", profile.text_font,
           sv_font_resource(font));
    char relative[SV_RESOURCE_PATH];
    if (SDL_snprintf(relative, sizeof(relative), "xtra/font/%s", profile.map_font) <
        (int)sizeof(relative))
        report_resource_source(root, library, "map-font", profile.map_font,
                               relative, SDL_PATHTYPE_FILE);
    if (SDL_snprintf(relative, sizeof(relative), "xtra/graphics/%s.bmp", profile.tiles) <
        (int)sizeof(relative))
        report_resource_source(root, library, "graphics", profile.tiles,
                               relative, SDL_PATHTYPE_FILE);
    if (SDL_snprintf(relative, sizeof(relative), "xtra/%s", profile.sound_pack) <
        (int)sizeof(relative))
        report_resource_source(root, library, "sound-pack", profile.sound_pack,
                               relative, SDL_PATHTYPE_DIRECTORY);
    if (SDL_snprintf(relative, sizeof(relative), "xtra/%s", profile.music_pack) <
        (int)sizeof(relative))
        report_resource_source(root, library, "music-pack", profile.music_pack,
                               relative, SDL_PATHTYPE_DIRECTORY);
    int window_count = 0;
    SDL_Window **windows = SDL_GetWindows(&window_count);
    SDL_free(windows);
    if (window_count != 1) { SDL_SetError("SV requires exactly one system window"); goto done; }
    SvEndpointInput input;
    sv_endpoint_input_begin(&input);
    float user_scale = profile.ui_scale / 100.0f;
    if (!sv_endpoint_draw(renderer,font,&endpoint,&input,user_scale) ||
        !sv_endpoint_render(renderer,font,&endpoint,&input,user_scale)) goto done;
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
        if (!sv_endpoint_draw(renderer,font,&endpoint,&input,user_scale)) goto done;
        if (options.frames && ++frames >= options.frames) break;
        SDL_Delay(16);
    }
    if (endpoint.phase == SV_ENDPOINT_SELECTED) {
        printf("SV endpoint selected host=%s port=%u\n",
               endpoint.host,(unsigned)endpoint.port);
        if (!options.selected_endpoint) {
            char account[80] = {0}, password[80] = {0};
            int entered = 1;
            if (!options.account || !options.password) {
                entered = enter_credentials(renderer, font, &endpoint, &input,
                                            options.frames, account, password, user_scale);
                options.account = account;
                options.password = password;
            }
            if (entered > 0) result = run_contact(window, renderer, font, &endpoint, &input,
                                                 options, user_scale);
            else result = entered < 0 ? 1 : 0;
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
