#include "input/native-input.h"
#include "input/physical.h"
static bool modifier(SDL_Scancode code)
{
    return code == SDL_SCANCODE_LCTRL || code == SDL_SCANCODE_RCTRL ||
           code == SDL_SCANCODE_LSHIFT || code == SDL_SCANCODE_RSHIFT ||
           code == SDL_SCANCODE_LALT || code == SDL_SCANCODE_RALT;
}
void sv_native_input_begin(SvNativeInput *input, SvApp *app)
{
    *input = (SvNativeInput){.generation = sv_app_view(app).generation,
                             .since_ns = SDL_GetTicksNS()};
}
bool sv_native_input(SvNativeInput *input, SvApp *app, const SDL_Event *event)
{
    SvAppView view = sv_app_view(app);
    if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) { input->latch = 0; return false; }
    if (event->type != SDL_EVENT_TEXT_INPUT && event->type != SDL_EVENT_KEY_DOWN) return false;
    /* Fail closed if lifecycle forgot to install the new session's input epoch. */
    if (input->generation != view.generation ||
        (event->common.timestamp && event->common.timestamp < input->since_ns)) return true;
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        /* UTF-8 field editing is owned by the endpoint adapter; a server
         * one-key request and raw gameplay dispatch accept one protocol byte. */
        const unsigned char *text = (const unsigned char *)event->text.text;
        if (!text || !text[0] || text[0] >= 128 || text[1]) return true;
        if (view.request.pending)
            (void)sv_app_accept_key(app, view.generation, view.request.sequence, text[0]);
        else (void)sv_app_physical(app, view.generation, text, 1);
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
    if (!view.request.pending && physical.key == SDLK_ESCAPE) return false;
    /* Layout-dependent printable keys are delivered by SDL_TEXT_INPUT. */
    if (physical.key >= 32 && physical.key <= 255 &&
        !(physical.modifiers & (SDL_KMOD_CTRL | SDL_KMOD_ALT))) return true;
    (void)sv_app_physical(app, view.generation, physical.bytes, physical.size);
    return true;
}
