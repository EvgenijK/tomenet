#ifndef SV_ENDPOINT_H
#define SV_ENDPOINT_H
#include "input/text-field.h"
#include <stdint.h>
#define SV_SERVER_COUNT 32
#define SV_HOST_LIMIT 79
typedef struct {
    char host[SV_HOST_LIMIT + 1];
    uint16_t port;
    char label[96];
    int ping_ms; /* -1 pending, -2 unavailable */
    int protocol;
} SvServer;
typedef enum { SV_ENDPOINT_LIST, SV_ENDPOINT_MANUAL,
               SV_ENDPOINT_SELECTED, SV_ENDPOINT_CANCELLED } SvEndpointPhase;
typedef struct {
    SvEndpointPhase phase;
    SvServer servers[SV_SERVER_COUNT];
    size_t server_count, selected;
    SvTextField editor;
    SvTextHistory history;
    char host[SV_HOST_LIMIT + 1];
    uint16_t port, default_port;
    int protocol;
    bool invalid_address;
} SvEndpoint;
void sv_endpoint_begin(SvEndpoint *endpoint, uint16_t default_port);
bool sv_endpoint_servers(SvEndpoint *endpoint, const SvServer *servers, size_t count);
bool sv_endpoint_ping(SvEndpoint *endpoint, const char *host, uint16_t port, int ping_ms);
bool sv_endpoint_choose(SvEndpoint *endpoint, size_t index);
void sv_endpoint_manual(SvEndpoint *endpoint);
void sv_endpoint_key(SvEndpoint *endpoint, SvEditKey key);
bool sv_endpoint_argument(SvEndpoint *endpoint, const char *address);
#endif
