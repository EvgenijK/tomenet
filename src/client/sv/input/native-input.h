#ifndef SV_NATIVE_INPUT_H
#define SV_NATIVE_INPUT_H
#include <SDL3/SDL.h>
#include "app.h"
typedef struct { uint64_t generation, since_ns; } SvNativeInput;
/* Call immediately after session creation, before polling SDL input. Old events
 * retain SDL timestamps and cannot be rebound to the replacement session. */
void sv_native_input_begin(SvNativeInput *input, SvApp *app);
/* Shared shell adapter queues accepted input; no widget interprets commands. */
bool sv_native_input(SvNativeInput *input, SvApp *app, const SDL_Event *event);
#endif
