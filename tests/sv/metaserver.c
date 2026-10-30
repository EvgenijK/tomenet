#include "input/metaserver.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
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
    const char metadata[] = "<server url='live.example' port='18348' protocol='2'>"
        "<game>TomeNET Arcade</game><version>4.9.4</version><notes>Europe &amp; more</notes>"
        "<static>1</static><player>Ada &lt;One&gt;</player><player>Bea</player></server>"
        "<server url='live.example' port='20348' protocol='2'>"
        "<game>TomeNET</game><version>4.9.3</version><static>0</static></server>"
        "<server url='unknown.example' port='18348'/>";
    assert(sv_metaserver_parse(metadata, sizeof(metadata) - 1, servers, &count));
    assert(count == 3 && servers[0].metadata_available && servers[0].official);
    assert(!strcmp(servers[0].game, "TomeNET Arcade") && !strcmp(servers[0].version, "4.9.4"));
    assert(!strcmp(servers[0].notes, "Europe & more"));
    assert(servers[0].player_count == 2 && !strcmp(servers[0].players, "Ada <One>, Bea"));
    assert(servers[1].metadata_available && servers[1].player_count == 0 && !servers[1].official);
    assert(!servers[2].metadata_available);
    char bounded[4096];
    size_t used = (size_t)snprintf(bounded, sizeof(bounded), "<server url='bounded.example' port='18348'>");
    for (unsigned i = 0; i < 40; ++i)
        used += (size_t)snprintf(bounded + used, sizeof(bounded) - used,
                                "<player>Player with a long name %02u</player>", i);
    used += (size_t)snprintf(bounded + used, sizeof(bounded) - used, "</server>");
    assert(sv_metaserver_parse(bounded, used, servers, &count));
    assert(servers[0].player_count == 40 && strlen(servers[0].players) < sizeof(servers[0].players));
    const char incomplete[] = "<server url='broken.example' port='18348'><version>4.9.4</version>";
    assert(!sv_metaserver_parse(incomplete, sizeof(incomplete) - 1, servers, &count));
    const char bad[] = "<server url='-unsafe' port='18348'/>";
    assert(!sv_metaserver_parse(bad, sizeof(bad) - 1, servers, &count));
    return 0;
}
