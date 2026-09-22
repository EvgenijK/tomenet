#include "app.h"
#include "ui/message-text.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const int version[6] = {4,7,0,2,0,2};
static void malformed_field_closes_without_publication(void)
{
    unsigned char bytes[263];
    memset(bytes, 'x', sizeof(bytes));
    bytes[0] = 46;
    bytes[257] = 0;
    memcpy(bytes + 258, (unsigned char[]){24,0,100,0,75}, 5);
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_receive(app, sv_app_view(app).generation, bytes, sizeof(bytes)) == SV_OK);
    assert(sv_app_step(app, 16).result == SV_DECODE_ERROR);
    SvAppView view = sv_app_view(app);
    assert(!view.active && !view.messages.count && !view.status.revision);
    assert(sv_app_destroy(app) == SV_OK);
}
static void fragmented_fields_keep_bytes_and_adjacent_hp(void)
{
    const size_t lengths[] = {0, 1, 254, 255};
    for (int layout = 0; layout < 2; ++layout) {
        int selected[6] = {4,7,0,2,0,layout + 1};
        for (size_t c = 0; c < sizeof(lengths) / sizeof(lengths[0]); ++c) {
            size_t length = lengths[c], packet = length + 2;
            unsigned char bytes[263] = {46};
            for (size_t i = 0; i < length; ++i) bytes[i + 1] = (unsigned char)(i % 255 + 1);
            bytes[length + 1] = 0;
            memcpy(bytes + packet, (unsigned char[]){24,0,100,0,75,1}, layout ? 6 : 5);
            for (size_t split = 0; split <= packet; ++split) {
                SvApp *app = sv_app_create((SvAlertSink){0});
                SvMessage event;
                assert(sv_app_open(app, selected) == SV_OK);
                uint64_t gen = sv_app_view(app).generation;
                assert(sv_app_receive(app, gen, bytes, split) == SV_OK);
                assert(sv_app_step(app, 16).processed == (split == packet ? 1 : 0));
                if (split < packet) {
                    assert(sv_app_take_message(app, gen, &event) == SV_WAITING);
                    assert(!sv_app_view(app).messages.count);
                    assert(sv_app_step(app, 16).processed == 0);
                }
                assert(sv_app_receive(app, gen, bytes + split, packet - split + (layout ? 6 : 5)) == SV_OK);
                sv_app_step(app, 16);
                assert(sv_app_take_message(app, gen, &event) == SV_OK);
                assert(event.sequence == 1 && event.length == length);
                assert(!memcmp(event.bytes, bytes + 1, length + 1));
                assert(sv_app_take_message(app, gen, &event) == SV_WAITING);
                SvAppView view = sv_app_view(app);
                assert(view.status.revision == 1 && view.status.current == 75);
                assert(view.status.drain == layout);
                assert(sv_app_destroy(app) == SV_OK);
            }
        }
    }
}
static void bounded_delivery_and_session_release(void)
{
    const unsigned char packet[] = {46,'x',0};
    SvApp *app = sv_app_create((SvAlertSink){0});
    SvMessage message;
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    for (size_t i = 0; i < SV_MESSAGE_PENDING; ++i) {
        assert(sv_app_receive(app, gen, packet, sizeof(packet)) == SV_OK);
        assert(sv_app_step(app, 1).result == SV_OK);
    }
    /* Reconstructed read views never consume required events, including those
     * older than the bounded live feed. */
    SvAppView snapshot = sv_app_view(app);
    assert(snapshot.messages.count == SV_MESSAGE_LINES);
    for (size_t i = 0; i < SV_MESSAGE_PENDING; ++i) {
        assert(sv_app_take_message(app, gen, &message) == SV_OK);
        assert(message.sequence == i + 1);
    }
    assert(sv_app_take_message(app, gen, &message) == SV_WAITING);
    assert(snapshot.messages.lines[0].sequence == SV_MESSAGE_PENDING - SV_MESSAGE_LINES + 1);
    for (size_t i = 0; i <= SV_MESSAGE_PENDING; ++i) {
        assert(sv_app_receive(app, gen, packet, sizeof(packet)) == SV_OK);
        SvStep step = sv_app_step(app, 1);
        assert(step.result == (i == SV_MESSAGE_PENDING ? SV_EVENT_OVERFLOW : SV_OK));
    }
    assert(!sv_app_view(app).active && sv_app_view(app).reason == SV_EVENT_OVERFLOW);
    assert(sv_app_take_message(app, gen, &message) == SV_CLOSED);
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_take_message(app, gen, &message) == SV_STALE);
    gen = sv_app_view(app).generation;
    assert(!sv_app_view(app).messages.count);
    assert(sv_app_take_message(app, gen, &message) == SV_WAITING);
    assert(sv_app_receive(app, gen, packet, sizeof(packet)) == SV_OK);
    sv_app_step(app, 1);
    assert(sv_app_close(app) == SV_OK); /* Pending events canceled with their session. */
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_take_message(app, sv_app_view(app).generation, &message) == SV_WAITING);
    assert(sv_app_destroy(app) == SV_OK);
}
static int count_alert(void *context, SvAlert alert)
{
    (void)alert;
    ++*(unsigned *)context;
    return 1;
}
static void messages_do_not_repeat_hp_effects(void)
{
    unsigned alerts = 0;
    const unsigned char bytes[] = {24,0,100,0,10,0,46,'x',0,46,'x',0};
    SvApp *app = sv_app_create((SvAlertSink){&alerts, count_alert});
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_set_alerts(app, (SvAlertOptions){.low_hp = 1}, (SvAttention){0}) == SV_OK);
    assert(sv_app_receive(app, sv_app_view(app).generation, bytes, sizeof(bytes)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 3);
    assert(alerts == 2 && sv_app_view(app).messages.count == 2);
    assert(sv_app_destroy(app) == SV_OK);
}
int main(void)
{
    const unsigned char bytes[] = {46,'H','i',0,46,'H','i',0,24,0,100,0,75,0};
    SvApp *app = sv_app_create((SvAlertSink){0});
    SvMessage first, second;
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    assert(sv_app_receive(app, generation, bytes, sizeof(bytes)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 3);
    assert(sv_app_view(app).status.current == 75);
    assert(sv_app_take_message(app, generation, &first) == SV_OK);
    assert(sv_app_take_message(app, generation, &second) == SV_OK);
    assert(first.sequence == 1 && second.sequence == 2);
    assert(first.length == 2 && !memcmp(first.bytes, "Hi", 3));
    assert(second.length == 2 && !memcmp(second.bytes, "Hi", 3));
    assert(sv_app_take_message(app, generation, &first) == SV_WAITING);
    assert(sv_app_view(app).messages.count == 2);
    SvMessageCache cache = {0};
    SvAppView before_clear = sv_app_view(app);
    assert(sv_messages_prepare(&cache, generation, before_clear.messages));
    assert(cache.count == 2 && !strcmp(cache.lines[0].text, "Hi"));
    assert(!sv_messages_prepare(&cache, generation, sv_app_view(app).messages));
    const unsigned char clear[] = {46,255,0};
    assert(sv_app_receive(app, generation, clear, sizeof(clear)) == SV_OK);
    assert(sv_app_step(app, 1).result == SV_OK);
    assert(sv_app_view(app).messages.count == 0);
    assert(sv_messages_prepare(&cache, generation, sv_app_view(app).messages));
    assert(cache.count == 0);
    assert(sv_messages_prepare(&cache, generation + 1, before_clear.messages));
    assert(cache.count == 2);
    assert(sv_app_take_message(app, generation, &first) == SV_OK);
    assert(first.sequence == 3 && first.length == 1 && first.bytes[0] == 255);
    assert(sv_app_destroy(app) == SV_OK);
    messages_do_not_repeat_hp_effects();
    fragmented_fields_keep_bytes_and_adjacent_hp();
    bounded_delivery_and_session_release();
    malformed_field_closes_without_publication();
    SvMessage formatted = {.length = 16, .bytes = {255,'r','A',255,'g','B',255,'.','C',255,'-','D',255,255,0x80,255}};
    SvMessageText text = sv_message_text(&formatted);
    assert(!strcmp(text.text, "ABCD{?"));
    assert(text.colors[0] == 4 && text.colors[1] == 5 && text.colors[2] == 4 && text.colors[3] == 4);
    assert(formatted.bytes[14] == 0x80 && formatted.bytes[15] == 255);
    puts("PASS: ordered message occurrences");
}
