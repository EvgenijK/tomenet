#ifndef SV_LOGIN_CREATION_BIRTH_PRIVATE_H
#define SV_LOGIN_CREATION_BIRTH_PRIVATE_H

/* Private representation for the core and its native adapter. External
 * callers use birth-interaction.h and never mutate this representation. */
#include "login/creation/birth-interaction.h"

struct SvBirth {
    uint64_t generation;
    SvBirthOwner owner;
    SvBirthSex highlighted_sex, chosen_sex;
    SvBirthSex dna_sex;
    int highlighted_class, chosen_visible_class, chosen_class, requested_class;
    int dna_class;
    int last_confirmed_class;
    uint8_t visible_classes[255], visible_class_count;
    uint8_t legal_classes[255], legal_class_count;
    int highlighted_race, chosen_race, dna_race;
    uint8_t legal_races[255], legal_race_count;
    int highlighted_trait, chosen_trait;
    uint8_t legal_traits[255], legal_trait_count;
    int dna_trait;
    SvBirthBody highlighted_body, chosen_body, dna_body;
    SvBirthBody legal_bodies[2];
    uint8_t legal_body_count;
    bool dna_available, auto_reincarnation;
    bool dna_class_available;
    bool dna_race_available;
    bool dna_trait_available;
    bool dna_body_available;
    bool dna_stats_available;
    int16_t dna_stats[6];
    bool dna_mode_available;
    SvBirthMode dna_mode;
    bool entered, active, closed, keyboard_navigated, mouse_navigated;
    bool random_used, dna_used, illegal_ignored, confirmed, quit;
    bool class_confirmed, class_random_used, class_dna_used;
    bool race_confirmed, race_random_used, race_dna_used;
    bool trait_confirmed, trait_random_used, trait_dna_used, trait_skipped;
    bool body_confirmed, body_random_used, body_dna_used, body_skipped;
    int highlighted_stat;
    int16_t stats[6];
    int16_t committed_stats[6];
    uint8_t legal_stats[6], legal_stat_count, stats_order_count;
    bool stats_prepared, stats_confirmed, stats_random_used, stats_dna_used;
    bool stats_confirmation_pending;
    bool stats_point_buy;
    int stats_remaining;
    SvBirthMode highlighted_mode, chosen_mode;
    SvBirthMode legal_modes[7];
    uint8_t legal_mode_count;
    bool mode_confirmed, mode_random_used, mode_dna_used, mode_ready;
    bool post_mode_trait_required;
    bool pre_mode_trait_saved;
    int pre_mode_highlighted_trait, pre_mode_chosen_trait;
    bool pre_mode_trait_confirmed;
    uint32_t server_flags[4];
    uint8_t race_count, trait_count;
    bool dedicated, first_run;
    uint32_t dedicated_mode_flags;
    uint32_t random_state;
    const SvContactSetup *setup;
};

#endif
