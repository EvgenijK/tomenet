#include "session/birth.h"

static bool legal_sex(SvBirthSex sex)
{
    return sex == SV_BIRTH_SEX_MALE || sex == SV_BIRTH_SEX_FEMALE;
}

static SvBirthInputResult confirm(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_SEX || !legal_sex(birth->highlighted_sex))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_sex = birth->highlighted_sex;
    birth->owner = SV_BIRTH_CLASS;
    birth->confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, bool dna_available,
                        SvBirthSex dna_sex)
{
    if (!birth || !generation || !setup || (dna_available && !legal_sex(dna_sex)))
        return SV_INVALID;
    *birth = (SvBirth){.generation = generation, .owner = SV_BIRTH_SEX,
        .highlighted_sex = SV_BIRTH_SEX_MALE, .chosen_sex = SV_BIRTH_SEX_NONE,
        .dna_sex = dna_sex, .dna_available = dna_available, .entered = true,
        .random_state = (uint32_t)(generation ^ 0x9e3779b9u), .setup = setup};
    return SV_OK;
}

SvBirthInputResult sv_birth_command(SvBirth *birth, uint64_t generation,
                                    SvBirthCommand command)
{
    if (!birth || generation != birth->generation || birth->quit)
        return SV_BIRTH_INPUT_IGNORED;
    if (birth->owner != SV_BIRTH_SEX) {
        if (command.kind == SV_BIRTH_QUIT) {
            birth->quit = true;
            return SV_BIRTH_INPUT_QUIT;
        }
        return SV_BIRTH_INPUT_IGNORED;
    }
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
    case SV_BIRTH_MOVE_NEXT:
        birth->highlighted_sex = birth->highlighted_sex == SV_BIRTH_SEX_MALE ?
            SV_BIRTH_SEX_FEMALE : SV_BIRTH_SEX_MALE;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_SEX:
        if (!legal_sex(command.sex)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_sex = command.sex;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_RANDOM:
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_sex = (birth->random_state & 1u) ?
            SV_BIRTH_SEX_MALE : SV_BIRTH_SEX_FEMALE;
        birth->random_used = true;
        return confirm(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_available || !legal_sex(birth->dna_sex)) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_sex = birth->dna_sex;
        birth->dna_used = true;
        return confirm(birth);
    case SV_BIRTH_CONFIRM:
        return confirm(birth);
    case SV_BIRTH_QUIT:
        birth->quit = true;
        return SV_BIRTH_INPUT_QUIT;
    }
    return SV_BIRTH_INPUT_IGNORED;
}

void sv_birth_view_prepare(SvBirthView *view, const SvBirth *birth)
{
    if (!view) return;
    *view = birth ? (SvBirthView){.owner = birth->owner,
        .highlighted_sex = birth->highlighted_sex,
        .dna_available = birth->dna_available} : (SvBirthView){0};
}
