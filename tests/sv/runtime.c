#include "app.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int answer(SvApp *app, void *context)
{
    const int version[] = {4,7,0,2,0,2};
    const unsigned char request[] = {184,0,0,0,7,'?',0};
    unsigned char output[6];
    unsigned char key = context ? 27 : 'Y';
    const unsigned char expected[] = {184,0,0,0,7,context ? 0 : 'Y'};
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    assert(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 1);
    assert(sv_app_key(app, gen, sv_app_view(app).request.sequence, key) == SV_OK);
    SvOutput reply = sv_app_take_output(app, gen, output, sizeof(output));
    return reply.result == SV_OK && reply.size == sizeof(expected) && !memcmp(output, expected, sizeof(expected));
}
static int incomplete(SvApp *app, void *context)
{
    (void)context;
    assert(answer(app, NULL));
    return 0; /* Required scenario step did not finish. */
}
static int decode_failure(SvApp *app, void *context)
{
    (void)context;
    const int version[] = {4,7,0,2,0,2};
    unsigned char bad[85];
    memset(bad, 'x', sizeof(bad)); bad[0] = 184;
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_receive(app, sv_app_view(app).generation, bad, sizeof(bad)) == SV_OK);
    return sv_app_step(app, 16).result == SV_OK;
}
static int denied_child(SvApp *app, void *context)
{
    (void)context;
    assert(sv_app_terminal_fallback(app) == SV_INVALID);
    return answer(app, NULL);
}
static int child_failure(SvApp *app, void *context)
{
    (void)context;
    SvRuntimeCheck child = sv_app_check_run(app, SV_SCENARIO_REQUEST, incomplete, NULL);
    assert(!child.completed);
    return 1; /* Cannot hide an incomplete child. */
}
static int reset_with_route(SvApp *app, void *context)
{
    (void)context;
    assert(answer(app, NULL));
    uint64_t old = sv_app_view(app).generation;
    assert(sv_app_terminal_fallback(app) == SV_INVALID);
    assert(sv_app_close(app) == SV_OK);
    SvRuntimeCheck child = sv_app_check_run(app, SV_SCENARIO_REQUEST, answer, (void *)1);
    assert(child.completed && !child.fallback_entries);
    assert(sv_app_key(app, old, 1, 'x') == SV_STALE);
    child = sv_app_check_run(app, SV_SCENARIO_REQUEST, denied_child, NULL);
    assert(child.completed && child.fallback_entries == 1);
    assert(sv_app_destroy(app) == SV_BUSY);
    return 1;
}
int main(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app);
    SvRuntimeCheck check = sv_app_check_run(app, SV_SCENARIO_REQUEST, answer, NULL);
    assert(check.completed && check.fallback_entries == 0);
    check = sv_app_check_run(app, SV_SCENARIO_REQUEST, answer, (void *)1);
    assert(check.completed && !check.fallback_entries);
    check = sv_app_check_run(app, SV_SCENARIO_REQUEST, incomplete, NULL);
    assert(!check.completed);
    assert(sv_runtime_write(stdout, check));
    check = sv_app_check_run(app, SV_SCENARIO_REQUEST, decode_failure, NULL);
    assert(!check.completed && !sv_app_view(app).active);
    check = sv_app_check_run(app, SV_SCENARIO_LIFECYCLE, child_failure, NULL);
    assert(!check.completed);
    check = sv_app_check_run(app, SV_SCENARIO_LIFECYCLE, reset_with_route, NULL);
    assert(check.completed && check.fallback_entries == 2);
    assert(sv_runtime_write(stdout, check));
    check = sv_app_check_run(app, SV_SCENARIO_REQUEST, answer, NULL);
    assert(check.completed && !check.fallback_entries);
    assert(sv_runtime_write(stdout, check));
    assert(sv_app_destroy(app) == SV_OK);
    puts("Runtime collector checks passed");
}
