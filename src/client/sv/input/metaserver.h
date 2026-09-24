#ifndef SV_METASERVER_H
#define SV_METASERVER_H
#include "input/endpoint.h"
#include <SDL3/SDL.h>

typedef struct SvMetaserver SvMetaserver;
/* Feed retrieval and ICMP probes run outside the render/input thread. A poll
 * copies one coherent snapshot; the endpoint model retains selection by host
 * and port while later snapshots update ping values. */
SvMetaserver *sv_metaserver_start(const char *host, uint16_t port);
bool sv_metaserver_poll(SvMetaserver *provider, SvEndpoint *endpoint, bool *failed);
void sv_metaserver_stop(SvMetaserver *provider);
bool sv_metaserver_parse(const char *xml, size_t size, SvServer *servers, size_t *count);
#endif
