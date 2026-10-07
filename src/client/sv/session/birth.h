#ifndef SV_SESSION_BIRTH_H
#define SV_SESSION_BIRTH_H
#include "result.h"
#include "session/character-setup.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SV_BIRTH_NONE,
    SV_BIRTH_SEX,
    SV_BIRTH_CLASS
} SvBirthOwner;

typedef enum {
    SV_BIRTH_SEX_NONE = -1,
    SV_BIRTH_SEX_MALE,
    SV_BIRTH_SEX_FEMALE
} SvBirthSex;

typedef enum {
    SV_BIRTH_MOVE_PREVIOUS,
    SV_BIRTH_MOVE_NEXT,
    SV_BIRTH_SELECT_SEX,
    SV_BIRTH_RANDOM,
    SV_BIRTH_DNA,
    SV_BIRTH_CONFIRM,
    SV_BIRTH_QUIT
} SvBirthCommandKind;

typedef struct {
    SvBirthCommandKind kind;
    SvBirthSex sex;
} SvBirthCommand;

typedef enum {
    SV_BIRTH_INPUT_IGNORED,
    SV_BIRTH_INPUT_HANDLED,
    SV_BIRTH_INPUT_CONFIRMED,
    SV_BIRTH_INPUT_QUIT
} SvBirthInputResult;

typedef struct {
    uint64_t generation;
    SvBirthOwner owner;
    SvBirthSex highlighted_sex, chosen_sex;
    SvBirthSex dna_sex;
    bool dna_available, auto_reincarnation;
    bool entered, keyboard_navigated, mouse_navigated;
    bool random_used, dna_used, illegal_ignored, confirmed, quit;
    uint32_t random_state;
    const SvContactSetup *setup;
} SvBirth;

typedef struct SvBirthView {
    SvBirthOwner owner;
    SvBirthSex highlighted_sex;
    bool dna_available;
} SvBirthView;

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, bool dna_available,
                        SvBirthSex dna_sex);
SvBirthInputResult sv_birth_command(SvBirth *birth, uint64_t generation,
                                    SvBirthCommand command);
void sv_birth_view_prepare(SvBirthView *view, const SvBirth *birth);

#endif
