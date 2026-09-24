#ifndef SV_ENDPOINT_RUN_H
#define SV_ENDPOINT_RUN_H
#include "input/endpoint.h"
typedef struct {
    SvEndpointPhase phase;
    char host[SV_HOST_LIMIT + 1];
    uint16_t port;
    int protocol;
} SvEndpointChoice;
typedef bool (*SvEndpointPoll)(void *context, SvEndpoint *endpoint, bool *failed);
typedef struct {
    const char *root, *library, *server, *server_list;
    int width, height, frames, windowed, window_override, ui_scale_override;
    unsigned port;
    const char *metaserver;
    SvEndpointChoice *selected_endpoint;
    SvEndpointPoll source_poll;
    void *source_context;
    const char *account, *password, *real_name;
} SvEndpointOptions;
int sv_endpoint_run(SvEndpointOptions options);
#endif
