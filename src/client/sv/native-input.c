#include "native-input.h"
bool sv_native_input(SvApp *app, const SDL_Event *event)
{
    SvAppView view = sv_app_view(app);
    if (!view.request.pending) return false;
    unsigned char key;
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        /* Stage A's declared ASCII input profile: one protocol byte per request. */
        const unsigned char *text = (const unsigned char *)event->text.text;
        if (!text || !text[0] || text[0] >= 128 || text[1]) return true;
        key = text[0];
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.repeat) return true;
        switch (event->key.key) {
        case SDLK_ESCAPE: key = 27; break;
        case SDLK_RETURN: case SDLK_KP_ENTER: key = 13; break;
        case SDLK_TAB: key = 9; break;
        case SDLK_BACKSPACE: key = 8; break;
        default:
            if ((event->key.mod & SDL_KMOD_CTRL) && event->key.key >= SDLK_A && event->key.key <= SDLK_Z)
                key = (unsigned char)(event->key.key - SDLK_A + 1);
            else return true; /* Printable keys arrive as text, with layout and shift applied. */
        }
    } else return false;
    (void)sv_app_key(app, view.generation, view.request.sequence, key);
    return true;
}
