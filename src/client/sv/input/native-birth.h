#ifndef SV_INPUT_NATIVE_BIRTH_H
#define SV_INPUT_NATIVE_BIRTH_H
#include "session/birth.h"
#include <SDL3/SDL.h>

#define SV_BIRTH_GUIDE_PLACEHOLDER_TEXT "The guide is in development"

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                          const SDL_Event *event);

#endif
