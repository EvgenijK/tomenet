#ifndef SV_NATIVE_INPUT_H
#define SV_NATIVE_INPUT_H
#include <SDL3/SDL.h>
#include "app.h"
/* Shared shell adapter: no widget interprets commands. Returns true if consumed. */
bool sv_native_input(SvApp *app, const SDL_Event *event);
#endif
