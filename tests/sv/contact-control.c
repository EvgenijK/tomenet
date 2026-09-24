#include "app.h"
#include <assert.h>
#include <string.h>

static const int version[6] = {4, 9, 4, 0, 0, 0};

static void control_packets_are_atomic_and_ordered(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    const unsigned char packets[] = {13, 11, 147, 213, 0, 0, 0, 1,
                                     0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4,
                                     146, 42};
    for (size_t i = 0; i < sizeof(packets); ++i) {
        assert(sv_app_receive(app, generation, &packets[i], 1) == SV_OK);
        SvStep step = sv_app_step(app, 20);
        assert(step.result == SV_OK || step.result == SV_WAITING);
    }
    SvAppView view = sv_app_view(app);
    assert(view.active && view.server_flags[0] == 1 && view.server_flags[3] == 4);
    unsigned char confirmed = 0;
    assert(sv_app_take_confirmation(app, generation, &confirmed) == SV_OK && confirmed == 42);
    assert(sv_app_take_confirmation(app, generation, &confirmed) == SV_WAITING);
    unsigned char output[32];
    assert(sv_app_take_output(app, generation, output, sizeof(output)).result == SV_WAITING);
    assert(sv_app_destroy(app) == SV_OK);
}

static void ping_echo_waits_for_whole_packet(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    const unsigned char ping[] = {166, 0, 0, 0, 0, 7, 0, 0, 0, 8,
                                  0, 0, 0, 9, 'x', 'y', 0, 13};
    for (size_t i = 0; i < sizeof(ping) - 1; ++i) {
        assert(sv_app_receive(app, generation, &ping[i], 1) == SV_OK);
        SvStep step = sv_app_step(app, 1);
        assert(step.result == SV_OK || step.result == SV_WAITING);
        unsigned char output[32];
        SvOutput sent = sv_app_take_output(app, generation, output, sizeof(output));
        if (i < 16) assert(sent.result == SV_WAITING);
        else {
            assert(sent.result == SV_OK && sent.size == 17);
            assert(output[1] == 1 && !memcmp(output + 2, ping + 2, 15));
        }
    }
    assert(sv_app_destroy(app) == SV_OK);
}

static void pause_needs_fresh_ack_and_keeps_network_live(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    const unsigned char pause_and_flags[] = {58, 213, 0, 0, 0, 1,
                                             0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4};
    assert(sv_app_receive(app, generation, pause_and_flags, sizeof(pause_and_flags)) == SV_OK);
    assert(sv_app_step(app, 2).processed == 2);
    SvAppView view = sv_app_view(app);
    assert(view.paused && view.server_flags[3] == 4);
    assert(sv_app_raw_key(app, generation, 'x') == SV_BUSY);
    assert(sv_app_ack_pause(app, generation, view.pause_sequence + 1) == SV_STALE);
    assert(sv_app_ack_pause(app, generation, view.pause_sequence) == SV_OK);
    assert(!sv_app_view(app).paused);
    assert(sv_app_destroy(app) == SV_OK);
}

static void full_send_queue_waits_without_partial_ping_reply(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    for (int i = 0; i < 512; ++i)
        assert(sv_app_raw_key(app, generation, 'a') == SV_OK);
    assert(sv_app_raw_key(app, generation, 'a') == SV_BACKPRESSURE);
    const unsigned char ping[] = {166, 0, 0, 0, 0, 7, 0, 0, 0, 8,
                                  0, 0, 0, 9, 'x', 0};
    assert(sv_app_receive(app, generation, ping, sizeof(ping)) == SV_OK);
    SvStep step = sv_app_step(app, 1);
    assert(step.result == SV_BACKPRESSURE && step.pending_bytes == sizeof(ping));
    assert(sv_app_view(app).active);
    unsigned char output[1024];
    SvOutput sent = sv_app_take_output(app, generation, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == sizeof(output));
    for (size_t i = 0; i < sizeof(output); i += 2)
        assert(output[i] == 156 && output[i + 1] == 'a');
    assert(sv_app_step(app, 1).processed == 1);
    sent = sv_app_take_output(app, generation, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == sizeof(ping));
    assert(output[1] == 1 && !memcmp(output + 2, ping + 2, sizeof(ping) - 2));
    assert(sv_app_destroy(app) == SV_OK);
}

static void idle_keepalive_is_independent_of_receive(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    assert(sv_app_keepalive(app, generation, 2999, 1000) == SV_WAITING);
    assert(sv_app_keepalive(app, generation, 3000, 1000) == SV_OK);
    assert(sv_app_keepalive(app, generation, 3001, 1000) == SV_WAITING);
    unsigned char output[16];
    SvOutput sent = sv_app_take_output(app, generation, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == 1 && output[0] == 13);
    assert(sv_app_destroy(app) == SV_OK);
}

typedef struct { int32_t seconds, microseconds; } Clock;
static void fixed_time(void *context, int32_t *seconds, int32_t *microseconds)
{
    Clock *clock = context;
    *seconds = clock->seconds;
    *microseconds = clock->microseconds;
}
static void pong_updates_bounded_lag_without_echo(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    Clock clock = {10, 250000};
    assert(sv_app_set_ping_clock(app, (SvPingClock){&clock, fixed_time}) == SV_OK);
    assert(sv_app_send_ping(app, generation) == SV_OK);
    unsigned char packet[32];
    SvOutput sent = sv_app_take_output(app, generation, packet, sizeof(packet));
    assert(sent.result == SV_OK && sent.size == 15);
    const unsigned char expected[] = {166, 0, 0, 0, 0, 1, 0, 0, 0, 10,
                                      0, 3, 208, 144, 0};
    assert(!memcmp(packet, expected, sizeof(expected)));
    clock.microseconds = 350000;
    packet[1] = 1;
    assert(sv_app_receive(app, generation, packet, sent.size) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    SvAppView view = sv_app_view(app);
    assert(view.ping.samples[0] == 100 && view.ping.average_ms == 100);
    assert(sv_app_take_output(app, generation, packet, sizeof(packet)).result == SV_WAITING);
    packet[2] = packet[3] = packet[4] = packet[5] = 255;
    assert(sv_app_receive(app, generation, packet, sent.size) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_view(app).ping.samples[0] == 100);
    assert(sv_app_destroy(app) == SV_OK);
}

static void flush_keeps_nonblocking_pacing_choices(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    assert(sv_app_set_flush_options(app, (SvFlushOptions){0, 1}) == SV_OK);
    assert(sv_app_frame(app, generation, 100) == SV_OK);
    const unsigned char flush[11] = {150, 150, 150, 150, 150, 150,
                                     150, 150, 150, 150, 150};
    assert(sv_app_receive(app, generation, flush, sizeof(flush)) == SV_OK);
    assert(sv_app_step(app, 10).processed == 10);
    assert(sv_app_view(app).flush_sequence == 10 && sv_app_view(app).flush_due_ms == 101);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_view(app).flush_sequence == 11 && sv_app_view(app).flush_due_ms == 100);
    assert(sv_app_set_flush_options(app, (SvFlushOptions){1, 1}) == SV_OK);
    assert(sv_app_frame(app, generation, 200) == SV_OK);
    assert(sv_app_receive(app, generation, flush, 1) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_view(app).flush_due_ms == 200);
    assert(sv_app_destroy(app) == SV_OK);
}

int main(void) {
    control_packets_are_atomic_and_ordered();
    ping_echo_waits_for_whole_packet();
    pause_needs_fresh_ack_and_keeps_network_live();
    full_send_queue_waits_without_partial_ping_reply();
    idle_keepalive_is_independent_of_receive();
    pong_updates_bounded_lag_without_echo();
    flush_keeps_nonblocking_pacing_choices();
}
