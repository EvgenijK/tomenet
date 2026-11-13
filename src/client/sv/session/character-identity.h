#ifndef SV_CHARACTER_IDENTITY_H
#define SV_CHARACTER_IDENTITY_H
#include <stdbool.h>
#include <stdint.h>
#include "session/character-setup.h"
/* Accepted server identity. Availability describes wire fields, not guesses
 * from a birth draft. name may contain the accepted login name on old servers. */
typedef struct {
    uint64_t generation, revision;
    bool available, trait_available, lives_available, name_available;
    int16_t race, class_id, trait, sex, lives;
    uint32_t wire_mode, mode;
    bool admin_wiz, admin_dm, fruit_bat;
    unsigned privileged, restricted;
    char name[SV_CONTACT_NAME_CAPACITY];
} SvCharacterIdentity;
static inline bool sv_character_identity_validate(const SvCharacterIdentity *identity,
                                                   const SvContactSetup *setup)
{
    if (!identity || !identity->available || identity->race < 0 ||
        identity->class_id < 0 || identity->trait < 0) return false;
    return !setup || (identity->race < setup->race_count &&
        identity->class_id < setup->class_count &&
        (setup->trait_count ? identity->trait < setup->trait_count : identity->trait == 0));
}
#endif
