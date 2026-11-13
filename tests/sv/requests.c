#include "app.h"
#include "input/native-input.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void split_cancel_abort(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    unsigned char packet[85] = {184,0x80,0x01,0x02,0x03};
    const size_t lengths[] = {0,1,78,79};
    unsigned cases = 0;
    for (int layout = 1; layout <= 2; ++layout) {
        int version[] = {4,7,0,2,0,layout};
        for (unsigned l = 0; l < 4; ++l) {
            size_t length = lengths[l], size = 6 + length;
            memset(packet + 5, 0xff, length); packet[5 + length] = 0;
            for (size_t split = 0; split <= size; ++split) {
                assert(sv_app_open(app, version) == SV_OK);
                uint64_t gen = sv_app_view(app).generation;
                unsigned char out[1024];
                assert(sv_app_receive(app, gen, packet, split) == SV_OK);
                SvStep step = sv_app_step(app, 16);
                assert(step.processed == (split == size ? 1u : 0u));
                if (split < size) {
                    assert(!sv_app_view(app).request.pending);
                    assert(sv_app_view(app).context == SV_CONTEXT_GAME);
                    assert(sv_app_take_output(app, gen, out, sizeof(out)).result == SV_WAITING);
                }
                assert(sv_app_receive(app, gen, packet + split, size - split) == SV_OK);
                const unsigned char hp[] = {24,0,100,0,37,0};
                assert(sv_app_receive(app, gen, hp, layout == 1 ? 5 : 6) == SV_OK);
                sv_app_step(app, 16);
                SvAppView view = sv_app_view(app);
                assert(view.request.pending && view.request.id == (int)0x80010203u);
                assert(!memcmp(view.request.prompt, packet + 5, length + 1));
                assert(view.status.current == 37 && view.status.revision == 1);
                assert(sv_app_key(app, gen, view.request.sequence + 1, 'x') == SV_STALE);
                assert(sv_app_key(app, gen - 1, view.request.sequence, 'x') == SV_STALE);
                if (split % 2) {
                    const unsigned char abort[] = {188,188};
                    assert(sv_app_receive(app, gen, abort, sizeof(abort)) == SV_OK);
                    assert(sv_app_step(app, 16).processed == 2);
                } else assert(sv_app_key(app, gen, view.request.sequence, 27) == SV_OK);
                SvOutput reply = sv_app_take_output(app, gen, out, 5);
                assert(reply.result == SV_OUTPUT_TOO_SMALL && reply.size == 6);
                reply = sv_app_take_output(app, gen, out, sizeof(out));
                const unsigned char expected[] = {184,0x80,0x01,0x02,0x03,0};
                assert(reply.result == SV_OK && reply.size == 6 && !memcmp(out, expected, 6));
                view = sv_app_view(app);
                assert(!view.request.pending && !view.request.prompt[0] && !view.request.sequence);
                assert(view.context == SV_CONTEXT_GAME && view.status.current == 37);
                assert(sv_app_key(app, gen, 1, 'x') == SV_STALE);
                assert(sv_app_take_output(app, gen, out, sizeof(out)).result == SV_WAITING);
                ++cases;
            }
        }
    }
    assert(sv_app_destroy(app) == SV_OK);
    printf("SV request split/HP/cancel/abort cases=%u\n", cases);
}
static SvResult game_key(SvApp *app, uint64_t generation, unsigned char key)
{
    return sv_app_physical(app, generation, &key, 1);
}
static void c005_gameplay(void)
{
    SvApp *app = sv_app_create((SvAlertSink){0});
    const int version[] = {4,7,0,2,0,2};
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t generation = sv_app_view(app).generation;
    SvCommand command;
    unsigned char output[SV_PROTOCOL_CAPACITY];
    assert(game_key(app, generation, '4') == SV_OK);
    assert(game_key(app, generation, 'J') == SV_BACKPRESSURE);
    assert(sv_app_take_command(app, generation, &command) == SV_OK);
    assert(command.command == ';' && command.direction == 4);
    assert(sv_app_take_command(app, generation, &command) == SV_WAITING);
    assert(game_key(app, generation, 'J') == SV_OK);
    SvOutput packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 2 && output[0] == 156 && output[1] == 'J');
    /* Same production path with effective preferences, without parser-only proof. */
    SvSettingsOptions options;
    sv_settings_options_defaults(&options);
    assert(sv_settings_option_set(&options, "rogue_like_commands", true));
    SvPreferences *preferences = sv_preferences_create("/tmp", NULL, &options);
    assert(preferences);
    size_t cursor = 0;
    assert(sv_app_keymap_preferences(app, preferences, &cursor, false) == SV_OK);
    sv_preferences_destroy(preferences);
    assert(game_key(app, generation, 'h') == SV_OK);
    assert(sv_app_take_command(app, generation, &command) == SV_OK);
    assert(command.command == ';' && command.direction == 4);
    assert(game_key(app, generation, 'X') == SV_OK);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 2 && output[1] == 'X');
    assert(game_key(app, generation, ']') == SV_OK);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 2 && output[1] == ']');
    assert(sv_app_set_master_commands(app, generation, true) == SV_OK);
    assert(game_key(app, generation, ']') == SV_OK);
    assert(sv_app_take_command(app, generation, &command) == SV_OK && command.command == ']');
    for (unsigned i = 0; i < SV_PROTOCOL_CAPACITY / 2; ++i)
        assert(sv_app_raw_key(app, generation, 'Z') == SV_OK);
    assert(game_key(app, generation, '\\') == SV_WAITING);
    assert(game_key(app, generation, '^') == SV_WAITING);
    assert(game_key(app, generation, 'Z') == SV_BACKPRESSURE);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == sizeof(output));
    /* A denied byte restores prefix state; explicit retry produces one packet. */
    assert(game_key(app, generation, 'Z') == SV_OK);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 2 && output[0] == 156 && output[1] == 26);
    assert(game_key(app, generation, 27) == SV_OK);
    assert(game_key(app, generation, 13) == SV_OK);
    assert(game_key(app, generation, '-') == SV_OK);
    assert(game_key(app, generation, 10) == SV_OK);
    assert(sv_app_take_command(app, generation, &command) == SV_OK);
    assert(command.command == '+' && command.direction == 2);
    assert(game_key(app, generation, '`') == SV_OK);
    assert(sv_app_take_output(app, generation, output, sizeof(output)).result == SV_WAITING);
    assert(game_key(app, generation, '\\') == SV_WAITING);
    assert(game_key(app, generation, 10) == SV_OK);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 2 && output[0] == 156 && output[1] == 10);
    assert(game_key(app, generation, ' ') == SV_OK);
    SvNativeInput denied;
    sv_native_input_begin(&denied, app);
    SDL_Event held = {.type = SDL_EVENT_TEXT_INPUT}; held.text.text = "X";
    assert(sv_native_input(&denied, app, &held) && denied.last_result == SV_BACKPRESSURE);
    assert(sv_app_take_command(app, generation, &command) == SV_OK && command.command == ' ');
    assert(sv_native_input(&denied, app, &held) && denied.last_result == SV_OK);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 2 && output[1] == 'X');

    SvNativeInput native;
    sv_native_input_begin(&native, app);
    SDL_Event event = {.type = SDL_EVENT_TEXT_INPUT};
    event.text.text = "^";
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_WAITING);
    const unsigned char hp[] = {24,0,100,0,37,0};
    assert(sv_app_receive(app, generation, hp, sizeof(hp)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    assert(!sv_native_input(&native, app, &event));
    event.type = SDL_EVENT_WINDOW_RESIZED;
    assert(!sv_native_input(&native, app, &event));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_ESCAPE; event.key.scancode = SDL_SCANCODE_ESCAPE;
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_OK);
    assert(game_key(app, generation, 'h') == SV_OK);
    assert(sv_app_take_command(app, generation, &command) == SV_OK && command.command == ';');

    assert(sv_app_begin_quantity(app, generation, 3, 5000, false) == SV_OK);
    const unsigned char f1[] = {31,'_','F','F','B','E',13};
    assert(sv_app_bind_physical(app, f1, sizeof(f1), '7', SV_MACRO_COMMAND) == SV_OK);
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_F1; event.key.scancode = SDL_SCANCODE_F1;
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_WAITING);
    assert(sv_app_bind_physical(app, f1, sizeof(f1), '7', SV_MACRO_HYBRID) == SV_OK);
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_WAITING);
    assert(!strcmp(sv_app_quantity_view(app, generation).field.bytes, "3"));
    assert(sv_app_bind_physical(app, f1, sizeof(f1), '7', SV_MACRO_NORMAL) == SV_OK);
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_OK);
    assert(!strcmp(sv_app_quantity_view(app, generation).field.bytes, "7"));
    assert(sv_app_quantity_edit(app, generation, SV_EDIT_CANCEL) == SV_OK);
    int32_t value;
    assert(sv_app_take_quantity(app, generation, &value) == SV_OK && value == 0);
    assert(sv_app_begin_quantity(app, generation, 3, 5000, false) == SV_OK);
    assert(sv_app_view(app).context == SV_CONTEXT_QUANTITY);
    assert(sv_app_raw_key(app, generation, 'Z') == SV_BUSY);
    assert(sv_app_key(app, generation, 999, '5') == SV_STALE);
    const unsigned char request[] = {184,0,0,0,9,'?',0};
    assert(sv_app_receive(app, generation, request, sizeof(request)) == SV_OK);
    assert(sv_app_step(app, 1).processed == 1);
    assert(sv_app_quantity_text(app, generation, "9") == SV_BUSY);
    assert(sv_app_key(app, generation, sv_app_view(app).request.sequence, 'y') == SV_OK);
    assert(sv_app_view(app).context == SV_CONTEXT_QUANTITY);
    packet = sv_app_take_output(app, generation, output, sizeof(output));
    assert(packet.result == SV_OK && packet.size == 6 && output[0] == 184 && output[5] == 'y');
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "2k";
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_OK);
    assert(!strcmp(sv_app_quantity_view(app, generation).field.bytes, "2k"));
    event = (SDL_Event){.type = SDL_EVENT_KEY_DOWN};
    event.key.key = SDLK_RETURN; event.key.scancode = SDL_SCANCODE_RETURN;
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_OK);
    assert(sv_app_view(app).quantity_completed);
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "J";
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_BUSY);
    assert(sv_app_take_quantity(app, generation, &value) == SV_OK && value == 2000);
    assert(sv_app_take_quantity(app, generation, &value) == SV_WAITING);
    assert(sv_app_view(app).context == SV_CONTEXT_GAME);
    assert(sv_app_begin_quantity(app, generation, 3, 5, false) == SV_OK);
    assert(game_key(app, generation, 27) == SV_OK);
    assert(sv_app_take_quantity(app, generation, &value) == SV_OK && value == 0);
    assert(sv_app_begin_quantity(app, generation, 3, 5, false) == SV_OK);
    event = (SDL_Event){.type = SDL_EVENT_TEXT_INPUT}; event.text.text = "`";
    assert(sv_native_input(&native, app, &event) && native.last_result == SV_OK);
    assert(sv_app_take_quantity(app, generation, &value) == SV_OK && value == 0);
    assert(sv_app_take_output(app, generation, output, sizeof(output)).result == SV_WAITING);
    assert(sv_app_begin_quantity(app, generation, 3, 5, false) == SV_OK);
    assert(sv_app_open(app, version) == SV_OK);
    uint64_t next = sv_app_view(app).generation;
    assert(sv_app_take_quantity(app, generation, &value) == SV_STALE);
    assert(!sv_app_view(app).quantity_active && !sv_app_view(app).quantity_completed);
    assert(sv_app_take_quantity(app, next, &value) == SV_WAITING);
    assert(sv_app_take_command(app, generation, &command) == SV_STALE);
    assert(game_key(app, next, '^') == SV_WAITING);
    assert(sv_app_open(app, version) == SV_OK);
    next = sv_app_view(app).generation;
    assert(game_key(app, next, 'h') == SV_OK);
    assert(sv_app_take_command(app, next, &command) == SV_OK);
    assert(command.command == ';' && command.direction == 4);
    assert(sv_app_close(app) == SV_OK);
    assert(game_key(app, next, 'Z') == SV_CLOSED);
    assert(sv_app_destroy(app) == SV_OK);
    puts("SV C005 app/native keymap/raw/quantity passed");
}
int main(void)
{
    split_cancel_abort();
    c005_gameplay();
    {
        SvApp *raw = sv_app_create((SvAlertSink){0});
        const int version[] = {4,7,0,2,0,2};
        assert(raw && sv_app_open(raw,version) == SV_OK);
        uint64_t generation = sv_app_view(raw).generation;
        SvNativeInput native;
        sv_native_input_begin(&native, raw);
        SDL_Event event = {0};
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = "^";
        assert(sv_native_input(&native,raw,&event));
        unsigned char bytes[4];
        SvOutput output = sv_app_take_output(raw,generation,bytes,sizeof(bytes));
        assert(output.result == SV_WAITING);
        event.text.text = "z";
        assert(sv_native_input(&native,raw,&event));
        output = sv_app_take_output(raw,generation,bytes,sizeof(bytes));
        const unsigned char expected[] = {156,26};
        assert(output.result == SV_OK && output.size == sizeof(expected));
        assert(!memcmp(bytes,expected,sizeof(expected)));
        const unsigned char f1[] = {31,'_','F','F','B','E',13};
        assert(sv_app_bind_physical(raw,f1,sizeof(f1),26,SV_MACRO_NORMAL) == SV_OK);
        event = (SDL_Event){0};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_F1;
        event.key.scancode = SDL_SCANCODE_F1;
        assert(sv_native_input(&native,raw,&event));
        output = sv_app_take_output(raw,generation,bytes,sizeof(bytes));
        assert(output.result == SV_OK && output.size == sizeof(expected));
        assert(!memcmp(bytes,expected,sizeof(expected)));
        assert(sv_app_raw_key(raw,generation,27) == SV_INVALID);
        assert(sv_app_raw_key(raw,generation,'-') == SV_INVALID);
        assert(sv_app_raw_key(raw,generation,13) == SV_INVALID);
        assert(sv_app_take_output(raw,generation,bytes,sizeof(bytes)).result == SV_WAITING);
        assert(sv_app_destroy(raw) == SV_OK);
    }
    const int version[] = {4,7,0,2,0,2};
    const unsigned char request[] = {184,0x12,0x34,0x56,0x78,'K','e','y','?',0};
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app, version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    assert(sv_app_receive(app, gen, request, sizeof(request)) == SV_OK);
    assert(sv_app_step(app, 16).processed == 1);
    SvAppView view = sv_app_view(app);
    assert(view.request.pending && view.request.id == 0x12345678);
    assert(!strcmp((const char *)view.request.prompt, "Key?"));
    assert(view.context == SV_CONTEXT_KEY_REQUEST);
    const unsigned char up[] = {31,'_','F','F','5','2',13};
    assert(sv_app_bind_physical(app,up,sizeof(up),'Y',SV_MACRO_NORMAL) == SV_OK);
    SvNativeInput native;
    sv_native_input_begin(&native, app);
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_UP;
    event.key.scancode = SDL_SCANCODE_UP;
    assert(sv_native_input(&native,app,&event));
    unsigned char output[16];
    SvOutput reply = sv_app_take_output(app, gen, output, sizeof(output));
    const unsigned char expected[] = {184,0x12,0x34,0x56,0x78,'Y'};
    assert(reply.result == SV_OK && reply.size == sizeof(expected));
    assert(!memcmp(output, expected, sizeof(expected)));
    assert(sv_app_view(app).context == SV_CONTEXT_GAME);
    assert(!sv_app_view(app).request.pending);
    assert(sv_app_view(app).context == SV_CONTEXT_GAME);
    /* Unterminated fixed-size prompt must not publish a truncated request. */
    assert(sv_app_open(app, version) == SV_OK);
    gen = sv_app_view(app).generation;
    unsigned char bad[85];
    memset(bad, 'x', sizeof(bad)); bad[0] = 184;
    assert(sv_app_receive(app, gen, bad, sizeof(bad)) == SV_OK);
    assert(sv_app_step(app, 16).result == SV_DECODE_ERROR);
    assert(!sv_app_view(app).request.pending);
    assert(sv_app_destroy(app) == SV_OK);
    puts("SV key request checks passed");
}
