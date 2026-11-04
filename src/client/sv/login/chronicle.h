#ifndef SV_CHRONICLE_H
#define SV_CHRONICLE_H
#include <SDL3/SDL.h>
#define SV_CHRONICLE_EVENTS 100
#define SV_CHRONICLE_SOURCE_EVENTS 40
#define SV_CHRONICLE_BODY_LIMIT (256 * 1024)
typedef enum { SV_CHRONICLE_DEATH, SV_CHRONICLE_OCCURRENCE, SV_CHRONICLE_SUPERUNIQUE } SvChronicleType;
typedef struct {
    SvChronicleType type;
    unsigned server; /* 0 EU, 1 NA, 2 APAC */
    char date[11], date_label[16], text[768];
    Uint32 color;
    unsigned source_order;
} SvChronicleEvent;
typedef struct SvChronicleSnapshot {
    SvChronicleEvent events[SV_CHRONICLE_EVENTS];
    size_t count;
    bool loading;
    unsigned completed_sources, failed_sources, revision;
} SvChronicleSnapshot;
typedef struct SvChronicle SvChronicle;
/* Source pages are supplementary; failure never prevents endpoint selection.
 * Refresh begins every five seconds; slow batches never overlap. Last good
 * source data remains visible during refresh and after transient failures.
 * Worker results are copied into the main owner's snapshot, never into UI state. */
/* Main-owner-only start; its one-time curl initialization outlives workers. */
SvChronicle *sv_chronicle_start(void);
bool sv_chronicle_poll(SvChronicle *provider, SvChronicleSnapshot *snapshot);
void sv_chronicle_stop(SvChronicle *provider);
/* Production HTML parser. now_month is 1..12, for yearless death records. */
bool sv_chronicle_parse(const char *html, size_t size, SvChronicleType type,
                        unsigned server, int now_year, int now_month,
                        SvChronicleEvent events[SV_CHRONICLE_SOURCE_EVENTS], size_t *count);
#endif
