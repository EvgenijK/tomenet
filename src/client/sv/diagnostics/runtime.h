#ifndef SV_RUNTIME_H
#define SV_RUNTIME_H
#include <stdint.h>
#include <stdio.h>
typedef enum {
    SV_SCENARIO_HP, SV_SCENARIO_MESSAGE, SV_SCENARIO_REQUEST,
    SV_SCENARIO_LIFECYCLE, SV_SCENARIO_GEOMETRY, SV_SCENARIO_TIMING,
    SV_SCENARIO_TIMING_DELAYED, SV_SCENARIO_ARCH, SV_SCENARIO_COUNT
} SvRuntimeScenario;
/* One bounded, denied route today; no payloads or arbitrary reason strings. */
typedef struct {
    SvRuntimeScenario scenario;
    int completed;
    uint32_t fallback_entries;
} SvRuntimeCheck;
int sv_runtime_write(FILE *stream, SvRuntimeCheck check);
#endif
