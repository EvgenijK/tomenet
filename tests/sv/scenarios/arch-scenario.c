#include "arch-scenario.h"
#include <stdio.h>
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Architecture native scenario failed at line %d: %s\n", __LINE__, #test); \
    return 0; } } while (0)
int sv_arch_scenario(SvApp *app, SvScenarioFrame submit, void *context)
{
    const int version[] = {4,7,0,2,0,2};
    const unsigned char hp[] = {24,0,100,0,50,0, 24,0,100,0,10,0, 24,0,100,0,50,0};
    const int expected[] = {50,10,50};
    unsigned char overflow[1025] = {0};
    CHECK(sv_app_open(app, version) == SV_OK);
    CHECK(sv_app_set_alerts(app, (SvAlertOptions){1,1}, (SvAttention){1}) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    CHECK(sv_app_receive(app, generation, hp, sizeof(hp)) == SV_OK);
    for (unsigned i = 0; i < 3; ++i) {
        SvStep step = sv_app_step(app, 1);
        CHECK(step.processed == 1 && step.pending_bytes == 12 - i * 6);
        CHECK(sv_app_view(app).status.current == expected[i]);
        CHECK(submit(context, sv_app_view(app)));
    }
    CHECK(sv_app_receive(app, generation, overflow, sizeof(overflow)) == SV_INPUT_OVERFLOW);
    CHECK(!sv_app_view(app).active && sv_app_view(app).reason == SV_INPUT_OVERFLOW);
    CHECK(submit(context, sv_app_view(app)));
    puts(sv_result_text(sv_app_view(app).reason));
    puts("SV architecture native passed budget_frames=3 failure_frames=1");
    /* Leave the failed session in place: the real shell must continue rendering. */
    return 1;
}
