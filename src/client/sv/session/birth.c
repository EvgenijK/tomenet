#include "session/birth.h"

static bool legal_sex(SvBirthSex sex)
{
    return sex == SV_BIRTH_SEX_MALE || sex == SV_BIRTH_SEX_FEMALE;
}

static bool compatible_class(const SvContactSetup *setup, unsigned class_id)
{
    if (class_id >= 32) return false;
    uint32_t class_bit = 1u << class_id;
    for (unsigned race = 0; race < setup->race_count; ++race)
        if (setup->races[race].choice & class_bit) return true;
    return false;
}

static bool legal_class(const SvBirth *birth, int class_id)
{
    if (class_id < 0 || class_id >= birth->setup->class_count) return false;
    if (birth->setup->classes[class_id].hidden) return false;
    return compatible_class(birth->setup, (unsigned)class_id);
}

static SvBirthInputResult confirm_sex(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_SEX || !legal_sex(birth->highlighted_sex))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_sex = birth->highlighted_sex;
    birth->owner = SV_BIRTH_CLASS;
    birth->confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult confirm_class(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_CLASS ||
        !legal_class(birth, birth->highlighted_class))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_class = birth->highlighted_class;
    birth->owner = SV_BIRTH_RACE;
    birth->class_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, bool dna_available,
                        SvBirthSex dna_sex, bool dna_class_available,
                        int dna_class)
{
    if (!birth || !generation || !setup || (dna_available && !legal_sex(dna_sex)))
        return SV_INVALID;
    *birth = (SvBirth){.generation = generation, .owner = SV_BIRTH_SEX,
        .highlighted_sex = SV_BIRTH_SEX_MALE, .chosen_sex = SV_BIRTH_SEX_NONE,
        .highlighted_class = SV_BIRTH_CLASS_NONE,
        .chosen_class = SV_BIRTH_CLASS_NONE,
        .dna_sex = dna_sex, .dna_available = dna_available, .entered = true,
        .dna_class = dna_class, .dna_class_available = dna_class_available,
        .random_state = (uint32_t)(generation ^ 0x9e3779b9u), .setup = setup};
    for (unsigned id = 0; id < setup->class_count; ++id) {
        if (setup->classes[id].hidden || !compatible_class(setup, id)) continue;
        birth->legal_classes[birth->legal_class_count++] = (uint8_t)id;
    }
    if (birth->legal_class_count)
        birth->highlighted_class = birth->legal_classes[0];
    return SV_OK;
}

static int class_position(const SvBirth *birth, int class_id)
{
    for (unsigned i = 0; i < birth->legal_class_count; ++i)
        if (birth->legal_classes[i] == class_id) return (int)i;
    return -1;
}

static SvBirthInputResult class_command(SvBirth *birth, SvBirthCommand command)
{
    int position = class_position(birth, birth->highlighted_class);
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + birth->legal_class_count - 1) % birth->legal_class_count;
        birth->highlighted_class = birth->legal_classes[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_MOVE_NEXT:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + 1) % birth->legal_class_count;
        birth->highlighted_class = birth->legal_classes[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_CLASS:
        if (!legal_class(birth, command.class_id)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_class = command.class_id;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_RANDOM:
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_class = birth->legal_classes[
            birth->random_state % birth->legal_class_count];
        birth->class_random_used = true;
        return confirm_class(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_class_available || !legal_class(birth, birth->dna_class)) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_class = birth->dna_class;
        birth->class_dna_used = true;
        return confirm_class(birth);
    case SV_BIRTH_CONFIRM:
        return confirm_class(birth);
    case SV_BIRTH_BACK:
        birth->chosen_class = SV_BIRTH_CLASS_NONE;
        birth->class_confirmed = false;
        birth->owner = SV_BIRTH_SEX;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_QUIT:
        birth->quit = true;
        return SV_BIRTH_INPUT_QUIT;
    default:
        return SV_BIRTH_INPUT_IGNORED;
    }
}

SvBirthInputResult sv_birth_command(SvBirth *birth, uint64_t generation,
                                    SvBirthCommand command)
{
    if (!birth || generation != birth->generation || birth->quit)
        return SV_BIRTH_INPUT_IGNORED;
    if (birth->owner == SV_BIRTH_CLASS)
        return class_command(birth, command);
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
        return confirm_sex(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_available || !legal_sex(birth->dna_sex)) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_sex = birth->dna_sex;
        birth->dna_used = true;
        return confirm_sex(birth);
    case SV_BIRTH_CONFIRM:
        return confirm_sex(birth);
    case SV_BIRTH_QUIT:
        birth->quit = true;
        return SV_BIRTH_INPUT_QUIT;
    case SV_BIRTH_SELECT_CLASS:
    case SV_BIRTH_BACK:
        return SV_BIRTH_INPUT_IGNORED;
    }
    return SV_BIRTH_INPUT_IGNORED;
}

void sv_birth_view_prepare(SvBirthView *view, const SvBirth *birth)
{
    if (!view) return;
    *view = birth ? (SvBirthView){.owner = birth->owner,
        .highlighted_sex = birth->highlighted_sex,
        .highlighted_class = birth->highlighted_class,
        .legal_class_count = birth->legal_class_count,
        .setup = birth->setup,
        .dna_available = birth->dna_available} : (SvBirthView){0};
    if (birth)
        for (unsigned i = 0; i < birth->legal_class_count; ++i)
            view->legal_classes[i] = birth->legal_classes[i];
}
