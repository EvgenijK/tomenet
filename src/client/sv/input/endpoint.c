#include "input/endpoint.h"
#include <stdlib.h>
#include <string.h>

void sv_endpoint_begin(SvEndpoint *endpoint, uint16_t default_port)
{
    *endpoint = (SvEndpoint){0};
    endpoint->default_port = default_port ? default_port : 18348;
    endpoint->protocol = 2;
    endpoint->selected = SIZE_MAX;
}

bool sv_endpoint_servers(SvEndpoint *endpoint, const SvServer *servers, size_t count)
{
    if (!servers || count > SV_SERVER_COUNT) return false;
    for (size_t i = 0; i < count; ++i)
        if (!servers[i].host[0] || !memchr(servers[i].host, 0, sizeof(servers[i].host)) ||
            !servers[i].port || !memchr(servers[i].label, 0, sizeof(servers[i].label))) return false;
    size_t selected = SIZE_MAX;
    if (endpoint->selected < endpoint->server_count) {
        const SvServer *old = &endpoint->servers[endpoint->selected];
        for (size_t i = 0; i < count; ++i)
            if (servers[i].port == old->port && !strcmp(servers[i].host, old->host)) {
                selected = i; break;
            }
    }
    memcpy(endpoint->servers, servers, count * sizeof(*servers));
    endpoint->server_count = count;
    endpoint->selected = selected;
    return true;
}

bool sv_endpoint_ping(SvEndpoint *endpoint, const char *host, uint16_t port, int ping_ms)
{
    for (size_t i = 0; i < endpoint->server_count; ++i)
        if (endpoint->servers[i].port == port && !strcmp(endpoint->servers[i].host, host)) {
            endpoint->servers[i].ping_ms = ping_ms;
            return true;
        }
    return false;
}

bool sv_endpoint_choose(SvEndpoint *endpoint, size_t index)
{
    if (index >= endpoint->server_count) return false;
    endpoint->selected = index;
    strcpy(endpoint->host, endpoint->servers[index].host);
    endpoint->port = endpoint->servers[index].port;
    endpoint->protocol = endpoint->servers[index].protocol > 0 ? endpoint->servers[index].protocol : 2;
    endpoint->phase = SV_ENDPOINT_SELECTED;
    return true;
}

void sv_endpoint_manual(SvEndpoint *endpoint)
{
    endpoint->phase = SV_ENDPOINT_MANUAL;
    endpoint->invalid_address = false;
    sv_text_begin_history(&endpoint->editor, &endpoint->history,
                          "europe.tomenet.eu", SV_HOST_LIMIT, false);
}

static bool address(SvEndpoint *endpoint, const char *input)
{
    size_t length = strlen(input);
    if (!length || length > SV_HOST_LIMIT) return false;
    const char *colon = strchr(input, ':');
    uint16_t port = endpoint->default_port;
    size_t host_length = length;
    if (colon && !strchr(colon + 1, ':')) {
        if (colon == input || !colon[1]) return false;
        char *end;
        unsigned long value = strtoul(colon + 1, &end, 10);
        if (*end || !value || value > 65535) return false;
        port = (uint16_t)value;
        host_length = (size_t)(colon - input);
    }
    for (size_t i = 0; i < host_length; ++i)
        if ((unsigned char)input[i] <= 32 || (unsigned char)input[i] >= 127) return false;
    memcpy(endpoint->host, input, host_length);
    endpoint->host[host_length] = 0;
    endpoint->port = port;
    endpoint->protocol = 2;
    endpoint->phase = SV_ENDPOINT_SELECTED;
    return true;
}

bool sv_endpoint_argument(SvEndpoint *endpoint, const char *input)
{
    return input && address(endpoint, input);
}

void sv_endpoint_key(SvEndpoint *endpoint, SvEditKey key)
{
    if (endpoint->phase == SV_ENDPOINT_LIST) {
        if (key == SV_EDIT_CANCEL) endpoint->phase = SV_ENDPOINT_CANCELLED;
        return;
    }
    if (endpoint->phase != SV_ENDPOINT_MANUAL) return;
    sv_text_key(&endpoint->editor, key);
    if (endpoint->editor.cancelled) {
        endpoint->host[0] = 0;
        endpoint->phase = SV_ENDPOINT_CANCELLED;
    } else if (endpoint->editor.accepted) {
        endpoint->invalid_address = !address(endpoint, endpoint->editor.bytes);
        if (endpoint->invalid_address) endpoint->editor.accepted = false;
    }
}
