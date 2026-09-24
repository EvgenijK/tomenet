#include "endpoint-run.h"
#include "input/native-endpoint.h"
#include "input/metaserver.h"
#include "ui/endpoint-scene.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

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
        owned_root = SDL_GetPrefPath("TomenetGame", "tomenet");
        root = owned_root;
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
    if (endpoint.phase == SV_ENDPOINT_SELECTED)
        printf("SV endpoint selected host=%s port=%u; contact pending SV-B-002\n",
               endpoint.host,(unsigned)endpoint.port);
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
