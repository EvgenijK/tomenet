#include "app.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const int version[] = {4,7,0,2,0,2};
static SvAppView request(SvApp *app, unsigned char id)
{
    const unsigned char packet[] = {184,0,0,0,id,'K','e','y','?',0};
    assert(sv_app_receive(app, sv_app_view(app).generation, packet, sizeof(packet)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 1);
    return sv_app_view(app);
}
static void expect_reply(SvApp *app, unsigned char id, unsigned char key)
{
    unsigned char bytes[16];
    const unsigned char expected[] = {184,0,0,0,id,key};
    SvOutput out = sv_app_take_output(app, sv_app_view(app).generation, bytes, sizeof(bytes));
    assert(out.result == SV_OK && out.size == sizeof(expected));
    assert(!memcmp(bytes, expected, sizeof(expected)));
    assert(sv_app_take_output(app, sv_app_view(app).generation, bytes, sizeof(bytes)).result == SV_WAITING);
}
static void macro_policy_and_order(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    assert(sv_app_bind_macro(app, 'a', 'b', SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_bind_macro(app, 'b', 'z', SV_MACRO_NORMAL) == SV_OK);
    assert(sv_app_bind_macro(app, 'h', 'Y', SV_MACRO_HYBRID) == SV_OK);
    assert(sv_app_bind_macro(app, 'c', 'N', SV_MACRO_COMMAND) == SV_OK);
    /* Backquote is MACRO_WAIT inside an expansion, not an Escape shortcut. */
    assert(sv_app_bind_macro(app, 'w', '`', SV_MACRO_NORMAL) == SV_INVALID);
    assert(sv_app_bind_macro(app, 'w', 30, SV_MACRO_NORMAL) == SV_INVALID);
    assert(sv_app_bind_macro(app, 128, 'Y', SV_MACRO_NORMAL) == SV_INVALID);
    const unsigned char keys[] = {'a','h','c','`'};
    const unsigned char expected[] = {'b','Y','c',0};
    for (unsigned i = 0; i < sizeof(keys); ++i) {
        SvAppView view = request(app, i + 1);
        assert(sv_app_accept_key(app, view.generation, view.request.sequence, keys[i]) == SV_OK);
        assert(sv_app_view(app).request.pending);
        SvInputStep step = sv_app_dispatch_input(app, 1);
        assert(step.dispatched == 1 && !step.stale && !step.pending && step.result == SV_OK);
        expect_reply(app, i + 1, expected[i]);
        assert(sv_app_view(app).context == SV_CONTEXT_GAME);
        assert(!sv_app_view(app).request.prompt[0]);
        assert(sv_app_dispatch_input(app, 16).dispatched == 0);
    }
    sv_app_destroy(app);
}
static void session_ownership_and_overflow(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    assert(sv_app_bind_macro(app, 'm', 'Y', SV_MACRO_NORMAL) == SV_OK);
    SvAppView old = request(app, 1);
    assert(sv_app_accept_key(app, old.generation, old.request.sequence, 'm') == SV_OK);
    /* A second accepted key may not answer a different request after completion. */
    assert(sv_app_accept_key(app, old.generation, old.request.sequence, 'N') == SV_OK);
    assert(sv_app_dispatch_input(app, 1).pending == 1);
    expect_reply(app, 1, 'Y');
    SvAppView next = request(app, 2);
    SvInputStep step = sv_app_dispatch_input(app, 1);
    assert(!step.dispatched && step.stale == 1 && !step.pending);
    assert(sv_app_view(app).request.id == 2 && sv_app_view(app).request.pending);
    const unsigned char messages[] = {46,'E',0,46,'E',0};
    assert(sv_app_receive(app, next.generation, messages, sizeof(messages)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 2);
    assert(sv_app_accept_key(app, next.generation, next.request.sequence, 'm') == SV_OK);
    assert(sv_app_close(app) == SV_OK);
    assert(sv_app_open(app, version) == SV_OK);
    SvAppView fresh = request(app, 1); /* Same wire id and sequence, new owner. */
    assert(fresh.request.sequence == old.request.sequence && fresh.generation != old.generation);
    assert(sv_app_accept_key(app, old.generation, old.request.sequence, 'x') == SV_STALE);
    assert(sv_app_key(app, old.generation, old.request.sequence, 'x') == SV_STALE);
    assert(sv_app_dispatch_input(app, 16).dispatched == 0);
    SvMessage event;
    assert(sv_app_take_message(app, fresh.generation, &event) == SV_WAITING);
    assert(!sv_app_view(app).messages.count);
    assert(sv_app_accept_key(app, fresh.generation, fresh.request.sequence, 'm') == SV_OK);
    assert(sv_app_dispatch_input(app, 1).dispatched == 1); /* Preferences survive. */
    expect_reply(app, 1, 'Y');
    fresh = request(app, 3);
    for (size_t i = 0; i < SV_KEY_PENDING; ++i)
        assert(sv_app_accept_key(app, fresh.generation, fresh.request.sequence, 'm') == SV_OK);
    assert(sv_app_accept_key(app, fresh.generation, fresh.request.sequence, 'm') == SV_KEY_OVERFLOW);
    assert(!sv_app_view(app).active && !sv_app_view(app).request.pending);
    assert(sv_app_dispatch_input(app, 16).result == SV_CLOSED);
    sv_app_destroy(app);
}
static void mandatory_overflow_and_abort(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    SvAppView view = request(app, 1);
    assert(sv_app_accept_key(app, view.generation, view.request.sequence, 'Y') == SV_OK);
    const unsigned char abort[] = {188};
    assert(sv_app_receive(app, view.generation, abort, sizeof(abort)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 1);
    assert(sv_app_dispatch_input(app, 16).stale == 1);
    expect_reply(app, 1, 0);
    /* Mandatory messages remain bounded while a request and queued input exist. */
    view = request(app, 2);
    const unsigned char message[] = {46,'E',0};
    for (size_t i = 0; i < SV_MESSAGE_PENDING; ++i) {
        assert(sv_app_receive(app, view.generation, message, sizeof(message)) == SV_OK);
        assert(sv_app_step(app, 1).result == SV_OK);
    }
    assert(sv_app_accept_key(app, view.generation, view.request.sequence, 'Y') == SV_OK);
    assert(sv_app_receive(app, view.generation, message, sizeof(message)) == SV_OK);
    assert(sv_app_step(app, 1).result == SV_EVENT_OVERFLOW);
    assert(!sv_app_view(app).active && !sv_app_view(app).request.pending);
    assert(sv_app_dispatch_input(app, 1).result == SV_CLOSED);
    assert(sv_app_open(app, version) == SV_OK);
    /* 170 six-byte replies fit the 1024-byte transport; the next must fail. */
    for (unsigned i = 0; i < 170; ++i) {
        view = request(app, i);
        assert(sv_app_accept_key(app, view.generation, view.request.sequence, 'Y') == SV_OK);
        assert(sv_app_dispatch_input(app, 1).dispatched == 1);
    }
    view = request(app, 171);
    assert(sv_app_accept_key(app, view.generation, view.request.sequence, 'Y') == SV_OK);
    assert(sv_app_dispatch_input(app, 1).result == SV_OUTPUT_OVERFLOW);
    assert(!sv_app_view(app).active && !sv_app_view(app).request.pending);
    sv_app_destroy(app);
}
int main(void)
{
    macro_policy_and_order();
    session_ownership_and_overflow();
    mandatory_overflow_and_abort();
    puts("PASS: input and session lifecycle contracts");
}
