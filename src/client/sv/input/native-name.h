#ifndef SV_INPUT_NATIVE_NAME_H
#define SV_INPUT_NATIVE_NAME_H
#include "input/login-interaction.h"
#include <SDL3/SDL.h>

SvLoginInputResult sv_native_name_event(SvLoginInteraction *input,
                                        const SDL_Event *event,
                                        SvTextResult *text_result,
                                        bool *clipboard_unavailable);
#endif
