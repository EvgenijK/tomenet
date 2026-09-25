#ifndef SV_PHYSICAL_H
#define SV_PHYSICAL_H
#include <SDL3/SDL.h>
typedef struct {
    unsigned char bytes[32];
    size_t size;
    SDL_Keycode key;
    SDL_Keymod modifiers;
} SvPhysicalKey;
/* SDL physical key -> baseline-compatible macro trigger bytes. Text input for
 * editable fields is handled separately by SDL_EVENT_TEXT_INPUT. */
bool sv_physical_key(const SDL_KeyboardEvent *event, SDL_Keymod latched,
                     SvPhysicalKey *output);
/* Decode the produced baseline trigger without losing its modifier prefix. */
int sv_physical_symbol(const unsigned char *bytes, size_t size);
#endif
