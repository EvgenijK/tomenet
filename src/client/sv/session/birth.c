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

static bool legal_race(const SvBirth *birth, int race_id)
{
    if (race_id < 0 || race_id >= birth->setup->race_count ||
        birth->chosen_class < 0 || birth->chosen_class >= 32)
        return false;
    return (birth->setup->races[race_id].choice &
            (1u << (unsigned)birth->chosen_class)) != 0;
}

static bool legal_trait(const SvBirth *birth, int trait_id)
{
    if (trait_id <= 0 || trait_id >= birth->setup->trait_count ||
        birth->chosen_race < 0 || birth->chosen_race >= 32 ||
        !legal_class(birth, birth->chosen_class) ||
        !legal_race(birth, birth->chosen_race))
        return false;
    return (birth->setup->traits[trait_id].choice &
            (1u << (unsigned)birth->chosen_race)) != 0;
}

static bool legal_body(const SvBirth *birth, SvBirthBody body)
{
    if (body != SV_BIRTH_BODY_NORMAL && body != SV_BIRTH_BODY_FRUIT_BAT)
        return false;
    /* The baseline protocol defines only the traditional (0) and point-buy (1)
       birth setups.  A different authoritative setup has no known body step. */
    if (birth->setup->creation_flags > 1) return false;
    return legal_class(birth, birth->chosen_class) &&
        legal_race(birth, birth->chosen_race) &&
        (birth->trait_skipped || legal_trait(birth, birth->chosen_trait));
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

static SvBirthInputResult confirm_race(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_RACE ||
        !legal_race(birth, birth->highlighted_race))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_race = birth->highlighted_race;
    birth->owner = SV_BIRTH_TRAIT;
    birth->race_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult confirm_trait(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_TRAIT ||
        !legal_trait(birth, birth->highlighted_trait))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_trait = birth->highlighted_trait;
    birth->owner = SV_BIRTH_BODY;
    birth->trait_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult confirm_body(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_BODY ||
        !legal_body(birth, birth->highlighted_body))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_body = birth->highlighted_body;
    birth->owner = SV_BIRTH_STATS;
    birth->body_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, bool dna_available,
                        SvBirthSex dna_sex, bool dna_class_available,
                        int dna_class, bool dna_race_available, int dna_race,
                        bool dna_trait_available, int dna_trait,
                        bool dna_body_available, SvBirthBody dna_body)
{
    if (!birth || !generation || !setup || (dna_available && !legal_sex(dna_sex)))
        return SV_INVALID;
    *birth = (SvBirth){.generation = generation, .owner = SV_BIRTH_SEX,
        .highlighted_sex = SV_BIRTH_SEX_MALE, .chosen_sex = SV_BIRTH_SEX_NONE,
        .highlighted_class = SV_BIRTH_CLASS_NONE,
        .chosen_class = SV_BIRTH_CLASS_NONE,
        .highlighted_race = SV_BIRTH_RACE_NONE,
        .chosen_race = SV_BIRTH_RACE_NONE,
        .highlighted_trait = SV_BIRTH_TRAIT_NONE,
        .chosen_trait = SV_BIRTH_TRAIT_NONE,
        .highlighted_body = SV_BIRTH_BODY_NONE,
        .chosen_body = SV_BIRTH_BODY_NONE,
        .dna_sex = dna_sex, .dna_available = dna_available, .entered = true,
        .dna_class = dna_class, .dna_class_available = dna_class_available,
        .dna_race = dna_race, .dna_race_available = dna_race_available,
        .dna_trait = dna_trait, .dna_trait_available = dna_trait_available,
        .dna_body = dna_body, .dna_body_available = dna_body_available,
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

static int race_position(const SvBirth *birth, int race_id)
{
    for (unsigned i = 0; i < birth->legal_race_count; ++i)
        if (birth->legal_races[i] == race_id) return (int)i;
    return -1;
}

static void prepare_races(SvBirth *birth)
{
    birth->legal_race_count = 0;
    for (unsigned id = 0; id < birth->setup->race_count; ++id) {
        if (!legal_race(birth, (int)id)) continue;
        birth->legal_races[birth->legal_race_count++] = (uint8_t)id;
    }
    if (race_position(birth, birth->highlighted_race) < 0)
        birth->highlighted_race = birth->legal_race_count ?
            birth->legal_races[0] : SV_BIRTH_RACE_NONE;
}

static SvBirthInputResult race_command(SvBirth *birth, SvBirthCommand command)
{
    int position = race_position(birth, birth->highlighted_race);
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + birth->legal_race_count - 1) % birth->legal_race_count;
        birth->highlighted_race = birth->legal_races[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_MOVE_NEXT:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + 1) % birth->legal_race_count;
        birth->highlighted_race = birth->legal_races[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_RACE:
        if (!legal_race(birth, command.race_id)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_race = command.race_id;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_RANDOM:
        if (!birth->legal_race_count) return SV_BIRTH_INPUT_IGNORED;
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_race = birth->legal_races[
            birth->random_state % birth->legal_race_count];
        birth->race_random_used = true;
        return confirm_race(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_race_available || !legal_race(birth, birth->dna_race)) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_race = birth->dna_race;
        birth->race_dna_used = true;
        return confirm_race(birth);
    case SV_BIRTH_CONFIRM:
        return confirm_race(birth);
    case SV_BIRTH_BACK:
        birth->chosen_race = SV_BIRTH_RACE_NONE;
        birth->race_confirmed = false;
        birth->owner = SV_BIRTH_CLASS;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_QUIT:
        birth->quit = true;
        return SV_BIRTH_INPUT_QUIT;
    default:
        return SV_BIRTH_INPUT_IGNORED;
    }
}

static int trait_position(const SvBirth *birth, int trait_id)
{
    for (unsigned i = 0; i < birth->legal_trait_count; ++i)
        if (birth->legal_traits[i] == trait_id) return (int)i;
    return -1;
}

static void prepare_bodies(SvBirth *birth);

static void prepare_traits(SvBirth *birth)
{
    birth->legal_trait_count = 0;
    for (unsigned id = 1; id < birth->setup->trait_count; ++id) {
        if (!legal_trait(birth, (int)id)) continue;
        birth->legal_traits[birth->legal_trait_count++] = (uint8_t)id;
    }
    if (trait_position(birth, birth->highlighted_trait) < 0)
        birth->highlighted_trait = birth->legal_trait_count ?
            birth->legal_traits[0] : SV_BIRTH_TRAIT_NONE;
    bool dummy = birth->setup->trait_count <= 1;
    if (!dummy && birth->chosen_race >= 0 && birth->chosen_race < 32)
        dummy = (birth->setup->traits[0].choice &
                 (1u << (unsigned)birth->chosen_race)) != 0;
    birth->trait_skipped = dummy || !birth->legal_trait_count;
    if (birth->trait_skipped) {
        birth->legal_trait_count = 0;
        birth->highlighted_trait = SV_BIRTH_TRAIT_NONE;
        birth->chosen_trait = SV_BIRTH_TRAIT_NONE;
        birth->trait_confirmed = false;
        birth->owner = SV_BIRTH_BODY;
    }
}

static SvBirthInputResult trait_command(SvBirth *birth, SvBirthCommand command)
{
    int position = trait_position(birth, birth->highlighted_trait);
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + birth->legal_trait_count - 1) %
            birth->legal_trait_count;
        birth->highlighted_trait = birth->legal_traits[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_MOVE_NEXT:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + 1) % birth->legal_trait_count;
        birth->highlighted_trait = birth->legal_traits[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_TRAIT:
        if (!legal_trait(birth, command.trait_id)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_trait = command.trait_id;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_RANDOM:
        if (!birth->legal_trait_count) return SV_BIRTH_INPUT_IGNORED;
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_trait = birth->legal_traits[
            birth->random_state % birth->legal_trait_count];
        birth->trait_random_used = true;
        return confirm_trait(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_trait_available ||
            !legal_trait(birth, birth->dna_trait)) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_trait = birth->dna_trait;
        birth->trait_dna_used = true;
        return confirm_trait(birth);
    case SV_BIRTH_CONFIRM:
        return confirm_trait(birth);
    case SV_BIRTH_BACK:
        birth->chosen_trait = SV_BIRTH_TRAIT_NONE;
        birth->trait_confirmed = false;
        birth->owner = SV_BIRTH_RACE;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_QUIT:
        birth->quit = true;
        return SV_BIRTH_INPUT_QUIT;
    default:
        return SV_BIRTH_INPUT_IGNORED;
    }
}

static int body_position(const SvBirth *birth, SvBirthBody body)
{
    for (unsigned i = 0; i < birth->legal_body_count; ++i)
        if (birth->legal_bodies[i] == body) return (int)i;
    return -1;
}

static void prepare_bodies(SvBirth *birth)
{
    birth->legal_body_count = 0;
    /* Do not give ownership to a choice surface for an unknown server setup. */
    birth->body_skipped = birth->setup->creation_flags > 1;
    if (birth->body_skipped) {
        birth->highlighted_body = SV_BIRTH_BODY_NONE;
        birth->chosen_body = SV_BIRTH_BODY_NONE;
        birth->body_confirmed = false;
        birth->owner = SV_BIRTH_STATS;
        return;
    }
    birth->legal_bodies[birth->legal_body_count++] = SV_BIRTH_BODY_NORMAL;
    birth->legal_bodies[birth->legal_body_count++] = SV_BIRTH_BODY_FRUIT_BAT;
    if (body_position(birth, birth->highlighted_body) < 0)
        birth->highlighted_body = birth->legal_bodies[0];
}

static SvBirthInputResult body_command(SvBirth *birth, SvBirthCommand command)
{
    int position = body_position(birth, birth->highlighted_body);
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + birth->legal_body_count - 1) %
            birth->legal_body_count;
        birth->highlighted_body = birth->legal_bodies[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_MOVE_NEXT:
        if (position < 0) return SV_BIRTH_INPUT_IGNORED;
        position = (position + 1) % birth->legal_body_count;
        birth->highlighted_body = birth->legal_bodies[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_BODY:
        if (!legal_body(birth, command.body)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_body = command.body;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_RANDOM:
        if (!birth->legal_body_count) return SV_BIRTH_INPUT_IGNORED;
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_body = birth->legal_bodies[
            birth->random_state % birth->legal_body_count];
        birth->body_random_used = true;
        return confirm_body(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_body_available ||
            !legal_body(birth, birth->dna_body)) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_body = birth->dna_body;
        birth->body_dna_used = true;
        return confirm_body(birth);
    case SV_BIRTH_CONFIRM:
        return confirm_body(birth);
    case SV_BIRTH_BACK:
        birth->chosen_body = SV_BIRTH_BODY_NONE;
        birth->body_confirmed = false;
        birth->chosen_trait = SV_BIRTH_TRAIT_NONE;
        birth->trait_confirmed = false;
        birth->owner = birth->trait_skipped ? SV_BIRTH_RACE : SV_BIRTH_TRAIT;
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
    if (birth->owner == SV_BIRTH_CLASS) {
        SvBirthInputResult result = class_command(birth, command);
        if (result == SV_BIRTH_INPUT_CONFIRMED) prepare_races(birth);
        return result;
    }
    if (birth->owner == SV_BIRTH_RACE) {
        SvBirthInputResult result = race_command(birth, command);
        if (result == SV_BIRTH_INPUT_CONFIRMED) {
            prepare_traits(birth);
            if (birth->owner == SV_BIRTH_BODY) prepare_bodies(birth);
        }
        return result;
    }
    if (birth->owner == SV_BIRTH_TRAIT) {
        SvBirthInputResult result = trait_command(birth, command);
        if (result == SV_BIRTH_INPUT_CONFIRMED) prepare_bodies(birth);
        return result;
    }
    if (birth->owner == SV_BIRTH_BODY)
        return body_command(birth, command);
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
    case SV_BIRTH_SELECT_RACE:
    case SV_BIRTH_SELECT_TRAIT:
    case SV_BIRTH_SELECT_BODY:
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
        .highlighted_race = birth->highlighted_race,
        .legal_race_count = birth->legal_race_count,
        .highlighted_trait = birth->highlighted_trait,
        .legal_trait_count = birth->legal_trait_count,
        .highlighted_body = birth->highlighted_body,
        .legal_body_count = birth->legal_body_count,
        .setup = birth->setup,
        .dna_available = birth->dna_available} : (SvBirthView){0};
    if (birth) {
        for (unsigned i = 0; i < birth->legal_class_count; ++i)
            view->legal_classes[i] = birth->legal_classes[i];
        for (unsigned i = 0; i < birth->legal_race_count; ++i)
            view->legal_races[i] = birth->legal_races[i];
        for (unsigned i = 0; i < birth->legal_trait_count; ++i)
            view->legal_traits[i] = birth->legal_traits[i];
        for (unsigned i = 0; i < birth->legal_body_count; ++i)
            view->legal_bodies[i] = birth->legal_bodies[i];
    }
}
