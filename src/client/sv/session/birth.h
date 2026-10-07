#ifndef SV_SESSION_BIRTH_H
#define SV_SESSION_BIRTH_H
#include "result.h"
#include "session/character-setup.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef SV_CLASS_BEFORE_RACE
#define SV_CLASS_BEFORE_RACE 1
#endif
#if SV_CLASS_BEFORE_RACE != 0 && SV_CLASS_BEFORE_RACE != 1
#error SV_CLASS_BEFORE_RACE must be 0 or 1
#endif

typedef enum {
    SV_BIRTH_NONE,
    SV_BIRTH_SEX,
    SV_BIRTH_CLASS,
    SV_BIRTH_RACE,
    SV_BIRTH_TRAIT,
    SV_BIRTH_BODY,
    SV_BIRTH_STATS,
    SV_BIRTH_MODE,
    SV_BIRTH_READY
} SvBirthOwner;

typedef enum {
    SV_BIRTH_SEX_NONE = -1,
    SV_BIRTH_SEX_MALE,
    SV_BIRTH_SEX_FEMALE
} SvBirthSex;

typedef enum {
    SV_BIRTH_CLASS_NONE = -1
} SvBirthClass;

typedef enum {
    SV_BIRTH_RACE_NONE = -1
} SvBirthRace;

typedef enum {
    SV_BIRTH_TRAIT_NONE = -1
} SvBirthTrait;

typedef enum {
    SV_BIRTH_BODY_NONE = -1,
    SV_BIRTH_BODY_NORMAL,
    SV_BIRTH_BODY_FRUIT_BAT
} SvBirthBody;

typedef enum {
    SV_BIRTH_MODE_NONE = -1,
    SV_BIRTH_MODE_NORMAL = 0x00,
    SV_BIRTH_MODE_SOLOIST = 0x05,
    SV_BIRTH_MODE_HELLISH = 0x06,
    SV_BIRTH_MODE_NO_GHOST = 0x04,
    SV_BIRTH_MODE_EVERLASTING = 0x08,
    SV_BIRTH_MODE_PVP = 0x10,
    SV_BIRTH_MODE_IDDC = 0x44,
    SV_BIRTH_MODE_IDDC_SOLOIST = 0x45,
    SV_BIRTH_MODE_IDDC_HELLISH = 0x46,
    SV_BIRTH_MODE_DEDICATED_PVP = 0x90
} SvBirthMode;

typedef enum {
    SV_BIRTH_MOVE_PREVIOUS,
    SV_BIRTH_MOVE_NEXT,
    SV_BIRTH_SELECT_SEX,
    SV_BIRTH_SELECT_CLASS,
    SV_BIRTH_SELECT_RACE,
    SV_BIRTH_SELECT_TRAIT,
    SV_BIRTH_SELECT_BODY,
    SV_BIRTH_SELECT_STAT,
    SV_BIRTH_SELECT_MODE,
    SV_BIRTH_DECREASE_STAT,
    SV_BIRTH_INCREASE_STAT,
    SV_BIRTH_RANDOM,
    SV_BIRTH_DNA,
    SV_BIRTH_CONFIRM,
    SV_BIRTH_BACK,
    SV_BIRTH_QUIT
} SvBirthCommandKind;

typedef struct {
    SvBirthCommandKind kind;
    SvBirthSex sex;
    int class_id;
    int race_id;
    int trait_id;
    SvBirthBody body;
    int stat;
    SvBirthMode mode;
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
    int highlighted_class, chosen_class, dna_class;
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
    uint8_t legal_stats[6], legal_stat_count, stats_order_count;
    bool stats_prepared, stats_confirmed, stats_random_used, stats_dna_used;
    bool stats_confirmation_pending;
    bool stats_point_buy;
    int stats_remaining;
    SvBirthMode highlighted_mode, chosen_mode;
    SvBirthMode legal_modes[6];
    uint8_t legal_mode_count;
    bool mode_confirmed, mode_random_used, mode_dna_used, mode_ready;
    bool post_mode_trait_required;
    uint32_t server_flags[4];
    size_t existing_characters;
    uint8_t race_count, trait_count;
    bool dedicated;
    uint32_t random_state;
    const SvContactSetup *setup;
} SvBirth;

typedef struct SvBirthView {
    SvBirthOwner owner;
    SvBirthSex highlighted_sex;
    int highlighted_class;
    uint8_t legal_classes[255], legal_class_count;
    int highlighted_race;
    uint8_t legal_races[255], legal_race_count;
    int highlighted_trait;
    uint8_t legal_traits[255], legal_trait_count;
    SvBirthBody highlighted_body;
    SvBirthBody legal_bodies[2];
    uint8_t legal_body_count;
    int highlighted_stat;
    int16_t stats[6];
    uint8_t legal_stats[6], legal_stat_count, stats_order_count;
    bool stats_point_buy;
    bool stats_confirmation_pending;
    int stats_remaining;
    SvBirthMode highlighted_mode;
    SvBirthMode legal_modes[6];
    uint8_t legal_mode_count;
    const SvContactSetup *setup;
    bool dna_available;
} SvBirthView;

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, const uint32_t server_flags[4],
                        size_t existing_characters, bool dedicated,
                        bool dna_available,
                        SvBirthSex dna_sex, bool dna_class_available,
                        int dna_class, bool dna_race_available, int dna_race,
                        bool dna_trait_available, int dna_trait,
                        bool dna_body_available, SvBirthBody dna_body,
                        bool dna_stats_available, const int16_t dna_stats[6],
                        bool dna_mode_available, SvBirthMode dna_mode);
SvBirthInputResult sv_birth_command(SvBirth *birth, uint64_t generation,
                                    SvBirthCommand command);
void sv_birth_close(SvBirth *birth, uint64_t generation);
void sv_birth_view_prepare(SvBirthView *view, const SvBirth *birth);

#endif
