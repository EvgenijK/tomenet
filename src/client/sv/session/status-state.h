#ifndef SV_STATUS_STATE_H
#define SV_STATUS_STATE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define SV_STATUS_TEXT 256
#define SV_STATUS_EFFECT_CAPACITY 64
/* Logical fields, independent of packet numbers and renderer placement. */
typedef enum {
    SV_HUD_HP, SV_HUD_MANA, SV_HUD_STAMINA, SV_HUD_SANITY,
    SV_HUD_STATS, SV_HUD_ARMOUR, SV_HUD_DEMOGRAPHICS, SV_HUD_COMBAT,
    SV_HUD_EXPERIENCE, SV_HUD_CURRENCY, SV_HUD_CONFUSION, SV_HUD_POISON,
    SV_HUD_STUDY, SV_HUD_BLOWS, SV_HUD_FOOD, SV_HUD_FEAR, SV_HUD_SPEED,
    SV_HUD_CUT, SV_HUD_BLIND, SV_HUD_STUN, SV_HUD_MONSTER,
    SV_HUD_AFK, SV_HUD_ENCUMBRANCE, SV_HUD_EXTRA, SV_HUD_MARTYR,
    SV_HUD_IDLE, SV_HUD_INDICATORS, SV_HUD_DEPTH, SV_HUD_FIELD_COUNT
} SvHudField;
typedef struct { int maximum, current; bool bar, boosted; unsigned char drain; } SvHudResource;
typedef struct { int maximum, current, index, base, temporary; bool boosted, available; } SvHudStat;
typedef struct { unsigned char attribute; char label[SV_STATUS_TEXT]; int damage, current, maximum; bool numeric_available, damage_available; } SvHudSanity;
typedef struct { int base, bonus; bool unknown, boosted; } SvHudArmour;
typedef struct { int height, weight, age, social_class; char body[SV_STATUS_TEXT]; } SvHudDemographics;
typedef struct { int hit, damage, ranged_hit, ranged_damage, melee_hit, melee_damage; } SvHudCombat;
typedef struct { int level, maximum_level, maximum_player_level; int32_t maximum, current, advance, previous; bool fractional; } SvHudExperience;
typedef struct { int32_t carried, bank; } SvHudCurrency;
typedef struct { unsigned char value, attribute; char label[SV_STATUS_TEXT]; } SvHudBlows;
typedef struct { unsigned char health, attribute; } SvHudMonster;
typedef struct { int x, y, z, town; unsigned char colour, sector_colour; char location[SV_STATUS_TEXT], location_secondary[SV_STATUS_TEXT], preposition[SV_STATUS_TEXT]; bool no_tele; } SvHudDepth;
typedef struct {
    uint64_t generation, revision, available;
    SvHudResource hp, mana, stamina;
    SvHudSanity sanity;
    SvHudStat stats[6];
    SvHudArmour armour;
    SvHudDemographics demographics;
    SvHudCombat combat;
    SvHudExperience experience;
    SvHudCurrency currency;
    int confusion, poison, study, food, fear, speed, cut, blind, stun;
    bool speed_boosted;
    SvHudBlows blows;
    SvHudMonster monster;
    int afk, martyr, idle;
    unsigned char encumbrance[15];
    char extra[SV_STATUS_TEXT];
    uint32_t indicators;
    SvHudDepth depth;
} SvHud;
/* One complete decoded change; only the selected field is consumed. */
typedef struct { SvHudField field; SvHud values; int stat_index; } SvHudUpdate;
typedef enum { SV_STATUS_BODY_EFFECT, SV_STATUS_IDLE_EFFECT } SvStatusEffectKind;
typedef struct {
    uint64_t generation, sequence;
    SvStatusEffectKind kind;
    union { char body[SV_STATUS_TEXT]; int idle; };
} SvStatusEffect;
static inline bool sv_hud_has(const SvHud *hud, SvHudField field)
{ return hud && (hud->available & (UINT64_C(1) << field)) != 0; }
#endif
