#include "input/native-input.h"
#include "input/key-encoding.h"
static bool modifier(SDL_Scancode code)
{
    return code == SDL_SCANCODE_LCTRL || code == SDL_SCANCODE_RCTRL ||
           code == SDL_SCANCODE_LSHIFT || code == SDL_SCANCODE_RSHIFT ||
           code == SDL_SCANCODE_LALT || code == SDL_SCANCODE_RALT;
}
void sv_native_input_begin(SvNativeInput *input, SvApp *app)
{
    *input = (SvNativeInput){.generation = sv_app_view(app).generation,
                             .since_ns = SDL_GetTicksNS(), .last_result = SV_WAITING};
}
void sv_native_input_pause(SvNativeInput *input, SvAppView view, uint64_t now_ns)
{
    input->generation = view.generation;
    input->pause_sequence = view.pause_sequence;
    input->since_ns = now_ns;
    input->latch = 0;
    SDL_FlushEvents(SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
    SDL_FlushEvent(SDL_EVENT_TEXT_INPUT);
}
bool sv_native_input(SvNativeInput *input, SvApp *app, const SDL_Event *event)
{
    SvAppView view = sv_app_view(app);
    input->last_result = SV_WAITING;
    if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) { input->latch = 0; return false; }
    if (event->type != SDL_EVENT_TEXT_INPUT && event->type != SDL_EVENT_KEY_DOWN) return false;
    /* Fail closed if lifecycle forgot to install the new session's input epoch. */
    if (input->generation != view.generation ||
        (event->common.timestamp && event->common.timestamp < input->since_ns)) {
        input->last_result = SV_STALE;
        return true;
    }
    if (view.paused) {
        /* The native frame owner establishes this boundary after successful
         * presentation. Unknown timestamps cannot prove fresh pause input. */
        if (input->pause_sequence != view.pause_sequence ||
            !event->common.timestamp || event->common.timestamp <= input->since_ns) {
            input->last_result = SV_STALE;
            return true;
        }
        if (event->type == SDL_EVENT_TEXT_INPUT && (!event->text.text || !*event->text.text))
            return true;
        if (event->type == SDL_EVENT_KEY_DOWN && event->key.repeat) return true;
        if (event->type == SDL_EVENT_KEY_DOWN && modifier(event->key.scancode)) return true;
        /* Printable KEY_DOWN is paired with TEXT_INPUT; acknowledge only once,
         * then reject its queued tail using a new timestamp boundary. */
        if (event->type == SDL_EVENT_KEY_DOWN && event->key.key >= 32 &&
            event->key.key <= 255 && !(event->key.mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT)))
            return true;
        input->last_result = sv_app_ack_pause(app, view.generation, view.pause_sequence);
        if (input->last_result == SV_OK) {
            sv_native_input_pause(input, sv_app_view(app), SDL_GetTicksNS());
        }
        return true;
    }
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        if (!view.request.pending && (view.quantity_active || view.quantity_completed)) {
            if (view.quantity_active)
                input->last_result = sv_app_quantity_text(app, view.generation, event->text.text);
            else input->last_result = SV_BUSY;
            return true;
        }
        /* UTF-8 field editing is owned by the endpoint adapter; a server
         * one-key request and raw gameplay dispatch accept one protocol byte. */
        const unsigned char *text = (const unsigned char *)event->text.text;
        if (!text || !text[0] || text[0] >= 128 || text[1]) return true;
        if (view.request.pending)
            input->last_result = sv_app_accept_key(app, view.generation, view.request.sequence, text[0]);
        else input->last_result = sv_app_physical(app, view.generation, text, 1);
        return true;
    }
    if (event->key.repeat) return true;
    SDL_Scancode scancode = event->key.scancode;
#ifdef SDL3_STICKY_KEYS
    if (modifier(scancode)) {
        if (scancode == SDL_SCANCODE_LCTRL || scancode == SDL_SCANCODE_RCTRL) input->latch |= SDL_KMOD_CTRL;
        if (scancode == SDL_SCANCODE_LSHIFT || scancode == SDL_SCANCODE_RSHIFT) input->latch |= SDL_KMOD_SHIFT;
        if (scancode == SDL_SCANCODE_LALT || scancode == SDL_SCANCODE_RALT) input->latch |= SDL_KMOD_ALT;
        return true;
    }
#else
    if (modifier(scancode)) return true;
#endif
    SvPhysicalKey physical;
    if (!sv_physical_key(&event->key, input->latch, &physical)) return true;
    input->latch = 0;
    if (!view.request.pending && (view.quantity_active || view.quantity_completed)) {
        if (!view.quantity_active) { input->last_result = SV_BUSY; return true; }
        SvEditKey edit;
        bool control = (physical.modifiers & SDL_KMOD_CTRL) != 0;
        switch (physical.key) {
        case SDLK_ESCAPE: edit = SV_EDIT_CANCEL; break;
        case SDLK_RETURN: case SDLK_KP_ENTER: edit = SV_EDIT_ACCEPT; break;
        case SDLK_LEFT: edit = control ? SV_EDIT_WORD_LEFT : SV_EDIT_LEFT; break;
        case SDLK_RIGHT: edit = control ? SV_EDIT_WORD_RIGHT : SV_EDIT_RIGHT; break;
        case SDLK_HOME: edit = SV_EDIT_HOME; break;
        case SDLK_END: edit = SV_EDIT_END; break;
        case SDLK_BACKSPACE: edit = control ? SV_EDIT_WORD_DELETE : SV_EDIT_BACKSPACE; break;
        case SDLK_DELETE: edit = SV_EDIT_DELETE; break;
        default:
            /* Printable editing arrives as SDL_TEXT_INPUT; other physical
             * triggers retain the existing normal field-binding route. */
            if (physical.key >= 32 && physical.key <= 255 &&
                !(physical.modifiers & (SDL_KMOD_CTRL | SDL_KMOD_ALT))) return true;
            input->last_result = sv_app_physical(app, view.generation,
                physical.bytes, physical.size);
            return true;
        }
        input->last_result = sv_app_quantity_edit(app, view.generation, edit);
        return true;
    }
    /* Layout-dependent printable keys are delivered by SDL_TEXT_INPUT. */
    if (physical.key >= 32 && physical.key <= 255 &&
        !(physical.modifiers & (SDL_KMOD_CTRL | SDL_KMOD_ALT))) return true;
    input->last_result = sv_app_physical(app, view.generation, physical.bytes, physical.size);
    return true;
}
