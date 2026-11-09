#include "ui/native-controls.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const int version[6] = {4,9,4,0,0,0};
typedef struct { SDL_Renderer *renderer; unsigned draws; uint32_t flags[32]; } Draw;
static bool draw(void *context, SvAppView view)
{
    Draw *state = context;
    assert(state->draws < 32);
    state->flags[state->draws++] = view.server_flags[0];
    return SDL_SetRenderDrawColor(state->renderer, (unsigned char)view.server_flags[0],0,0,255) &&
        SDL_RenderClear(state->renderer);
}
static void flags(SvApp *app, uint64_t generation, unsigned char value)
{
    const unsigned char bytes[] = {213,0,0,0,value,0,0,0,0,0,0,0,0,0,0,0,0};
    assert(sv_app_receive(app,generation,bytes,sizeof(bytes)) == SV_OK);
}
static SDL_Event text(uint64_t timestamp)
{
    SDL_Event event = {0}; event.type = SDL_EVENT_TEXT_INPUT;
    event.text.timestamp = timestamp; event.text.text = "x";
    return event;
}
static void pause_input_and_present(SvApp *app, SvNativeControls *controls, Draw *state)
{
    uint64_t gen = sv_app_view(app).generation;
    const unsigned char request[] = {184,0,0,0,7,'?',0};
    assert(sv_app_receive(app,gen,request,sizeof(request)) == SV_OK);
    assert(sv_app_step(app,1).processed == 1);
    assert(sv_app_accept_key(app,gen,sv_app_view(app).request.sequence,'z') == SV_OK);
    flags(app,gen,3);
    const unsigned char pause = 58;
    assert(sv_app_receive(app,gen,&pause,1) == SV_OK);
    flags(app,gen,4);
    uint64_t old = SDL_GetTicksNS();
    assert(sv_native_controls_step(controls,app,16).processed == 3);
    assert(state->draws == 1 && state->flags[0] == 3);
    assert(sv_app_view(app).server_flags[0] == 4 && sv_app_view(app).paused);
    assert(sv_app_view(app).request.pending);
    SDL_Event event = text(old);
    assert(sv_native_input(controls->input,app,&event));
    assert(controls->input->last_result == SV_STALE && sv_app_view(app).paused);
    assert(sv_app_dispatch_input(app,1).dispatched == 0);
    SDL_Delay(1);
    event = text(SDL_GetTicksNS());
    assert(sv_native_input(controls->input,app,&event));
    assert(controls->input->last_result == SV_OK && !sv_app_view(app).paused);
    assert(sv_app_view(app).request.pending); /* acknowledgement is not its answer */
    assert(sv_native_input(controls->input,app,&event));
    assert(controls->input->last_result == SV_STALE);
    unsigned char output[32];
    assert(sv_app_take_output(app,gen,output,sizeof(output)).result == SV_WAITING);
    /* A quantity draft survives pause and acknowledgement cannot edit it. */
    assert(sv_app_close(app) == SV_OK && sv_app_open(app,version) == SV_OK);
    gen = sv_app_view(app).generation;
    sv_native_input_begin(controls->input,app);
    assert(sv_app_begin_quantity(app,gen,1,99,false) == SV_OK);
    assert(sv_app_quantity_text(app,gen,"12") == SV_OK);
    assert(sv_app_receive(app,gen,&pause,1) == SV_OK);
    assert(sv_native_controls_step(controls,app,1).processed == 1);
    SDL_Delay(1); event = text(SDL_GetTicksNS());
    assert(sv_native_input(controls->input,app,&event) && controls->input->last_result == SV_OK);
    SvQuantity quantity = sv_app_quantity_view(app,gen);
    assert(quantity.active && !strcmp(quantity.field.bytes,"12"));
    /* Two packet occurrences require two fresh acknowledgements, even when
     * both decode in the same budget while servicing the network. */
    const unsigned char pauses[] = {58,58};
    assert(sv_app_receive(app,gen,pauses,sizeof(pauses)) == SV_OK);
    assert(sv_native_controls_step(controls,app,16).processed == 2);
    uint64_t sequence = sv_app_view(app).pause_sequence;
    SDL_Delay(1); event = text(SDL_GetTicksNS());
    assert(sv_native_input(controls->input,app,&event));
    assert(sv_app_view(app).paused && sv_app_view(app).pause_sequence == sequence+1);
    assert(sv_app_ack_pause(app,gen,sequence) == SV_STALE);
    assert(sv_native_input(controls->input,app,&event) && controls->input->last_result == SV_STALE);
    SDL_Delay(1); event = text(SDL_GetTicksNS());
    assert(sv_native_input(controls->input,app,&event) && !sv_app_view(app).paused);
}
static void flush_submission_and_pacing(SvApp *app, SvNativeControls *controls, Draw *state)
{
    assert(sv_app_open(app,version) == SV_OK);
    uint64_t gen = sv_app_view(app).generation;
    state->draws = 0;
    const unsigned char bytes[11] = {150,150,150,150,150,150,150,150,150,150,150};
    assert(sv_app_set_flush_options(app,(SvFlushOptions){0,1}) == SV_OK);
    assert(sv_app_receive(app,gen,bytes,sizeof(bytes)) == SV_OK);
    SvStep step = sv_native_controls_step(controls,app,16);
    assert(step.processed == 1 && state->draws == 1 && step.pending_bytes == 10);
    assert(sv_app_view(app).flush_due_ms == controls->frame_ms+1);
    /* The owner can receive more network bytes while pacing yields immediately. */
    flags(app,gen,9);
    step = sv_native_controls_step(controls,app,16);
    if (!step.processed) assert(step.result == SV_WAITING && step.pending_bytes > 0);
    while (sv_app_view(app).flush_sequence < 11) {
        SDL_Delay(2);
        sv_native_controls_step(controls,app,16);
    }
    assert(state->draws == 11 && sv_app_view(app).flush_due_ms == controls->frame_ms);
    assert(sv_app_view(app).server_flags[0] == 9);
    assert(sv_app_set_flush_options(app,(SvFlushOptions){1,1}) == SV_OK);
    assert(sv_app_receive(app,gen,bytes,sizeof(bytes)) == SV_OK);
    step = sv_native_controls_step(controls,app,16);
    assert(step.processed == 11 && state->draws == 22);
    assert(sv_app_view(app).flush_due_ms == controls->frame_ms);
    /* A partial trailing packet ends the outer processing batch; it must not
     * retain the old thin count indefinitely while waiting for more bytes. */
    const unsigned char partial[] = {150,146};
    assert(sv_app_receive(app,gen,partial,sizeof(partial)) == SV_OK);
    assert(sv_native_controls_step(controls,app,16).processed == 1);
    assert(!controls->batch);
    const unsigned char finish = 42;
    assert(sv_app_receive(app,gen,&finish,1) == SV_OK);
    assert(sv_native_controls_step(controls,app,16).processed == 1);
    assert(sv_app_close(app) == SV_OK);
    assert(sv_native_controls_step(controls,app,1).result == SV_CLOSED && controls->due_ns == 0);
}
int main(void)
{
    assert(SDL_Init(SDL_INIT_EVENTS));
    SDL_Surface *surface = SDL_CreateSurface(8,8,SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
    assert(renderer);
    SvApp *app = sv_app_create((SvAlertSink){0});
    assert(app && sv_app_open(app,version) == SV_OK);
    SvNativeInput input; sv_native_input_begin(&input,app);
    Draw state = {.renderer=renderer};
    SvNativeControls controls = {.renderer=renderer,.input=&input,.context=&state,.draw=draw};
    assert(sv_native_controls_bind(&controls,app) == SV_OK);
    pause_input_and_present(app,&controls,&state);
    flush_submission_and_pacing(app,&controls,&state);
    assert(sv_app_set_control_sink(app,(SvControlSink){0}) == SV_OK);
    assert(sv_app_destroy(app) == SV_OK);
    SDL_DestroyRenderer(renderer); SDL_DestroySurface(surface); SDL_Quit();
    puts("PASS: native pause freshness/request/quantity, ordered SDL submission and nonblocking flush pacing");
}
