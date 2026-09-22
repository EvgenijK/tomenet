#include "app.h"
#include "ui/status.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int version[6] = {4, 7, 0, 2, 0, 2};
static void budget_preserves_updates(void)
{
    const unsigned char hp[] = {24,0,100,0,50,0, 24,0,100,0,10,0, 24,0,100,0,50,0};
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    assert(sv_app_receive(app, generation, hp, sizeof(hp)) == SV_OK);
    assert(!sv_app_view(app).status.available);
    SvStep step = sv_app_step(app, 1);
    assert(step.processed == 1 && step.pending_bytes == 12);
    assert(sv_app_view(app).status.current == 50);
    step = sv_app_step(app, 1);
    assert(step.processed == 1 && step.pending_bytes == 6);
    assert(sv_app_view(app).status.current == 10);
    step = sv_app_step(app, 1);
    assert(step.processed == 1 && step.pending_bytes == 0);
    assert(sv_app_view(app).status.current == 50);
    assert(sv_app_view(app).status.revision == 3);
    assert(sv_app_step(app, 1).processed == 0);
    sv_app_destroy(app);
}
typedef struct { SvAlert items[16]; size_t count; int fail; } Alerts;
static int deliver(void *context, SvAlert alert)
{
    Alerts *alerts = context;
    assert(alerts->count < 16);
    alerts->items[alerts->count++] = alert;
    return !alerts->fail;
}
static void alerts_survive_intermediate_and_repeated_hp(void)
{
    const unsigned char hp[] = {24,0,100,0,50,0, 24,0,100,0,10,0,
                               24,0,100,0,10,255, 24,0,100,0,50,0};
    Alerts alerts = {0};
    SvApp *app = sv_app_create((SvAlertSink){&alerts, deliver});
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_set_alerts(app, (SvAlertOptions){1,1}, (SvAttention){1}) == SV_OK);
    assert(sv_app_receive(app, sv_app_view(app).generation, hp, sizeof(hp)) == SV_OK);
    assert(sv_app_step(app, 4).processed == 4);
    assert(sv_app_view(app).status.current == 50);
    assert(alerts.count == 5);
    assert(alerts.items[0] == SV_DAMAGE_SOUND);
    assert(alerts.items[1] == SV_LOW_HP_SOUND && alerts.items[2] == SV_LOW_HP_NOTICE);
    assert(alerts.items[3] == SV_LOW_HP_SOUND && alerts.items[4] == SV_LOW_HP_NOTICE);
    alerts.count = 0; alerts.fail = 1;
    assert(sv_app_receive(app, sv_app_view(app).generation, hp + 6, 6) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_view(app).active && sv_app_view(app).executor_failed);
    assert(alerts.count == 3); /* One failed effect must not swallow following effects. */
    alerts.count = 0;
    assert(sv_app_set_alerts(app, (SvAlertOptions){0,0}, (SvAttention){1}) == SV_OK);
    assert(sv_app_receive(app, sv_app_view(app).generation, hp + 6, 6) == SV_OK);
    sv_app_step(app, 1);
    assert(alerts.count == 0);
    sv_app_destroy(app);
}
static void alert_options_attention_and_drain_are_independent(void)
{
    static const struct {
        SvAlertOptions options;
        SvAttention attention;
        unsigned char drain, current;
        unsigned count;
        SvAlert expected[3];
    } cases[] = {
        {{1,1}, {1}, 255, 10, 2, {SV_LOW_HP_SOUND, SV_LOW_HP_NOTICE}},
        {{0,1}, {1},   0, 10, 1, {SV_DAMAGE_SOUND}},
        {{1,0}, {1},   0, 10, 2, {SV_LOW_HP_SOUND, SV_LOW_HP_NOTICE}},
        {{1,1}, {0},   0, 10, 2, {SV_LOW_HP_SOUND, SV_LOW_HP_NOTICE}},
        {{0,1}, {0},   0, 10, 0, {0}},
        {{1,1}, {1},   0, 10, 3, {SV_DAMAGE_SOUND, SV_LOW_HP_SOUND, SV_LOW_HP_NOTICE}},
        {{1,0}, {0},   0, 20, 0, {0}},
        {{1,0}, {0},   0, 19, 2, {SV_LOW_HP_SOUND, SV_LOW_HP_NOTICE}}
    };
    Alerts alerts = {0};
    SvApp *app = sv_app_create((SvAlertSink){&alerts, deliver});
    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        const unsigned char initial[] = {24,0,100,0,50,0};
        unsigned char hp[] = {24,0,100,0,cases[i].current,cases[i].drain};
        assert(sv_app_open(app, version) == SV_OK);
        sv_app_set_alerts(app, cases[i].options, cases[i].attention);
        uint64_t gen = sv_app_view(app).generation;
        assert(sv_app_receive(app, gen, initial, sizeof(initial)) == SV_OK);
        assert(sv_app_step(app, 1).processed == 1);
        alerts.count = 0;
        assert(sv_app_receive(app, gen, hp, sizeof(hp)) == SV_OK);
        assert(sv_app_step(app, 1).processed == 1);
        assert(alerts.count == cases[i].count);
        for (unsigned j = 0; j < cases[i].count; ++j) assert(alerts.items[j] == cases[i].expected[j]);
    }
    sv_app_destroy(app);
}
static unsigned released;
static void release_bytes(void *owner) { ++released; free(owner); }
static void stale_completion_cannot_touch_reopened_session(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t old = sv_app_view(app).generation;
    unsigned char *bytes = malloc(6);
    memcpy(bytes, (unsigned char[]){24,0,100,0,10,0}, 6);
    SvOwnedBytes input = {old, bytes, 6, bytes, release_bytes};
    assert(sv_app_close(app) == SV_OK);
    assert(sv_app_receive(app, old, bytes, 6) == SV_CLOSED);
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_view(app).generation != old);
    assert(sv_app_receive_owned(app, &input) == SV_STALE);
    assert(released == 1 && !input.release && !input.bytes);
    assert(sv_app_step(app, 1).processed == 0);
    assert(!sv_app_view(app).status.available);
    unsigned char output[32];
    assert(sv_app_take_output(app, old, output, sizeof(output)).result == SV_STALE);
    sv_app_destroy(app);
}
static void presentation_tracks_data_and_layout_independently(void)
{
    SvStatusCache cache = {0};
    SvPresentationKey key = {1024, 768, 1.0f, 1};
    SvAppView view = {.generation=1, .active=1,
                     .status={100,50,0,0,0,1,1}};
    assert(sv_status_prepare(&cache, view, key) == (SV_TEXT_CHANGED | SV_LAYOUT_CHANGED));
    assert(!strcmp(cache.text, "HP 50 / 100"));
    assert(sv_status_prepare(&cache, view, key) == 0);
    key.width = 1200;
    assert(sv_status_prepare(&cache, view, key) == SV_LAYOUT_CHANGED);
    key.scale = 1.5f;
    assert(sv_status_prepare(&cache, view, key) == SV_LAYOUT_CHANGED);
    ++key.font_revision;
    assert(sv_status_prepare(&cache, view, key) == SV_LAYOUT_CHANGED);
    view.status.current = 10; ++view.status.revision;
    assert(sv_status_prepare(&cache, view, key) == SV_TEXT_CHANGED);
    assert(!strcmp(cache.text, "HP 10 / 100"));
    ++view.generation; view.status.current = 80; /* Same revision, different session. */
    assert(sv_status_prepare(&cache, view, key) & SV_TEXT_CHANGED);
    assert(!strcmp(cache.text, "HP 80 / 100"));
}
static void backpressure_retains_owned_input_for_retry(void)
{
    const unsigned char hp[] = {24,0,100,0,75,0};
    unsigned char batch[1020];
    for (size_t i = 0; i < sizeof(batch); i += sizeof(hp)) memcpy(batch+i, hp, sizeof(hp));
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    assert(sv_app_receive(app, gen, batch, sizeof(batch)) == SV_OK);
    assert(sv_app_receive_capacity(app) == 4);
    unsigned before = released;
    unsigned char *bytes = malloc(sizeof(hp));
    memcpy(bytes, hp, sizeof(hp));
    SvOwnedBytes input = {gen, bytes, sizeof(hp), bytes, release_bytes};
    assert(sv_app_receive_owned(app, &input) == SV_BACKPRESSURE);
    assert(released == before && input.bytes == bytes);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_receive_owned(app, &input) == SV_OK);
    assert(released == before + 1 && !input.release);
    assert(sv_app_step(app, 200).processed == 170);
    assert(sv_app_view(app).status.revision == 171);
    assert(sv_app_view(app).status.current == 75 && sv_app_receive_capacity(app) == 1024);
    sv_app_destroy(app);
}
static void partial_input_and_recovery_remain_atomic(void)
{
    const unsigned char hp[] = {24,0,100,0,75,0};
    const unsigned char unknown[] = {7,24,0,100,0,10,0};
    const unsigned char expected[] = {211,0,0,0,7,0,0,0,24,111,0};
    SvApp *app = sv_app_create((SvAlertSink){0});
    for (int layout = 1; layout <= 2; ++layout) {
        int v[] = {4,7,0,2,0,layout};
        size_t length = layout == 1 ? 5 : 6;
        for (size_t split = 0; split < length; ++split) {
            assert(sv_app_open(app, v) == SV_OK);
            uint64_t gen = sv_app_view(app).generation;
            assert(sv_app_receive(app, gen, hp, split) == SV_OK);
            SvStep step = sv_app_step(app, 5);
            assert(step.result == SV_WAITING && step.processed == 0);
            assert(!sv_app_view(app).status.available);
            assert(sv_app_step(app, 5).processed == 0);
            assert(sv_app_receive(app, gen, hp+split, length-split) == SV_OK);
            assert(sv_app_step(app, 5).processed == 1);
            assert(sv_app_view(app).status.current == 75 && sv_app_view(app).status.revision == 1);
        }
    }
    uint64_t gen = sv_app_view(app).generation;
    assert(sv_app_receive(app, gen, unknown, sizeof(unknown)) == SV_OK);
    assert(sv_app_step(app, 5).result == SV_RECOVERED);
    assert(sv_app_view(app).status.current == 75 && sv_app_view(app).status.revision == 1);
    unsigned char output[32];
    SvOutput result = sv_app_take_output(app, gen, output, 2);
    assert(result.result == SV_OUTPUT_TOO_SMALL && result.size == 11);
    result = sv_app_take_output(app, gen, output, sizeof(output));
    assert(result.result == SV_OK && result.size == sizeof(expected));
    assert(!memcmp(output, expected, sizeof(expected)));
    assert(sv_app_take_output(app, gen, output, sizeof(output)).result == SV_WAITING);
    assert(sv_app_receive(app, gen, hp, sizeof(hp)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_view(app).status.revision == 2);
    sv_app_destroy(app);
}
static void mandatory_output_exhaustion_closes_only_session(void)
{
    const unsigned char unknown = 7;
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    for (unsigned i = 0; i < 93; ++i) {
        assert(sv_app_receive(app, gen, &unknown, 1) == SV_OK);
        assert(sv_app_step(app, 1).result == SV_RECOVERED);
    }
    assert(sv_app_receive(app, gen, &unknown, 1) == SV_OK);
    assert(sv_app_step(app, 1).result == SV_OUTPUT_OVERFLOW);
    assert(!sv_app_view(app).active && sv_app_view(app).reason == SV_OUTPUT_OVERFLOW);
    assert(sv_app_open(app, version) == SV_OK);
    unsigned char overflow[1025] = {0};
    assert(sv_app_receive(app, sv_app_view(app).generation, overflow, sizeof(overflow)) == SV_INPUT_OVERFLOW);
    assert(!sv_app_view(app).active && sv_app_view(app).reason == SV_INPUT_OVERFLOW);
    sv_app_destroy(app);
}
typedef struct { SvApp *app; unsigned attempts; } Reentry;
static int reentrant_executor(void *context, SvAlert alert)
{
    Reentry *r = context;
    unsigned char output[32];
    (void)alert;
    assert(sv_app_view(r->app).status.current == 10); /* Whole update is visible. */
    uint64_t gen = sv_app_view(r->app).generation;
    assert(sv_app_close(r->app) == SV_BUSY);
    assert(sv_app_destroy(r->app) == SV_BUSY);
    assert(sv_app_open(r->app, version) == SV_BUSY);
    assert(sv_app_receive(r->app, gen, NULL, 0) == SV_BUSY);
    assert(sv_app_step(r->app, 1).result == SV_BUSY);
    assert(sv_app_set_alerts(r->app, (SvAlertOptions){0}, (SvAttention){0}) == SV_BUSY);
    assert(sv_app_take_output(r->app, gen, output, sizeof(output)).result == SV_BUSY);
    ++r->attempts;
    return 1;
}
static void executors_cannot_reenter_model_mutation(void)
{
    const unsigned char hp[] = {24,0,100,0,10,0};
    Reentry reentry = {0};
    reentry.app = sv_app_create((SvAlertSink){&reentry, reentrant_executor});
    assert(sv_app_open(reentry.app, version) == SV_OK);
    sv_app_set_alerts(reentry.app, (SvAlertOptions){1,1}, (SvAttention){1});
    assert(sv_app_receive(reentry.app, sv_app_view(reentry.app).generation, hp, sizeof(hp)) == SV_OK);
    assert(sv_app_step(reentry.app, 1).processed == 1);
    assert(reentry.attempts == 2 && sv_app_view(reentry.app).status.revision == 1);
    sv_app_destroy(reentry.app);
}
int main(void)
{
    stale_completion_cannot_touch_reopened_session();
    presentation_tracks_data_and_layout_independently();
    backpressure_retains_owned_input_for_retry();
    partial_input_and_recovery_remain_atomic();
    mandatory_output_exhaustion_closes_only_session();
    executors_cannot_reenter_model_mutation();
    budget_preserves_updates();
    alerts_survive_intermediate_and_repeated_hp();
    alert_options_attention_and_drain_are_independent();
    puts("PASS: SV architecture contracts");
    return 0;
}
