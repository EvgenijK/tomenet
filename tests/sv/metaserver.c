#include "input/metaserver.h"
#include <assert.h>
#include <string.h>
int main(void)
{
    const char feed[] = "<meta>Live</meta><server url=\"First.Example\" port=\"18348\" protocol=\"2\"/>"
        "<server static='1' url='second.example' port='18350' protocol='3'/>";
    SvServer servers[SV_SERVER_COUNT] = {{0}};
    size_t count = 0;
    assert(sv_metaserver_parse(feed, sizeof(feed) - 1, servers, &count));
    assert(count == 2 && !strcmp(servers[0].host, "First.Example"));
    assert(servers[0].port == 18348 && servers[0].protocol == 2);
    assert(servers[1].port == 18350 && servers[1].protocol == 3);
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint, 18348);
    assert(sv_endpoint_servers(&endpoint, servers, count));
    endpoint.selected = 1;
    assert(sv_endpoint_ping(&endpoint, "second.example", 18350, 37));
    servers[0].ping_ms = 19;
    servers[1].ping_ms = 37;
    assert(sv_endpoint_servers(&endpoint, servers, count));
    assert(endpoint.selected == 1 && endpoint.servers[1].ping_ms == 37);
    assert(sv_endpoint_choose(&endpoint, endpoint.selected));
    assert(!strcmp(endpoint.host, "second.example") && endpoint.port == 18350 && endpoint.protocol == 3);
    const char bad[] = "<server url='-unsafe' port='18348'/>";
    assert(!sv_metaserver_parse(bad, sizeof(bad) - 1, servers, &count));
    return 0;
}
