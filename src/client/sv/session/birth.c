#include "session/birth.h"
#include <string.h>

static bool legal_sex(SvBirthSex sex)
{
    return sex == SV_BIRTH_SEX_MALE || sex == SV_BIRTH_SEX_FEMALE;
}

#if SV_CLASS_BEFORE_RACE
static bool compatible_class(const SvContactSetup *setup, unsigned class_id)
{
    if (class_id >= 32) return false;
    uint32_t class_bit = 1u << class_id;
    for (unsigned race = 0; race < setup->race_count; ++race)
        if (setup->races[race].choice & class_bit) return true;
    return false;
}
#endif

#if !SV_CLASS_BEFORE_RACE
static bool compatible_race(const SvContactSetup *setup, unsigned race_id)
{
    if (race_id >= setup->race_count) return false;
    for (unsigned class_id = 0; class_id < setup->class_count; ++class_id) {
        if (setup->classes[class_id].hidden || class_id >= 32) continue;
        if (setup->races[race_id].choice & (1u << class_id)) return true;
    }
    return false;
}
#endif

static bool legal_class(const SvBirth *birth, int class_id)
{
    if (class_id < 0 || class_id >= birth->setup->class_count) return false;
    if (birth->setup->classes[class_id].hidden) return false;
#if SV_CLASS_BEFORE_RACE
    return compatible_class(birth->setup, (unsigned)class_id);
#else
    if (class_id >= 32 || birth->chosen_race < 0 ||
        birth->chosen_race >= birth->setup->race_count)
        return false;
    return (birth->setup->races[birth->chosen_race].choice &
            (1u << (unsigned)class_id)) != 0;
#endif
}

static int resolved_class(const SvBirth *birth, int visible_class, int race_id);

static unsigned maia_race_id(void)
{
#ifdef ENABLE_KOBOLD
    return 17;
#else
    return 16;
#endif
}

static bool trait_derived_hidden_class(const SvBirth *birth, int class_id)
{
    return class_id >= 0 && class_id < birth->setup->class_count &&
        birth->setup->classes[class_id].hidden &&
        (!strcmp(birth->setup->classes[class_id].title, "Hell Knight") ||
         !strcmp(birth->setup->classes[class_id].title, "Corrupted Priest"));
}

static bool legal_race(const SvBirth *birth, int race_id)
{
    if (race_id < 0 || race_id >= birth->setup->race_count) return false;
#if SV_CLASS_BEFORE_RACE
    if (birth->chosen_visible_class < 0 || birth->chosen_visible_class >= 32)
        return false;
    if (!(birth->setup->races[race_id].choice &
          (1u << (unsigned)birth->chosen_visible_class)))
        return false;
    if (birth->requested_class == SV_BIRTH_CLASS_NONE) return true;
    if (trait_derived_hidden_class(birth, birth->requested_class))
        return race_id == (int)maia_race_id();
    return resolved_class(birth, birth->chosen_visible_class, race_id) ==
        birth->requested_class;
#else
    return compatible_race(birth->setup, (unsigned)race_id);
#endif
}

static int resolved_class(const SvBirth *birth, int visible_class, int race_id)
{
    if (visible_class < 0 || visible_class >= birth->setup->class_count ||
        race_id < 0 || race_id >= birth->setup->race_count ||
        visible_class >= 32 ||
        !(birth->setup->races[race_id].choice &
          (1u << (unsigned)visible_class)))
        return visible_class;
    for (unsigned id = 0; id < birth->setup->class_count && id < 32; ++id) {
        const SvContactClass *candidate = &birth->setup->classes[id];
        if (candidate->hidden && candidate->base_class == visible_class &&
            (birth->setup->races[race_id].choice & (1u << id)))
            return (int)id;
    }
    return visible_class;
}

static int dna_visible_class(const SvBirth *birth, int server_class)
{
    if (legal_class(birth, server_class)) return server_class;
    if (server_class < 0 || server_class >= birth->setup->class_count ||
        server_class >= 32 || !birth->setup->classes[server_class].hidden)
        return SV_BIRTH_CLASS_NONE;
    int visible_class = birth->setup->classes[server_class].base_class;
    if (!legal_class(birth, visible_class)) return SV_BIRTH_CLASS_NONE;
    if (trait_derived_hidden_class(birth, server_class)) {
#if SV_CLASS_BEFORE_RACE
        if (birth->chosen_race >= 0 &&
            birth->chosen_race != (int)maia_race_id())
#else
        if (birth->chosen_race != (int)maia_race_id() ||
            birth->chosen_trait != 15)
#endif
            return SV_BIRTH_CLASS_NONE;
        return visible_class;
    }
    if (birth->chosen_race >= 0)
        return resolved_class(birth, visible_class, birth->chosen_race) ==
            server_class ? visible_class : SV_BIRTH_CLASS_NONE;
    for (unsigned race = 0; race < birth->setup->race_count; ++race)
        if (resolved_class(birth, visible_class, (int)race) == server_class)
            return visible_class;
    return SV_BIRTH_CLASS_NONE;
}

static bool legal_trait(const SvBirth *birth, int trait_id)
{
    if (birth->post_mode_trait_required)
        return birth->chosen_race == (int)maia_race_id() &&
            (trait_id == 14 || trait_id == 15) &&
            trait_id < birth->setup->trait_count;
    if (trait_id <= 0 || trait_id >= birth->setup->trait_count ||
        birth->chosen_race < 0 || birth->chosen_race >= 32 ||
        !legal_race(birth, birth->chosen_race))
        return false;
#if SV_CLASS_BEFORE_RACE
    if (!legal_class(birth, birth->chosen_visible_class)) return false;
    if (trait_derived_hidden_class(birth, birth->requested_class) &&
        trait_id != 15)
        return false;
#endif
    return (birth->setup->traits[trait_id].choice &
            (1u << (unsigned)birth->chosen_race)) != 0;
}

static bool legal_body(const SvBirth *birth, SvBirthBody body)
{
    if (body != SV_BIRTH_BODY_NORMAL && body != SV_BIRTH_BODY_FRUIT_BAT)
        return false;
    if (body == SV_BIRTH_BODY_FRUIT_BAT && birth->first_run) return false;
    /* The baseline protocol defines only the traditional (0) and point-buy (1)
       birth setups.  A different authoritative setup has no known body step. */
    if (birth->setup->creation_flags > 1) return false;
    if (!legal_race(birth, birth->chosen_race)) return false;
#if SV_CLASS_BEFORE_RACE
    if (!legal_class(birth, birth->chosen_visible_class)) return false;
#endif
    return
        (birth->trait_skipped || legal_trait(birth, birth->chosen_trait));
}

static void prepare_classes(SvBirth *birth);
static void prepare_races(SvBirth *birth);
static void prepare_stats(SvBirth *birth);
static void restart_stats_interaction(SvBirth *birth);
static void prepare_modes(SvBirth *birth);
static void prepare_traits(SvBirth *birth);

static void restore_pre_mode_trait(SvBirth *birth)
{
    if (!birth->post_mode_trait_required) return;
    birth->post_mode_trait_required = false;
    if (birth->pre_mode_trait_saved) {
        birth->highlighted_trait = birth->pre_mode_highlighted_trait;
        birth->chosen_trait = birth->pre_mode_chosen_trait;
        birth->trait_confirmed = birth->pre_mode_trait_confirmed;
        birth->pre_mode_trait_saved = false;
    }
    prepare_traits(birth);
}

static void leave_post_mode_trait(SvBirth *birth)
{
    restore_pre_mode_trait(birth);
    birth->chosen_mode = SV_BIRTH_MODE_NONE;
    birth->mode_confirmed = false;
    birth->mode_ready = false;
}

static void enter_stats_or_mode(SvBirth *birth)
{
    birth->owner = SV_BIRTH_STATS;
    prepare_stats(birth);
    if (!birth->legal_stat_count) {
        birth->owner = SV_BIRTH_MODE;
        prepare_modes(birth);
    }
}

static SvBirthInputResult confirm_sex(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_SEX || !legal_sex(birth->highlighted_sex))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_sex = birth->highlighted_sex;
#if SV_CLASS_BEFORE_RACE
    birth->owner = SV_BIRTH_CLASS;
#else
    birth->owner = SV_BIRTH_RACE;
    prepare_races(birth);
#endif
    birth->confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult confirm_class(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_CLASS ||
        !legal_class(birth, birth->highlighted_class))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_visible_class = birth->highlighted_class;
    birth->chosen_class = birth->highlighted_class;
#if SV_CLASS_BEFORE_RACE
    birth->owner = SV_BIRTH_RACE;
#else
    birth->chosen_class = resolved_class(birth, birth->chosen_visible_class,
                                         birth->chosen_race);
    enter_stats_or_mode(birth);
#endif
    birth->last_confirmed_class = birth->chosen_class;
    birth->class_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult confirm_race(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_RACE ||
        !legal_race(birth, birth->highlighted_race))
        return SV_BIRTH_INPUT_IGNORED;
#if SV_CLASS_BEFORE_RACE
    int resolved = resolved_class(birth, birth->chosen_visible_class,
                                  birth->highlighted_race);
    if (birth->requested_class != SV_BIRTH_CLASS_NONE &&
        !trait_derived_hidden_class(birth, birth->requested_class) &&
        resolved != birth->requested_class) {
        birth->illegal_ignored = true;
        birth->auto_reincarnation = false;
        return SV_BIRTH_INPUT_IGNORED;
    }
#endif
    birth->chosen_race = birth->highlighted_race;
#if SV_CLASS_BEFORE_RACE
    bool trait_derived =
        trait_derived_hidden_class(birth, birth->requested_class);
    birth->chosen_class = trait_derived ? birth->chosen_visible_class : resolved;
    if (!trait_derived) {
        birth->requested_class = SV_BIRTH_CLASS_NONE;
        prepare_races(birth);
    }
#endif
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
#if SV_CLASS_BEFORE_RACE
    if (trait_derived_hidden_class(birth, birth->requested_class)) {
        birth->requested_class = SV_BIRTH_CLASS_NONE;
        prepare_races(birth);
    }
#endif
    if (birth->post_mode_trait_required) {
        birth->owner = SV_BIRTH_READY;
        birth->mode_ready = true;
    } else birth->owner = SV_BIRTH_BODY;
    birth->trait_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult confirm_body(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_BODY ||
        !legal_body(birth, birth->highlighted_body))
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_body = birth->highlighted_body;
#if SV_CLASS_BEFORE_RACE
    birth->owner = SV_BIRTH_STATS;
    prepare_stats(birth);
#else
    birth->owner = SV_BIRTH_CLASS;
#endif
    birth->body_confirmed = true;
    return SV_BIRTH_INPUT_CONFIRMED;
}

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, const uint32_t server_flags[4],
                        bool first_run, uint32_t dedicated_mode_flags,
                        bool dna_available,
                        SvBirthSex dna_sex, bool dna_class_available,
                        int dna_class, bool dna_race_available, int dna_race,
                        bool dna_trait_available, int dna_trait,
                        bool dna_body_available, SvBirthBody dna_body,
                        bool dna_stats_available, const int16_t dna_stats[6],
                        bool dna_mode_available, SvBirthMode dna_mode)
{
    if (!birth || !generation || !setup || !server_flags || !dna_stats ||
        (dna_available && !legal_sex(dna_sex)))
        return SV_INVALID;
    *birth = (SvBirth){.generation = generation, .owner = SV_BIRTH_SEX,
        .highlighted_sex = SV_BIRTH_SEX_MALE, .chosen_sex = SV_BIRTH_SEX_NONE,
        .highlighted_class = SV_BIRTH_CLASS_NONE,
        .chosen_visible_class = SV_BIRTH_CLASS_NONE,
        .chosen_class = SV_BIRTH_CLASS_NONE,
        .requested_class = SV_BIRTH_CLASS_NONE,
        .last_confirmed_class = SV_BIRTH_CLASS_NONE,
        .highlighted_race = SV_BIRTH_RACE_NONE,
        .chosen_race = SV_BIRTH_RACE_NONE,
        .highlighted_trait = SV_BIRTH_TRAIT_NONE,
        .chosen_trait = SV_BIRTH_TRAIT_NONE,
        .highlighted_body = SV_BIRTH_BODY_NONE,
        .chosen_body = SV_BIRTH_BODY_NONE,
        .highlighted_mode = SV_BIRTH_MODE_NONE,
        .chosen_mode = SV_BIRTH_MODE_NONE,
        .dna_sex = dna_sex, .dna_available = dna_available,
        .entered = true, .active = true,
        .dna_class = dna_class, .dna_class_available = dna_class_available,
        .dna_race = dna_race, .dna_race_available = dna_race_available,
        .dna_trait = dna_trait, .dna_trait_available = dna_trait_available,
        .dna_body = dna_body, .dna_body_available = dna_body_available,
        .dna_stats_available = dna_stats_available,
        .dna_mode_available = dna_mode_available, .dna_mode = dna_mode,
        .random_state = (uint32_t)(generation ^ 0x9e3779b9u), .setup = setup,
        .race_count = setup->race_count, .trait_count = setup->trait_count,
        .dedicated = dedicated_mode_flags != 0, .first_run = first_run,
        .dedicated_mode_flags = dedicated_mode_flags};
    for (unsigned i = 0; i < 4; ++i) birth->server_flags[i] = server_flags[i];
    for (unsigned i = 0; i < 6; ++i) birth->dna_stats[i] = dna_stats[i];
#if SV_CLASS_BEFORE_RACE
    prepare_classes(birth);
#else
    prepare_races(birth);
#endif
    return SV_OK;
}

static void prepare_classes(SvBirth *birth)
{
    birth->legal_class_count = 0;
    bool highlighted_legal = false;
    for (unsigned id = 0; id < birth->setup->class_count; ++id) {
        if (!legal_class(birth, (int)id)) continue;
        birth->legal_classes[birth->legal_class_count++] = (uint8_t)id;
        highlighted_legal |= birth->highlighted_class == (int)id;
    }
    if (!highlighted_legal)
        birth->highlighted_class = birth->legal_class_count ?
            birth->legal_classes[0] : SV_BIRTH_CLASS_NONE;
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
        if (!birth->legal_class_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_class = birth->legal_classes[
            birth->random_state % birth->legal_class_count];
        birth->class_random_used = true;
        birth->requested_class = SV_BIRTH_CLASS_NONE;
        return confirm_class(birth);
    case SV_BIRTH_DNA: {
        int visible_class = birth->dna_class_available ?
            dna_visible_class(birth, birth->dna_class) : SV_BIRTH_CLASS_NONE;
        if (visible_class == SV_BIRTH_CLASS_NONE) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_class = visible_class;
        birth->class_dna_used = true;
#if SV_CLASS_BEFORE_RACE
        birth->requested_class = birth->dna_class == visible_class ?
            SV_BIRTH_CLASS_NONE : birth->dna_class;
#endif
        return confirm_class(birth);
    }
    case SV_BIRTH_CONFIRM:
        birth->requested_class = SV_BIRTH_CLASS_NONE;
        return confirm_class(birth);
    case SV_BIRTH_BACK:
        birth->chosen_visible_class = SV_BIRTH_CLASS_NONE;
        birth->chosen_class = SV_BIRTH_CLASS_NONE;
        birth->requested_class = SV_BIRTH_CLASS_NONE;
        birth->class_confirmed = false;
#if SV_CLASS_BEFORE_RACE
        birth->owner = SV_BIRTH_SEX;
#else
        birth->owner = birth->body_skipped ?
            (birth->trait_skipped ? SV_BIRTH_RACE : SV_BIRTH_TRAIT) :
            SV_BIRTH_BODY;
#endif
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
        if (!birth->legal_race_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
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
#if SV_CLASS_BEFORE_RACE
        if (birth->requested_class != SV_BIRTH_CLASS_NONE &&
            !trait_derived_hidden_class(birth, birth->requested_class) &&
            resolved_class(birth, birth->chosen_visible_class,
                           birth->dna_race) != birth->requested_class) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
#endif
        birth->highlighted_race = birth->dna_race;
        SvBirthInputResult result = confirm_race(birth);
        if (result == SV_BIRTH_INPUT_CONFIRMED) birth->race_dna_used = true;
        return result;
    case SV_BIRTH_CONFIRM:
        return confirm_race(birth);
    case SV_BIRTH_BACK:
        birth->chosen_race = SV_BIRTH_RACE_NONE;
        birth->race_confirmed = false;
#if SV_CLASS_BEFORE_RACE
        birth->chosen_class = birth->chosen_visible_class;
        birth->owner = SV_BIRTH_CLASS;
#else
        birth->owner = SV_BIRTH_SEX;
#endif
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
    bool dummy = !birth->post_mode_trait_required &&
        birth->setup->trait_count <= 1;
    if (!birth->post_mode_trait_required && !dummy &&
        birth->chosen_race >= 0 && birth->chosen_race < 32)
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
        if (birth->post_mode_trait_required) {
            if (birth->setup->creation_flags <= 1) {
                restart_stats_interaction(birth);
                birth->owner = SV_BIRTH_STATS;
            } else {
                prepare_modes(birth);
                birth->owner = SV_BIRTH_MODE;
            }
        } else birth->owner = SV_BIRTH_RACE;
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
#if SV_CLASS_BEFORE_RACE
        enter_stats_or_mode(birth);
#else
        birth->owner = SV_BIRTH_CLASS;
        prepare_classes(birth);
#endif
        return;
    }
    birth->legal_bodies[birth->legal_body_count++] = SV_BIRTH_BODY_NORMAL;
    if (!birth->first_run)
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
        if (birth->chosen_class == SV_BIRTH_CLASS_ARCHER ||
            birth->last_confirmed_class == SV_BIRTH_CLASS_ARCHER) {
            birth->highlighted_body = SV_BIRTH_BODY_NORMAL;
        } else {
            birth->random_state ^= birth->random_state << 13;
            birth->random_state ^= birth->random_state >> 17;
            birth->random_state ^= birth->random_state << 5;
            birth->highlighted_body = birth->legal_bodies[
                birth->random_state % birth->legal_body_count];
        }
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
#if !SV_CLASS_BEFORE_RACE
        birth->last_confirmed_class = SV_BIRTH_CLASS_NONE;
#endif
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

static int stat_position(const SvBirth *birth, int stat)
{
    for (unsigned i = 0; i < birth->legal_stat_count; ++i)
        if (birth->legal_stats[i] == stat) return (int)i;
    return -1;
}

static void restart_stats_interaction(SvBirth *birth)
{
    birth->legal_stat_count = birth->setup->creation_flags <= 1 ? 6 : 0;
    birth->stats_order_count = 0;
    birth->stats_confirmation_pending = false;
    if (birth->highlighted_stat < 0 || birth->highlighted_stat >= 6)
        birth->highlighted_stat = birth->legal_stat_count ? 0 : -1;
    for (unsigned i = 0; i < 6; ++i) {
        birth->stats[i] = birth->stats_point_buy ? 10 : -1;
        birth->legal_stats[i] = (uint8_t)i;
    }
    birth->stats_remaining = birth->stats_point_buy ? 30 : 0;
}

static void prepare_stats(SvBirth *birth)
{
    if (birth->stats_prepared) return;
    birth->stats_prepared = true;
    birth->highlighted_stat = birth->setup->creation_flags <= 1 ? 0 : -1;
    birth->stats_point_buy = birth->setup->creation_flags == 1;
    for (unsigned i = 0; i < 6; ++i) birth->committed_stats[i] = -1;
    restart_stats_interaction(birth);
}

static int stat_cost(int value)
{
    if (value <= 12) return value - 10;
    if (value <= 14) return 2 + (value - 12) * 2;
    if (value <= 16) return 6 + (value - 14) * 3;
    return 16;
}

static int stat_bonus(const SvBirth *birth, int stat)
{
    if (stat < 0 || stat >= 6 || birth->chosen_class < 0 ||
        birth->chosen_class >= birth->setup->class_count ||
        birth->chosen_race < 0 || birth->chosen_race >= birth->setup->race_count)
        return 0;
    return birth->setup->classes[birth->chosen_class].adjustment[stat] +
        birth->setup->races[birth->chosen_race].adjustment[stat];
}

static int point_buy_remaining(const int16_t stats[6])
{
    int remaining = 30;
    for (unsigned i = 0; i < 6; ++i) remaining -= stat_cost(stats[i]);
    return remaining;
}

static bool legal_point_buy(const SvBirth *birth, const int16_t stats[6])
{
    unsigned maxed = 0;
    for (unsigned i = 0; i < 6; ++i) {
        if (stats[i] < 8 || stats[i] > 17) return false;
        if (stats[i] == 17 && ++maxed > 1) return false;
        if (stats[i] < 10 && stats[i] + stat_bonus(birth, (int)i) < 3)
            return false;
    }
    return point_buy_remaining(stats) >= 0;
}

static SvBirthInputResult confirm_stat_order_choice(SvBirth *birth)
{
    int position = stat_position(birth, birth->highlighted_stat);
    if (position < 0 || birth->stats_order_count >= 6) {
        birth->illegal_ignored = true;
        return SV_BIRTH_INPUT_IGNORED;
    }
    birth->stats[birth->stats_order_count++] = (int16_t)birth->highlighted_stat;
    for (unsigned i = (unsigned)position; i + 1 < birth->legal_stat_count; ++i)
        birth->legal_stats[i] = birth->legal_stats[i + 1];
    --birth->legal_stat_count;
    if (!birth->legal_stat_count) {
        for (unsigned i = 0; i < 6; ++i)
            birth->committed_stats[i] = birth->stats[i];
        birth->stats_confirmed = true;
        birth->owner = SV_BIRTH_MODE;
        return SV_BIRTH_INPUT_CONFIRMED;
    }
    birth->highlighted_stat = birth->legal_stats[0];
    return SV_BIRTH_INPUT_HANDLED;
}

static SvBirthInputResult stats_command(SvBirth *birth, SvBirthCommand command)
{
    int position = stat_position(birth, birth->highlighted_stat);
    if (birth->stats_confirmation_pending &&
        command.kind != SV_BIRTH_CONFIRM && command.kind != SV_BIRTH_QUIT &&
        !(command.kind == SV_BIRTH_SELECT_STAT &&
          command.stat == birth->highlighted_stat))
        birth->stats_confirmation_pending = false;
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
        if (position < 0 || !birth->legal_stat_count)
            return SV_BIRTH_INPUT_IGNORED;
        position = (position + birth->legal_stat_count - 1) %
            birth->legal_stat_count;
        birth->highlighted_stat = birth->legal_stats[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_MOVE_NEXT:
        if (position < 0 || !birth->legal_stat_count)
            return SV_BIRTH_INPUT_IGNORED;
        position = (position + 1) % birth->legal_stat_count;
        birth->highlighted_stat = birth->legal_stats[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_STAT:
        if (stat_position(birth, command.stat) < 0) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_stat = command.stat;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_DECREASE_STAT: {
        if (!birth->stats_point_buy || birth->highlighted_stat < 0 ||
            birth->highlighted_stat >= 6)
            return SV_BIRTH_INPUT_IGNORED;
        int stat = birth->highlighted_stat;
        int value = birth->stats[stat];
        if (value <= 8 || (value <= 10 && value + stat_bonus(birth, stat) <= 3)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->stats[stat] = (int16_t)(value - 1);
        birth->stats_remaining = point_buy_remaining(birth->stats);
        return SV_BIRTH_INPUT_HANDLED;
    }
    case SV_BIRTH_INCREASE_STAT: {
        if (!birth->stats_point_buy || birth->highlighted_stat < 0 ||
            birth->highlighted_stat >= 6)
            return SV_BIRTH_INPUT_IGNORED;
        int stat = birth->highlighted_stat;
        int value = birth->stats[stat];
        if (value >= 17) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        int16_t candidate[6];
        for (unsigned i = 0; i < 6; ++i) candidate[i] = birth->stats[i];
        candidate[stat] = (int16_t)(value + 1);
        if (!legal_point_buy(birth, candidate)) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->stats[stat] = candidate[stat];
        birth->stats_remaining = point_buy_remaining(birth->stats);
        return SV_BIRTH_INPUT_HANDLED;
    }
    case SV_BIRTH_RANDOM:
        if (birth->stats_point_buy) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        while (birth->legal_stat_count) {
            birth->random_state ^= birth->random_state << 13;
            birth->random_state ^= birth->random_state >> 17;
            birth->random_state ^= birth->random_state << 5;
            birth->highlighted_stat = birth->legal_stats[
                birth->random_state % birth->legal_stat_count];
            (void)confirm_stat_order_choice(birth);
        }
        birth->stats_random_used = true;
        return SV_BIRTH_INPUT_CONFIRMED;
    case SV_BIRTH_DNA:
        if (!birth->stats_point_buy && birth->auto_reincarnation)
            return SV_BIRTH_INPUT_HANDLED;
        if (!birth->stats_point_buy || !birth->dna_stats_available) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        for (unsigned i = 0; i < 6; ++i)
            birth->stats[i] = birth->dna_stats[i] >= 8 &&
                birth->dna_stats[i] <= 17 ? birth->dna_stats[i] : 8;
        for (unsigned i = 0; i < 6; ++i)
            birth->committed_stats[i] = birth->stats[i];
        birth->stats_remaining = point_buy_remaining(birth->stats);
        birth->stats_dna_used = true;
        birth->stats_confirmed = true;
        birth->stats_confirmation_pending = false;
        birth->owner = SV_BIRTH_MODE;
        return SV_BIRTH_INPUT_CONFIRMED;
    case SV_BIRTH_CONFIRM:
        if (birth->stats_point_buy) {
            if (!legal_point_buy(birth, birth->stats)) {
                birth->illegal_ignored = true;
                return SV_BIRTH_INPUT_IGNORED;
            }
            if (birth->stats_remaining > 0 &&
                !birth->stats_confirmation_pending) {
                birth->stats_confirmation_pending = true;
                return SV_BIRTH_INPUT_HANDLED;
            }
            for (unsigned i = 0; i < 6; ++i)
                birth->committed_stats[i] = birth->stats[i];
            birth->stats_confirmed = true;
            birth->stats_confirmation_pending = false;
            birth->owner = SV_BIRTH_MODE;
            return SV_BIRTH_INPUT_CONFIRMED;
        }
        return confirm_stat_order_choice(birth);
    case SV_BIRTH_BACK:
        restart_stats_interaction(birth);
        leave_post_mode_trait(birth);
#if SV_CLASS_BEFORE_RACE
        birth->owner = birth->body_skipped ?
            (birth->trait_skipped ? SV_BIRTH_RACE : SV_BIRTH_TRAIT) :
            SV_BIRTH_BODY;
        if (birth->owner == SV_BIRTH_RACE)
            birth->chosen_class = birth->chosen_visible_class;
#else
        birth->owner = SV_BIRTH_CLASS;
#endif
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_QUIT:
        birth->quit = true;
        return SV_BIRTH_INPUT_QUIT;
    default:
        return SV_BIRTH_INPUT_IGNORED;
    }
}

static int mode_position(const SvBirth *birth, SvBirthMode mode)
{
    for (unsigned i = 0; i < birth->legal_mode_count; ++i)
        if (birth->legal_modes[i] == mode) return (int)i;
    return -1;
}

static void add_mode(SvBirth *birth, SvBirthMode mode)
{
    birth->legal_modes[birth->legal_mode_count++] = mode;
}

static void prepare_modes(SvBirth *birth)
{
    bool rpg = (birth->server_flags[3] & 0x01u) != 0;
    birth->legal_mode_count = 0;
    if (birth->dedicated) {
        if (birth->dedicated_mode_flags & 0x40u) {
            add_mode(birth, SV_BIRTH_MODE_IDDC);
            add_mode(birth, SV_BIRTH_MODE_IDDC_SOLOIST);
            add_mode(birth, SV_BIRTH_MODE_IDDC_HELLISH);
        }
        if (!rpg && (birth->dedicated_mode_flags & 0x80u))
            add_mode(birth, SV_BIRTH_MODE_DEDICATED_PVP);
        if (mode_position(birth, birth->highlighted_mode) < 0)
            birth->highlighted_mode = birth->legal_mode_count ?
                birth->legal_modes[0] : SV_BIRTH_MODE_NONE;
        return;
    }
    if (!rpg) add_mode(birth, SV_BIRTH_MODE_NORMAL);
    add_mode(birth, SV_BIRTH_MODE_NO_GHOST);
    add_mode(birth, SV_BIRTH_MODE_SOLOIST);
    if (!rpg) add_mode(birth, SV_BIRTH_MODE_EVERLASTING);
    if (!birth->first_run) add_mode(birth, SV_BIRTH_MODE_HELLISH);
    if (!rpg && !birth->first_run) add_mode(birth, SV_BIRTH_MODE_PVP);
    if (mode_position(birth, birth->highlighted_mode) < 0)
        birth->highlighted_mode = birth->legal_mode_count ?
            birth->legal_modes[0] : SV_BIRTH_MODE_NONE;
}

static SvBirthInputResult confirm_mode(SvBirth *birth)
{
    if (birth->owner != SV_BIRTH_MODE ||
        mode_position(birth, birth->highlighted_mode) < 0)
        return SV_BIRTH_INPUT_IGNORED;
    birth->chosen_mode = birth->highlighted_mode;
    birth->mode_confirmed = true;
    unsigned maia_race =
#ifdef ENABLE_KOBOLD
        17;
#else
        16;
#endif
    bool pvp = birth->chosen_mode == SV_BIRTH_MODE_PVP ||
        birth->chosen_mode == SV_BIRTH_MODE_DEDICATED_PVP;
    bool post_mode_trait_required = pvp &&
        birth->chosen_race == (int)maia_race &&
        (birth->server_flags[3] & 0x200u) != 0;
    if (post_mode_trait_required && !birth->post_mode_trait_required) {
        birth->pre_mode_highlighted_trait = birth->highlighted_trait;
        birth->pre_mode_chosen_trait = birth->chosen_trait;
        birth->pre_mode_trait_confirmed = birth->trait_confirmed;
        birth->pre_mode_trait_saved = true;
    }
    if (!post_mode_trait_required && birth->post_mode_trait_required)
        restore_pre_mode_trait(birth);
    birth->post_mode_trait_required = post_mode_trait_required;
    if (birth->post_mode_trait_required) {
        birth->mode_ready = false;
        birth->owner = SV_BIRTH_TRAIT;
        prepare_traits(birth);
    } else {
        birth->mode_ready = true;
        birth->owner = SV_BIRTH_READY;
    }
    return SV_BIRTH_INPUT_CONFIRMED;
}

static SvBirthInputResult mode_command(SvBirth *birth, SvBirthCommand command)
{
    int position = mode_position(birth, birth->highlighted_mode);
    switch (command.kind) {
    case SV_BIRTH_MOVE_PREVIOUS:
        if (position < 0 || !birth->legal_mode_count)
            return SV_BIRTH_INPUT_IGNORED;
        position = (position + birth->legal_mode_count - 1) %
            birth->legal_mode_count;
        birth->highlighted_mode = birth->legal_modes[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_MOVE_NEXT:
        if (position < 0 || !birth->legal_mode_count)
            return SV_BIRTH_INPUT_IGNORED;
        position = (position + 1) % birth->legal_mode_count;
        birth->highlighted_mode = birth->legal_modes[position];
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_SELECT_MODE:
        if (mode_position(birth, command.mode) < 0) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_mode = command.mode;
        return SV_BIRTH_INPUT_HANDLED;
    case SV_BIRTH_RANDOM:
        if (!birth->legal_mode_count) return SV_BIRTH_INPUT_IGNORED;
        birth->random_state ^= birth->random_state << 13;
        birth->random_state ^= birth->random_state >> 17;
        birth->random_state ^= birth->random_state << 5;
        birth->highlighted_mode = birth->legal_modes[
            birth->random_state % birth->legal_mode_count];
        birth->mode_random_used = true;
        return confirm_mode(birth);
    case SV_BIRTH_DNA:
        if (!birth->dna_mode_available ||
            mode_position(birth, birth->dna_mode) < 0) {
            birth->illegal_ignored = true;
            birth->auto_reincarnation = false;
            return SV_BIRTH_INPUT_IGNORED;
        }
        birth->highlighted_mode = birth->dna_mode;
        birth->mode_dna_used = true;
        return confirm_mode(birth);
    case SV_BIRTH_CONFIRM:
        return confirm_mode(birth);
    case SV_BIRTH_BACK:
        birth->chosen_mode = SV_BIRTH_MODE_NONE;
        birth->mode_confirmed = false;
        birth->mode_ready = false;
        if (birth->setup->creation_flags <= 1) {
            birth->owner = SV_BIRTH_STATS;
            restart_stats_interaction(birth);
        } else {
#if SV_CLASS_BEFORE_RACE
            birth->owner = birth->trait_skipped ?
                SV_BIRTH_RACE : SV_BIRTH_TRAIT;
#else
            birth->owner = SV_BIRTH_CLASS;
#endif
        }
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
    if (!birth || generation != birth->generation || !birth->active || birth->quit)
        return SV_BIRTH_INPUT_IGNORED;
    if (command.kind == SV_BIRTH_AUTOMATIC_DNA) {
        if (birth->owner < SV_BIRTH_SEX || birth->owner > SV_BIRTH_MODE)
            return SV_BIRTH_INPUT_IGNORED;
        if (birth->owner == SV_BIRTH_STATS && !birth->stats_point_buy)
            return SV_BIRTH_INPUT_IGNORED;
        SvBirthInputResult result = SV_BIRTH_INPUT_HANDLED;
        birth->auto_reincarnation = true;
        while (birth->auto_reincarnation && birth->owner >= SV_BIRTH_SEX &&
               birth->owner <= SV_BIRTH_MODE) {
            SvBirthOwner owner = birth->owner;
            result = sv_birth_command(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_DNA});
            if (result == SV_BIRTH_INPUT_IGNORED || birth->owner == owner)
                break;
        }
        return result;
    }
    if (birth->owner == SV_BIRTH_CLASS) {
        SvBirthInputResult result = class_command(birth, command);
#if SV_CLASS_BEFORE_RACE
        if (result == SV_BIRTH_INPUT_CONFIRMED) prepare_races(birth);
#endif
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
        if (result == SV_BIRTH_INPUT_CONFIRMED && birth->owner == SV_BIRTH_BODY)
            prepare_bodies(birth);
        return result;
    }
    if (birth->owner == SV_BIRTH_BODY) {
        SvBirthInputResult result = body_command(birth, command);
#if !SV_CLASS_BEFORE_RACE
        if (result == SV_BIRTH_INPUT_CONFIRMED) prepare_classes(birth);
#endif
        return result;
    }
    if (birth->owner == SV_BIRTH_STATS) {
        SvBirthInputResult result = stats_command(birth, command);
        if (result == SV_BIRTH_INPUT_CONFIRMED && birth->owner == SV_BIRTH_MODE) {
            prepare_modes(birth);
            if (!birth->stats_point_buy && birth->auto_reincarnation)
                return sv_birth_command(birth, generation,
                    (SvBirthCommand){.kind = SV_BIRTH_AUTOMATIC_DNA});
        }
        return result;
    }
    if (birth->owner == SV_BIRTH_MODE)
        return mode_command(birth, command);
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
    case SV_BIRTH_SELECT_STAT:
    case SV_BIRTH_SELECT_MODE:
    case SV_BIRTH_DECREASE_STAT:
    case SV_BIRTH_INCREASE_STAT:
    case SV_BIRTH_AUTOMATIC_DNA:
    case SV_BIRTH_BACK:
        return SV_BIRTH_INPUT_IGNORED;
    }
    return SV_BIRTH_INPUT_IGNORED;
}

void sv_birth_close(SvBirth *birth, uint64_t generation)
{
    if (!birth || generation != birth->generation || !birth->active) return;
    birth->active = false;
    birth->closed = true;
    birth->owner = SV_BIRTH_NONE;
    birth->setup = NULL;
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
        .highlighted_stat = birth->highlighted_stat,
        .legal_stat_count = birth->legal_stat_count,
        .stats_order_count = birth->stats_order_count,
        .stats_point_buy = birth->stats_point_buy,
        .stats_confirmation_pending = birth->stats_confirmation_pending,
        .stats_remaining = birth->stats_remaining,
        .highlighted_mode = birth->highlighted_mode,
        .legal_mode_count = birth->legal_mode_count,
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
        for (unsigned i = 0; i < 6; ++i) view->stats[i] = birth->stats[i];
        for (unsigned i = 0; i < birth->legal_stat_count; ++i)
            view->legal_stats[i] = birth->legal_stats[i];
        for (unsigned i = 0; i < birth->legal_mode_count; ++i)
            view->legal_modes[i] = birth->legal_modes[i];
    }
}
