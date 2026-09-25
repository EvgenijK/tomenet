#include "endpoint-run.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <string.h>

typedef struct { int manual; } Script;
typedef struct { unsigned calls; bool selected_at_ping, ping_applied; } Feed;
static bool failed_feed(void *context, SvEndpoint *endpoint, bool *failed)
{
    (void)context; (void)endpoint;
    *failed = true;
    return true;
}
static bool poll_feed(void *context, SvEndpoint *endpoint, bool *failed)
{
    Feed *feed = context;
    *failed = false;
    ++feed->calls;
    if (feed->calls == 1) {
        SvServer rows[] = {
            {.host = "host.example", .port = 18349, .label = "Test server", .ping_ms = -1},
            {.host = "second.example", .port = 18350, .label = "Second server", .ping_ms = -1},
        };
        assert(sv_endpoint_servers(endpoint, rows, 2));
        return true;
    }
    if (feed->calls == 32) {
        feed->selected_at_ping = endpoint->selected == 1;
        feed->ping_applied = sv_endpoint_ping(endpoint, "second.example", 18350, 37);
        return true;
    }
    return false;
}
static int events(void *data)
{
    Script *script = data;
    SDL_Delay(400);
    SDL_Event event = {0};
    if (script->manual) {
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = "q";
        assert(SDL_PushEvent(&event));
        event = (SDL_Event){0};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_ESCAPE;
        event.key.scancode = SDL_SCANCODE_ESCAPE;
        assert(SDL_PushEvent(&event));
    } else {
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_DOWN;
        event.key.scancode = SDL_SCANCODE_DOWN;
        assert(SDL_PushEvent(&event));
        assert(SDL_PushEvent(&event));
        SDL_Delay(300);
        event = (SDL_Event){0};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_RETURN;
        event.key.scancode = SDL_SCANCODE_RETURN;
        assert(SDL_PushEvent(&event));
    }
    return 0;
}
static int credential_events(void *data)
{
    (void)data;
    SDL_Delay(400);
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = "Test";
    assert(SDL_PushEvent(&event));
    event = (SDL_Event){0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_RETURN;
    event.key.scancode = SDL_SCANCODE_RETURN;
    assert(SDL_PushEvent(&event));
    event = (SDL_Event){0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = "pw";
    assert(SDL_PushEvent(&event));
    event.text.text = "\xc3\xa9";
    assert(SDL_PushEvent(&event));
    event = (SDL_Event){0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_RETURN;
    event.key.scancode = SDL_SCANCODE_RETURN;
    assert(SDL_PushEvent(&event));
    SDL_Delay(250);
    event = (SDL_Event){0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_ESCAPE;
    event.key.scancode = SDL_SCANCODE_ESCAPE;
    assert(SDL_PushEvent(&event));
    return 0;
}
int main(int argc, char **argv)
{
    assert(argc == 4);
    for (int manual = 0; manual < 2; ++manual) {
        Script script = {.manual = manual};
        Feed feed = {0};
        SDL_Thread *thread = SDL_CreateThread(events, "sv-scene-input", &script);
        assert(thread);
        SvEndpointChoice choice = {0};
        SvEndpointOptions options = {.root = argv[1], .library = argv[2],
            .server_list = manual ? argv[3] : NULL, .width = 1024, .height = 768,
            .frames = 180, .windowed = 1, .selected_endpoint = &choice,
            .source_poll = manual ? NULL : poll_feed, .source_context = &feed};
        assert(sv_endpoint_run(options) == 0);
        SDL_WaitThread(thread, NULL);
        if (manual) assert(choice.phase == SV_ENDPOINT_CANCELLED && !choice.host[0]);
        else assert(feed.selected_at_ping && feed.ping_applied &&
                    choice.phase == SV_ENDPOINT_SELECTED && choice.port == 18350 &&
                    !strcmp(choice.host, "second.example"));
    }
    SvEndpointChoice failed = {0};
    SvEndpointOptions fallback = {.root = argv[1], .library = argv[2],
        .width = 1024, .height = 768, .frames = 2,
        .windowed = 1, .selected_endpoint = &failed, .source_poll = failed_feed};
    assert(sv_endpoint_run(fallback) == 0);
    assert(failed.phase == SV_ENDPOINT_MANUAL && !failed.host[0]);
    SDL_Thread *credential_thread = SDL_CreateThread(credential_events, "sv-contact-credentials", NULL);
    assert(credential_thread);
    SvEndpointOptions credentials = {.root = argv[1], .library = argv[2],
        .server = "127.0.0.1:1", .width = 1024, .height = 768,
        .frames = 180, .windowed = 1};
    assert(sv_endpoint_run(credentials) == 0);
    SDL_WaitThread(credential_thread, NULL);
    return 0;
}
