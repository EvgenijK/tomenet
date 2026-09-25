#define _POSIX_C_SOURCE 200809L
#include "input/metaserver.h"
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct { int listener; bool served; } Fixture;
static int serve(void *data)
{
    Fixture *fixture = data;
    fd_set ready;
    FD_ZERO(&ready);
    FD_SET(fixture->listener, &ready);
    struct timeval timeout = {3, 0};
    if (select(fixture->listener + 1, &ready, NULL, NULL, &timeout) != 1) return 1;
    int client = accept(fixture->listener, NULL, NULL);
    if (client < 0) return 1;
    const char feed[] = "<meta>Test</meta><server url='127.0.0.1' port='18348' protocol='2'/>"
        "<server url='localhost' port='18350' protocol='3'/>";
    size_t half = (sizeof(feed) - 1) / 2;
    if (send(client, feed, half, 0) != (ssize_t)half) return 1;
    SDL_Delay(20);
    if (send(client, feed + half, sizeof(feed) - 1 - half, 0) != (ssize_t)(sizeof(feed) - 1 - half)) return 1;
    close(client);
    fixture->served = true;
    return 0;
}
int main(void)
{
    assert(SDL_Init(0));
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener >= 0);
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_port = 0};
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(bind(listener, (struct sockaddr *)&address, sizeof(address)) == 0);
    assert(listen(listener, 1) == 0);
    socklen_t length = sizeof(address);
    assert(getsockname(listener, (struct sockaddr *)&address, &length) == 0);
    Fixture fixture = {.listener = listener};
    SDL_Thread *server = SDL_CreateThread(serve, "sv-feed-fixture", &fixture);
    assert(server);
    SvMetaserver *provider = sv_metaserver_start("127.0.0.1", ntohs(address.sin_port));
    assert(provider);
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint, 18348);
    bool selected = false, ping_updated = false;
    uint64_t deadline = SDL_GetTicks() + 4000;
    while (SDL_GetTicks() < deadline && !ping_updated) {
        bool failed = false;
        if (sv_metaserver_poll(provider, &endpoint, &failed)) assert(!failed);
        if (endpoint.server_count == 2 && !selected) {
            endpoint.selected = 1;
            selected = true;
        }
        if (selected && endpoint.servers[0].ping_ms == 42 &&
            endpoint.servers[1].ping_ms == 42) ping_updated = true;
        SDL_Delay(10);
    }
    assert(selected && ping_updated && endpoint.selected == 1);
    assert(sv_endpoint_choose(&endpoint,1));
    assert(!strcmp(endpoint.host,"localhost") && endpoint.port == 18350 && endpoint.protocol == 3);
    sv_metaserver_stop(provider);
    int status = -1;
    SDL_WaitThread(server, &status);
    assert(status == 0 && fixture.served);
    fixture.served = false;
    server = SDL_CreateThread(serve, "sv-feed-cancel-fixture", &fixture);
    assert(server);
    assert(setenv("SV_PING_DELAY_MS", "5000", 1) == 0);
    provider = sv_metaserver_start("127.0.0.1", ntohs(address.sin_port));
    assert(provider);
    sv_endpoint_begin(&endpoint, 18348);
    deadline = SDL_GetTicks() + 3000;
    while (SDL_GetTicks() < deadline && endpoint.server_count != 2) {
        bool failed = false;
        if (sv_metaserver_poll(provider, &endpoint, &failed)) assert(!failed);
        SDL_Delay(10);
    }
    assert(endpoint.server_count == 2);
    SDL_Delay(80); /* The fixture ping is running when startup is cancelled. */
    uint64_t cancelled_at = SDL_GetTicks();
    sv_metaserver_stop(provider);
    assert(SDL_GetTicks() - cancelled_at < 200);
    SDL_WaitThread(server, &status);
    assert(status == 0 && fixture.served);
    SDL_Delay(100);
    close(listener);
    SDL_Quit();
    puts("SV live metaserver split feed, ping delivery, stable selection passed");
    return 0;
}
