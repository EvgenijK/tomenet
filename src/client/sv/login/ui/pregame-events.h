#ifndef SV_PREGAME_EVENTS_H
#define SV_PREGAME_EVENTS_H
#include <SDL3/SDL.h>

/* Keyboard timestamps come from the native backend and need not share the
 * SDL tick clock. Retire pending input at owner boundaries instead of judging
 * freshly polled keyboard/text events against scene creation time. */
static inline bool sv_pregame_keyboard_event(const SDL_Event *event)
{
    return event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP ||
           event->type == SDL_EVENT_TEXT_INPUT || event->type == SDL_EVENT_TEXT_EDITING;
}

static inline void sv_pregame_input_boundary(void)
{
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
    SDL_FlushEvent(SDL_EVENT_TEXT_INPUT);
    SDL_FlushEvent(SDL_EVENT_TEXT_EDITING);
}
#endif
