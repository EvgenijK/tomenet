#ifndef SV_RUNTIME_H
#define SV_RUNTIME_H
#include "app.h"
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
/* Required events/replies must be verified by the scenario callback. Nested
 * failures poison their parent; session teardown does not reset observation. */
SvRuntimeCheck sv_app_check_run(SvApp *app, SvRuntimeScenario scenario,
                              int (*run)(SvApp *, void *), void *context);
int sv_runtime_write(FILE *stream, SvRuntimeCheck check);
#endif
