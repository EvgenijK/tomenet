#include "app.h"
#include <assert.h>
#include <string.h>

static const int version[6] = {4, 9, 4, 0, 0, 0};

static void payload_free_controls_keep_the_following_packet(void)
{
    SvProtocol *protocol = sv_protocol_create(version);
    assert(protocol);
    /* END and KEYPRESS carry no payload. The following flags are a sentinel. */
    const unsigned char bytes[] = {11, 147, 213,
        0,0,0,1, 0,0,0,2, 0,0,0,3, 0,0,0,4};
    SvChange change;
    assert(sv_protocol_receive(protocol, bytes, sizeof(bytes)) == SV_OK);
    assert(sv_protocol_next(protocol, &change) == SV_OK && change.kind == SV_CHANGE_NOOP);
    assert(sv_protocol_next(protocol, &change) == SV_OK && change.kind == SV_CHANGE_NOOP);
    assert(sv_protocol_next(protocol, &change) == SV_OK && change.kind == SV_CHANGE_SERVER_FLAGS);
    assert(change.server_flags[0] == 1 && change.server_flags[3] == 4);
    assert(sv_protocol_next(protocol, &change) == SV_WAITING);
    unsigned char output[32];
    assert(sv_protocol_output(protocol, output, sizeof(output)).result == SV_WAITING);

    const unsigned char partial[] = {146};
    change = (SvChange){.kind = SV_CHANGE_NOOP};
    assert(sv_protocol_receive(protocol, partial, sizeof(partial)) == SV_OK);
    assert(sv_protocol_next(protocol, &change) == SV_WAITING);
    assert(change.kind == SV_CHANGE_NOOP && sv_protocol_pending(protocol) == 1);
    const unsigned char tail[] = {255, 11};
    assert(sv_protocol_receive(protocol, tail, sizeof(tail)) == SV_OK);
    assert(sv_protocol_next(protocol, &change) == SV_OK);
    assert(change.kind == SV_CHANGE_CONFIRM && change.confirmed_command == 255);
    assert(sv_protocol_next(protocol, &change) == SV_OK && change.kind == SV_CHANGE_NOOP);
    assert(!sv_protocol_pending(protocol));
    sv_protocol_destroy(protocol);
}

static void recovery_waits_atomically_and_publishes_one_diagnostic(void)
{
    SvProtocol *protocol = sv_protocol_create(version);
    assert(protocol);
    SvChange change;
    const unsigned char prefix = 147;
    assert(sv_protocol_receive(protocol, &prefix, 1) == SV_OK);
    assert(sv_protocol_next(protocol, &change) == SV_OK);
    for (unsigned i = 0; i < SV_PROTOCOL_CAPACITY / 2 - 5; ++i)
        assert(sv_protocol_raw_key(protocol, 'x') == SV_OK);
    assert(sv_protocol_output_capacity(protocol) == 10);
    /* The known following packet is deliberately discarded by recovery. */
    const unsigned char bytes[] = {0, 11, 147};
    assert(sv_protocol_receive(protocol, bytes, sizeof(bytes)) == SV_OK);
    change = (SvChange){.kind = SV_CHANGE_NOOP};
    assert(sv_protocol_next(protocol, &change) == SV_BACKPRESSURE);
    assert(change.kind == SV_CHANGE_NOOP);
    assert(sv_protocol_pending(protocol) == sizeof(bytes));
    assert(sv_protocol_output_capacity(protocol) == 10);
    unsigned char output[SV_PROTOCOL_CAPACITY];
    SvOutput sent = sv_protocol_output(protocol, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == SV_PROTOCOL_CAPACITY - 10);
    for (size_t i = 0; i < sent.size; i += 2)
        assert(output[i] == 156 && output[i + 1] == 'x');
    assert(sv_protocol_next(protocol, &change) == SV_RECOVERED);
    assert(change.kind == SV_CHANGE_MESSAGE);
    assert(!strcmp((const char *)change.message.bytes,
        "\377RReceived unknown packet type (0, 147), dropping"));
    assert(change.message.length == strlen((const char *)change.message.bytes));
    assert(!sv_protocol_pending(protocol));
    sent = sv_protocol_output(protocol, output, sizeof(output));
    const unsigned char expected[] = {211,0,0,0,0,0,0,0,147,111,0};
    assert(sent.result == SV_OK && sent.size == sizeof(expected));
    assert(!memcmp(output, expected, sizeof(expected)));
    assert(sv_protocol_next(protocol, &change) == SV_WAITING);
    assert(sv_protocol_output(protocol, output, sizeof(output)).result == SV_WAITING);
    sv_protocol_destroy(protocol);
}

static void recovery_reaches_the_session_and_does_not_replay(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    const unsigned char batch[] = {11, 0, 147};
    assert(sv_app_receive(app, generation, batch, sizeof(batch)) == SV_OK);
    assert(sv_app_step(app, 8).processed == 2);
    SvMessages messages = sv_app_view(app).messages;
    assert(messages.count == 1 && messages.lines[0].sequence == 1);
    assert(!strcmp((const char *)messages.lines[0].bytes,
        "\377RReceived unknown packet type (0, 11), dropping"));
    unsigned char output[32];
    const unsigned char expected[] = {211,0,0,0,0,0,0,0,11,111,0};
    SvOutput sent = sv_app_take_output(app, generation, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == sizeof(expected));
    assert(!memcmp(output, expected, sizeof(expected)));
    assert(sv_app_step(app, 8).processed == 0);
    assert(sv_app_view(app).messages.revision == messages.revision);
    assert(sv_app_take_output(app, generation, output, sizeof(output)).result == SV_WAITING);
    assert(sv_app_close(app) == SV_OK && sv_app_open(app, version) == SV_OK);
    assert(sv_app_receive(app, generation, batch, sizeof(batch)) == SV_STALE);
    assert(!sv_app_view(app).messages.count);
    assert(sv_app_destroy(app) == SV_OK);
}

int main(void)
{
    payload_free_controls_keep_the_following_packet();
    recovery_waits_atomically_and_publishes_one_diagnostic();
    recovery_reaches_the_session_and_does_not_replay();
}
