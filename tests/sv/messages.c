#include "app.h"
#include "files/message-sinks.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
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

static size_t read_file(const char *name, char *out, size_t capacity)
{
    FILE *file = fopen(name, "r"); assert(file);
    size_t size = fread(out, 1, capacity - 1, file);
    assert(!ferror(file) && !fclose(file));
    out[size] = 0;
    return size;
}
static void receive_text(SvApp *app, const char *text)
{
    unsigned char packet[SV_MESSAGE_BYTES + 1] = {46};
    size_t size = strlen(text) + 1;
    assert(size <= SV_MESSAGE_BYTES);
    memcpy(packet + 1, text, size);
    assert(sv_app_receive(app, sv_app_view(app).generation, packet, size + 1) == SV_OK);
    assert(sv_app_step(app, 1).result == SV_OK);
}
static void file_notice(void *context, SvMessageFileFailure kind)
{
    unsigned *notices = context;
    ++notices[kind];
}
static void production_message_file_effects(void)
{
    char root[] = "/tmp/sv-message-files-XXXXXX", notes[1024], clone[1024], data[4096];
    assert(mkdtemp(root));
    snprintf(notes, sizeof(notes), "%s/user/notes-Alice.txt", root);
    snprintf(clone, sizeof(clone), "%s/user/stdout.txt", root);
    FILE *console = tmpfile(); assert(console);
    unsigned notices[3] = {0};
    SvMessageFiles files;
    assert(sv_message_files_begin(&files, root, "Alice", console, file_notice, notices));
    assert(!sv_message_files_begin(&files, root, "../escape", console, file_notice, notices));
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(sv_app_set_message_sink(app, sv_message_files_sink(&files)) == SV_OK);
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    /* First note: six routing/colour bytes, then Note from. Input deliberately
     * contains all special markers, coloured payload and a broken final marker. */
    const char note[] = "\375\376\377w\377wNote from Alice: \374hi\375\376\377r!\377";
    unsigned char packet[SV_MESSAGE_BYTES + 1] = {46};
    size_t n = strlen(note);
    memcpy(packet + 1, note, n + 1);
    assert(sv_app_receive(app, gen, packet, 8) == SV_OK);
    assert(sv_app_step(app, 2).processed == 0);
    assert(access(notes, F_OK) && access(clone, F_OK));
    assert(sv_app_receive(app, gen, packet + 8, n + 2 - 8) == SV_OK);
    assert(sv_app_step(app, 1).result == SV_OK);
    size_t before = read_file(notes, data, sizeof(data));
    assert(strstr(data, "] Alice: hi!\n"));
    assert(access(clone, F_OK) && ftell(console) == 0); /* Both defaults off. */
    receive_text(app, "\377w  same\377r!\377");
    receive_text(app, "\377w  same\377r!\377");
    read_file(notes, data, sizeof(data));
    char *first = strstr(data, "]   same!\n"); assert(first);
    assert(strstr(first + 1, "]   same!\n")); /* Identical real events preserved. */
    receive_text(app, "ordinary");
    before = read_file(notes, data, sizeof(data));
    receive_text(app, "\377w  no longer a note");
    assert(read_file(notes, data, sizeof(data)) == before);
    receive_text(app, note);
    receive_text(app, note); /* Header during chain uses baseline offset 2. */
    before = read_file(notes, data, sizeof(data));
    assert(strstr(data, "] Note from Alice: hi!\n"));
    /* Neither read views, observer work nor acknowledging events repeats effects. */
    for (int i = 0; i < 3; ++i) {
        SvMessageCache redraw = {0};
        assert(sv_messages_prepare(&redraw, gen, sv_app_view(app).messages));
        assert(sv_app_step(app, 1).processed == 0);
    }
    SvMessage message;
    while (sv_app_take_message(app, gen, &message) == SV_OK) {}
    assert(read_file(notes, data, sizeof(data)) == before);
    sv_message_files_options(&files, true, true);
    receive_text(app, "\375\376\377rABC\377");
    assert(read_file(clone, data, sizeof(data)) == 4 && !strcmp(data, "ABC\n"));
    rewind(console);
    assert(fread(data, 1, sizeof(data), console) == 4 && !memcmp(data, "ABC\n", 4));
    receive_text(app, "\377"); /* Clear-topline has no clone output. */
    assert(read_file(clone, data, sizeof(data)) == 4);
    /* Session reset discards the note chain but does not truncate shared files. */
    receive_text(app, note);
    before = read_file(notes, data, sizeof(data));
    assert(sv_app_open(app, version) == SV_OK);
    assert(sv_app_receive(app, gen, packet, n + 2) == SV_STALE);
    receive_text(app, "\377w  after relog");
    assert(read_file(notes, data, sizeof(data)) == before);
    assert(sv_message_files_begin(&files, root, "Bob", console, file_notice, notices));
    receive_text(app, "\377w  after account switch");
    char bob[1024]; snprintf(bob, sizeof(bob), "%s/user/notes-Bob.txt", root);
    assert(access(bob, F_OK));
    receive_text(app, note);
    read_file(bob, data, sizeof(data)); assert(strstr(data, "] Alice: hi!\n"));
    /* Real append-open and flush failures remain optional and independently
     * reported. /dev/full supplies a Linux flush failure, not a fake writer. */
    FILE *full = fopen("/dev/full", "w"); assert(full);
    files.console = full;
    sv_message_files_options(&files, true, true);
    assert(!unlink(clone) && !mkdir(clone, 0700));
    assert(!unlink(bob) && !mkdir(bob, 0700));
    receive_text(app, note);
    assert(notices[SV_MESSAGE_FILE_NOTES] == 1);
    assert(notices[SV_MESSAGE_FILE_CONSOLE] == 1);
    assert(notices[SV_MESSAGE_FILE_CLONE] == 1);
    assert(sv_app_view(app).active && sv_app_view(app).executor_failed);
    assert(!rmdir(bob) && !symlink("/dev/full", bob));
    sv_message_files_options(&files, false, false);
    receive_text(app, note); /* Notes append succeeds, fclose flush fails. */
    assert(notices[SV_MESSAGE_FILE_NOTES] == 2);
    assert(!unlink(bob));
    assert(sv_app_destroy(app) == SV_OK);
    fclose(full); fclose(console);
    assert(!rmdir(clone) && !unlink(notes));
    char user[1024]; snprintf(user, sizeof(user), "%s/user", root);
    assert(!rmdir(user) && !rmdir(root));
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
    production_message_file_effects();
    puts("PASS: ordered message occurrences and production file sinks");
}
