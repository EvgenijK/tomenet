#include "login/chronicle.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    SvChronicleEvent rows[SV_CHRONICLE_SOURCE_EVENTS];
    size_t count;
    /* Official source formats: deaths pre; legends body/br; superuniques pre
     * with the site's malformed closing span carrying a style attribute. */
    const char deaths[] = "<h2>Recent Deaths</h2><pre>01 Jan - Novice Ada (1) was killed.\n"
        "31 Dec - Champion Bea (40) was defeated by &lt;poison&gt;.\n</pre>";
    assert(sv_chronicle_parse(deaths, strlen(deaths), SV_CHRONICLE_DEATH, 0, 2026, 1, rows, &count));
    assert(count == 2 && !strcmp(rows[0].date, "2026-01-01") && !strcmp(rows[1].date, "2025-12-31"));
    assert(rows[0].color == 0x777777 && rows[1].color == 0xff3030);
    assert(!strcmp(rows[1].text, "Champion Bea (40) was defeated by <poison>."));
    const char legends[] = "<h2>Noteworthy Occurances</h2>Ironman Records<br/>"
        "<span style='color: #666666'>2026-10-13 </span><span style='color: #b70000'>"
        "His royalty August (54) died and was instantly resurrected<br/></span>"
        "2026-02-30 Invalid calendar date<br/><div id=\"footer\">";
    assert(sv_chronicle_parse(legends, strlen(legends), SV_CHRONICLE_OCCURRENCE, 1, 2026, 10, rows, &count));
    assert(count == 1 && rows[0].color == 0xb70000 && rows[0].server == 1);
    const char uniques[] = "<pre>2026-07-20 : Tik&#39;srvzllat was slain by Lightbearer.\n"
        "2024-09-27 : </span><span style=\"color: #af00ff\">The Living Lightning was slain by Omgoshies.</span style=\"color: #000000\">\n</pre>";
    assert(sv_chronicle_parse(uniques, strlen(uniques), SV_CHRONICLE_SUPERUNIQUE, 2, 2026, 10, rows, &count));
    assert(count == 2 && rows[1].color == 0xaf00ff && rows[0].server == 2);
    assert(!strcmp(rows[0].text, "Tik'srvzllat was slain by Lightbearer."));
    assert(!sv_chronicle_parse(uniques, SV_CHRONICLE_BODY_LIMIT + 1, SV_CHRONICLE_DEATH, 0, 2026, 10, rows, &count));
    assert(!sv_chronicle_parse("<pre>\0</pre>", 12, SV_CHRONICLE_DEATH, 0, 2026, 10, rows, &count));
    assert(!sv_chronicle_parse("<pre>incomplete", 15, SV_CHRONICLE_DEATH, 0, 2026, 10, rows, &count));
    puts("SV Chronicle production parser: formats, colors, entities, rollover and malformed bounds passed");
    return 0;
}
