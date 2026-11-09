#include "app.h"
#include <assert.h>
#include <string.h>

static const int version[6] = {4, 9, 4, 0, 0, 0};

static void control_packets_are_atomic_and_ordered(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    uint64_t owner;
    assert(sv_app_begin_confirmation(app, generation, &owner) == SV_OK);
    const unsigned char packets[] = {13, 11, 147, 213, 0, 0, 0, 1,
                                     0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4,
                                     146, 42, 146, 43};
    for (size_t i = 0; i < sizeof(packets); ++i) {
        assert(sv_app_receive(app, generation, &packets[i], 1) == SV_OK);
        SvStep step = sv_app_step(app, 20);
        assert(step.result == SV_OK || step.result == SV_WAITING);
    }
    SvAppView view = sv_app_view(app);
    assert(view.active && view.server_flags[0] == 1 && view.server_flags[3] == 4);
    unsigned char confirmed = 0;
    assert(sv_app_take_confirmation(app, generation, owner, &confirmed) == SV_OK && confirmed == 42);
    assert(sv_app_take_confirmation(app, generation, owner, &confirmed) == SV_OK && confirmed == 43);
    assert(sv_app_take_confirmation(app, generation, owner, &confirmed) == SV_WAITING);
    assert(sv_app_end_confirmation(app, generation, owner) == SV_OK);
    assert(sv_app_take_confirmation(app, generation, owner, &confirmed) == SV_STALE);
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
    assert(sv_app_send_ping(app,generation) == SV_OK);
    sent = sv_app_take_output(app,generation,packet,sizeof(packet));
    assert(sent.size == 15); packet[1] = 1;
    clock.microseconds = 550000;
    assert(sv_app_receive(app,generation,packet,sent.size) == SV_OK);
    assert(sv_app_step(app,1).processed == 1);
    unsigned char report[1024];
    const unsigned char request = 207;
    assert(sv_app_receive(app,generation,&request,1) == SV_OK);
    assert(sv_app_step(app,1).processed == 1);
    sent = sv_app_take_output(app,generation,report,sizeof(report));
    assert(sent.result == SV_OK && report[0] == request);
    size_t at = 1 + strlen((const char *)report+1)+1;
    at += strlen((const char *)report+at)+1;
    const unsigned char mean[] = {0,0,0,150};
    assert(!memcmp(report+at,mean,sizeof(mean))); /* mean of positive 100/200 */
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

static void character_setup_is_owned_by_session_generation(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    SvContactSetup setup = {.race_count = 1, .motd_size = 5};
    strcpy(setup.races[0].title, "Human");
    memcpy(setup.motd, "Hello", 5);
    assert(sv_app_set_character_setup(app, generation, &setup) == SV_OK);
    memset(&setup, 0, sizeof(setup));
    const SvContactSetup *stored = sv_app_character_setup(app, generation);
    assert(stored && stored->race_count == 1 && !strcmp(stored->races[0].title, "Human"));
    assert(sv_app_set_character_setup(app, generation, stored) == SV_INVALID);
    assert(sv_app_close(app) == SV_OK);
    assert(!sv_app_character_setup(app, generation));
    assert(sv_app_open(app, version) == SV_OK);
    assert(!sv_app_character_setup(app, sv_app_view(app).generation));
    assert(sv_app_destroy(app) == SV_OK);
}

/* C003 reports through the actual app/protocol dispatcher. Layout flags below
 * are independent expected branches, not calls to the production serializer. */
static int report_int(const unsigned char *bytes, size_t *at)
{
    uint32_t n = ((uint32_t)bytes[*at] << 24) | ((uint32_t)bytes[*at+1] << 16) |
        ((uint32_t)bytes[*at+2] << 8) | bytes[*at+3];
    *at += 4; return (int32_t)n;
}
static const char *report_string(const unsigned char *bytes, size_t *at, size_t size)
{
    assert(*at < size);
    const char *value = (const char *)bytes + *at;
    const char *end = memchr(value, 0, size - *at);
    assert(end && end - value < 80);
    *at += (size_t)(end - value) + 1; return value;
}
static void resource_reporting_layouts_and_lifecycle(void)
{
    const struct { int version[6]; unsigned layout; } cases[] = {
        {{4,8,0,0,0,0},0}, {{4,8,0,0,0,1},1},
        {{4,8,1,1,9,9},1}, {{4,8,1,2,0,0},1}, {{4,8,1,2,0,1},1},
        {{4,9,1,0,0,0},1},
        {{4,9,1,0,0,1},2}, {{4,9,2,0,0,0},2}, {{4,9,2,0,0,1},3},
        {{4,9,2,1,0,0},3}, {{4,9,2,1,0,1},4}, {{4,9,2,1,0,2},5}};
    SvApp *app = sv_app_create((SvAlertSink){0}); assert(app);
    for (unsigned c = 0; c < sizeof(cases)/sizeof(cases[0]); ++c)
    for (unsigned length = 78; length <= 80; ++length)
    for (unsigned enabled = 0; enabled <= 1; ++enabled) {
        assert(sv_app_open(app, cases[c].version) == SV_OK);
        uint64_t generation = sv_app_view(app).generation;
        SvSettingsAudioSnapshot audio = {0};
        audio.status = enabled ? SV_AUDIO_READY : SV_AUDIO_NO_DEVICE;
        audio.sfx = 1; audio.music = 2;
        char *names[] = {audio.sound_name,audio.sound_version,audio.music_name,audio.music_version};
        for (unsigned i = 0; i < 4; ++i) { memset(names[i], 'a'+i, length); names[i][length] = 0; }
        assert(sv_app_set_reporting_audio(app,generation,&audio) == SV_OK);
        SvSettingsAudioSnapshot invalid = audio;
        memset(invalid.sound_name,'x',sizeof(invalid.sound_name));
        assert(sv_app_set_reporting_audio(app,generation,&invalid) == SV_INVALID);
        const unsigned char request = 207;
        assert(sv_app_receive(app,generation,&request,1) == SV_OK);
        assert(sv_app_step(app,1).processed == 1);
        unsigned char bytes[1024];
        SvOutput output = sv_app_take_output(app,generation,bytes,sizeof(bytes));
        if (!cases[c].layout) { assert(output.result == SV_WAITING); continue; }
        assert(output.result == SV_OK && bytes[0] == request);
        size_t at = 1;
        assert(strstr(report_string(bytes,&at,output.size),"TomeNET"));
        assert(!strcmp(report_string(bytes,&at,output.size),"Linux"));
        if (cases[c].layout >= 2) assert(report_int(bytes,&at) == -1);
        if (cases[c].layout >= 3) {
            assert(report_int(bytes,&at) == -1); /* Guide absent, no I/O */
            if (cases[c].layout < 5) {
                assert(report_int(bytes,&at) == 0); assert(report_int(bytes,&at) == 0);
            }
            (void)report_string(bytes,&at,output.size); /* compiled client tag */
            assert(report_int(bytes,&at) == -1);
            if (cases[c].layout >= 4) for (unsigned i = 0; i < 4; ++i) {
                const char *reported = report_string(bytes,&at,output.size);
                size_t expected = enabled ? (length > 79 ? 79 : length) : 0;
                assert(strlen(reported) == expected && !memcmp(reported,names[i],expected));
                assert(strlen(names[i]) == length); /* full local bytes survive */
            }
        }
        assert(at == output.size);
        assert(sv_app_open(app,cases[c].version) == SV_OK);
        assert(sv_app_set_reporting_audio(app,generation,&audio) == SV_STALE);
        assert(sv_app_send_font(app,generation) == SV_STALE);
        generation = sv_app_view(app).generation;
        assert(sv_app_send_font(app,generation) == SV_OK);
        output = sv_app_take_output(app,generation,bytes,sizeof(bytes));
        const unsigned char missing[] = {208,0,0,'N','O','_','G','R','A','P','H','I','C','S',0,0};
        if (cases[c].version[1] == 8 && (cases[c].version[2] == 0 || cases[c].version[3] < 2))
            assert(output.result == SV_WAITING);
        else assert(output.size == sizeof(missing) && !memcmp(bytes,missing,sizeof(missing)));
    }
    /* Retry version request after output backpressure, exactly once. */
    assert(sv_app_open(app,version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    for (unsigned i = 0; i < 512; ++i) assert(sv_app_raw_key(app,generation,'x') == SV_OK);
    const unsigned char request = 207;
    assert(sv_app_receive(app,generation,&request,1) == SV_OK);
    assert(sv_app_step(app,1).result == SV_BACKPRESSURE);
    unsigned char bytes[1024];
    assert(sv_app_take_output(app,generation,bytes,sizeof(bytes)).size == 1024);
    assert(sv_app_step(app,1).processed == 1);
    assert(sv_app_take_output(app,generation,bytes,sizeof(bytes)).size > 1 && bytes[0] == request);
    assert(sv_app_step(app,1).result == SV_WAITING);
    assert(sv_app_take_output(app,generation,bytes,sizeof(bytes)).result == SV_WAITING);
    assert(sv_app_destroy(app) == SV_OK);
}

int main(void) {
    resource_reporting_layouts_and_lifecycle();
    control_packets_are_atomic_and_ordered();
    ping_echo_waits_for_whole_packet();
    pause_needs_fresh_ack_and_keeps_network_live();
    full_send_queue_waits_without_partial_ping_reply();
    idle_keepalive_is_independent_of_receive();
    pong_updates_bounded_lag_without_echo();
    flush_keeps_nonblocking_pacing_choices();
    character_setup_is_owned_by_session_generation();
}
