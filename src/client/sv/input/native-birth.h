#ifndef SV_INPUT_NATIVE_BIRTH_H
#define SV_INPUT_NATIVE_BIRTH_H
#include "input/birth-interaction.h"
#include <SDL3/SDL.h>

#define SV_BIRTH_GUIDE_PLACEHOLDER_TEXT "The guide is in development"
#define SV_BIRTH_CHOICE_ROW_TOP 320
#define SV_BIRTH_CHOICE_ROW_HEIGHT 40
#define SV_BIRTH_CHOICE_TEXT_INSET 20

typedef struct SvBirthViewport {
    SvBirthOwner owner;
    unsigned first, rows;
    int logical_height;
    int highlighted_identity;
} SvBirthViewport;

void sv_birth_viewport_sync(SvBirthViewport *viewport, const SvBirth *birth,
                            int logical_height);

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                         uint64_t since_ns,
                                         SvBirthViewport *viewport,
                                         const SDL_Event *event);

#endif
