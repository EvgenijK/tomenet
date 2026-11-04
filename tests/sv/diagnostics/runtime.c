#include "runtime.h"

typedef struct {
    SvRuntimeCheck check;
    int overflow;
    int (*run)(SvApp *, void *);
    void *context;
} SvRuntimeScope;

static void fallback(void *context)
{
    SvRuntimeScope *scope = context;
    if (scope->check.fallback_entries == UINT32_MAX) scope->overflow = 1;
    else ++scope->check.fallback_entries;
}

static int run_scope(SvApp *app, void *context)
{
    SvRuntimeScope *scope = context;
    return scope->run(app, scope->context) && !scope->overflow;
}

SvRuntimeCheck sv_app_check_run(SvApp *app, SvRuntimeScenario scenario,
                              int (*run)(SvApp *, void *), void *context)
{
    SvRuntimeScope scope = {{scenario, 0, 0}, 0, run, context};
    int valid = run && scenario >= 0 && scenario < SV_SCENARIO_COUNT;
    scope.check.completed = sv_app_run(app, valid ? run_scope : NULL, &scope,
                                      fallback, &scope);
    return scope.check;
}

int sv_runtime_write(FILE *stream, SvRuntimeCheck check)
{
    static const char *names[] = {"hp", "message", "request", "lifecycle", "geometry",
        "timing", "timing-delayed", "arch"};
    if (!stream || check.scenario < 0 || check.scenario >= SV_SCENARIO_COUNT) return 0;
    if (fprintf(stream, "{\"scenario\":\"scenario.stage-a.%s\",\"completed\":%s,"
        "\"fallbackEntries\":%u,\"routes\":[", names[check.scenario],
        check.completed ? "true" : "false", (unsigned)check.fallback_entries) < 0) return 0;
    if (check.fallback_entries && fprintf(stream,
        "{\"routeId\":\"route.terminal-handoff\",\"reason\":\"future-flow\",\"count\":%u}",
        (unsigned)check.fallback_entries) < 0) return 0;
    return fputs("]}\n", stream) >= 0 && fflush(stream) == 0;
}
