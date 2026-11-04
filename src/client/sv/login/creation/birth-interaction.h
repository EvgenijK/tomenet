#ifndef SV_LOGIN_CREATION_BIRTH_INTERACTION_H
#define SV_LOGIN_CREATION_BIRTH_INTERACTION_H
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
    SV_BIRTH_CLASS_NONE = -1,
    SV_BIRTH_CLASS_ARCHER = 5
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
    /* Historical pre-4.4.1a mode identity (MODE_FRUIT_BAT_OLD). */
    SV_BIRTH_MODE_FRUIT_BAT = 0x20,
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
    SV_BIRTH_AUTOMATIC_DNA,
    SV_BIRTH_CANCEL_STATS_CONFIRMATION,
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
    SV_BIRTH_INPUT_PLACEHOLDER,
    SV_BIRTH_INPUT_QUIT,
    SV_BIRTH_INPUT_BACK_TO_NAME
} SvBirthInputResult;

/* Mutable state belongs to the character-creation interaction. */
typedef struct SvBirth SvBirth;



typedef struct SvBirthView {
    SvBirthSex chosen_sex;
    int chosen_class, chosen_race, chosen_trait;
    SvBirthBody chosen_body;
    SvBirthMode chosen_mode;
    bool class_confirmed, race_confirmed, trait_confirmed, body_confirmed;
    bool stats_prepared, stats_confirmed, mode_confirmed;
    bool stat_can_increase[6], stat_can_decrease[6];
    bool dna_class_available, dna_race_available, dna_trait_available;
    bool dna_body_available, dna_stats_available, dna_mode_available;
    SvBirthOwner owner;
    SvBirthSex highlighted_sex;
    int highlighted_class;
    uint8_t visible_classes[255], visible_class_count;
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
    SvBirthMode legal_modes[7];
    uint8_t legal_mode_count;
    const SvContactSetup *setup;
    bool dna_available;
} SvBirthView;

/* Value snapshot for lifecycle observation and existing diagnostics. It owns
 * its data and cannot be applied back to the interaction. UI uses SvBirthView;
 * transport uses the validated SvBirthPlayDraft instead of these draft values. */
typedef struct {
    uint64_t generation;
    SvBirthOwner owner;
    SvBirthSex highlighted_sex, chosen_sex;
    int highlighted_class, chosen_class;
    uint8_t visible_classes[255], visible_class_count;
    uint8_t legal_classes[255], legal_class_count;
    int highlighted_race, chosen_race;
    uint8_t legal_races[255], legal_race_count, race_count;
    int highlighted_trait, chosen_trait;
    uint8_t legal_traits[255], legal_trait_count, trait_count;
    SvBirthBody highlighted_body, chosen_body;
    SvBirthBody legal_bodies[2];
    uint8_t legal_body_count;
    bool dna_available, auto_reincarnation;
    bool entered, active, closed, keyboard_navigated, mouse_navigated;
    bool random_used, dna_used, illegal_ignored, confirmed, quit;
    bool class_confirmed, class_random_used, class_dna_used;
    bool race_confirmed, race_random_used, race_dna_used;
    bool trait_confirmed, trait_random_used, trait_dna_used, trait_skipped;
    bool body_confirmed, body_random_used, body_dna_used, body_skipped;
    int highlighted_stat;
    int16_t stats[6], committed_stats[6];
    uint8_t legal_stats[6], legal_stat_count, stats_order_count;
    bool stats_confirmed, stats_random_used, stats_dna_used;
    bool stats_confirmation_pending, stats_point_buy;
    int stats_remaining;
    SvBirthMode highlighted_mode, chosen_mode;
    SvBirthMode legal_modes[7];
    uint8_t legal_mode_count;
    bool mode_confirmed, mode_random_used, mode_dna_used, mode_ready;
} SvBirthSnapshot;

/* Creation returns zero-initialized inactive state, or NULL on allocation
 * failure. Close applies generation-aware interaction semantics; destroy frees
 * storage. The borrowed setup passed to begin must outlive the interaction. */
SvBirth *sv_birth_create(void);
void sv_birth_destroy(SvBirth *birth);
void sv_birth_snapshot_prepare(SvBirthSnapshot *snapshot, const SvBirth *birth);

SvResult sv_birth_begin(SvBirth *birth, uint64_t generation,
                        const SvContactSetup *setup, const uint32_t server_flags[4],
                        bool first_run, uint32_t dedicated_mode_flags,
                        bool dna_available,
                        SvBirthSex dna_sex, bool dna_class_available,
                        int dna_class, bool dna_race_available, int dna_race,
                        bool dna_trait_available, int dna_trait,
                        bool dna_body_available, SvBirthBody dna_body,
                        bool dna_stats_available, const int16_t dna_stats[6],
                        bool dna_mode_available, SvBirthMode dna_mode);
SvBirthInputResult sv_birth_command(SvBirth *birth, uint64_t generation,
                                    SvBirthCommand command);
/* Committed creation identity for the transport owner; unfinished drafts
 * cannot cross this public interaction boundary. */
typedef struct {
    int16_t mode, race, class_id, trait, stats[6];
} SvBirthPlayDraft;
bool sv_birth_committed_play(const SvBirth *birth, uint64_t generation,
                             SvBirthPlayDraft *draft);
void sv_birth_close(SvBirth *birth, uint64_t generation);
/* Arrays/scalars are copied; view->setup is borrowed and is valid only while
 * the owning attempt remains alive. Do not retain it across flow mutation or
 * teardown. */
void sv_birth_view_prepare(SvBirthView *view, const SvBirth *birth);

#endif
