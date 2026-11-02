#ifndef SV_INPUT_NATIVE_BIRTH_H
#define SV_INPUT_NATIVE_BIRTH_H
#include "input/birth-interaction.h"
#include <SDL3/SDL.h>

#define SV_BIRTH_CHOICE_ROW_TOP 320
#define SV_BIRTH_CHOICE_ROW_HEIGHT 40
#define SV_BIRTH_CHOICE_TEXT_INSET 20

typedef struct SvBirthViewport {
    SvBirthOwner owner;
    unsigned first, rows;
    int logical_height, logical_width;
    unsigned columns, summary_scroll, detail_scroll, summary_max, detail_max;
    int hover, focus;
    bool choice_keyboard_focus;
    int highlighted_identity;
    int held_stat, held_direction;
    uint64_t held_generation, repeat_at_ms;
} SvBirthViewport;

unsigned sv_birth_choice_count(const SvBirthView *view);
int sv_birth_choice_id(const SvBirthView *view,unsigned position);
int sv_birth_highlight(const SvBirthView *view);
bool sv_birth_choice_legal(const SvBirthView *view,unsigned position);
const char *sv_birth_choice_title(const SvBirthView *view,unsigned position);
const char *sv_birth_mode_title(SvBirthMode mode);
bool sv_birth_action_available(const SvBirthView *view,SvBirthCommandKind kind);

void sv_birth_viewport_sync(SvBirthViewport *viewport, const SvBirth *birth,
                            int logical_height);

/* Called by the scene each frame; repeats one adjustment after the hold delay. */
SvBirthInputResult sv_native_birth_tick(SvBirth *birth,uint64_t generation,
                                       SvBirthViewport *viewport,uint64_t now_ms);
void sv_native_birth_release(SvBirthViewport *viewport);

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                         uint64_t since_ns,
                                         SvBirthViewport *viewport,
                                         const SDL_Event *event);

#endif
