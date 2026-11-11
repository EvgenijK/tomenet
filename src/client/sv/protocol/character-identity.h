#ifndef SV_PROTOCOL_CHARACTER_IDENTITY_H
#define SV_PROTOCOL_CHARACTER_IDENTITY_H
#include "result.h"
#include "session/character-identity.h"
/* One PKT_CHAR_INFO frame. WAITING/error consumes zero and leaves out unchanged.
 * Complete output is copied; subsequent frames remain the caller's property. */
SvResult sv_character_identity_decode(const int version[6], const void *bytes,
    size_t available, SvCharacterIdentity *out, size_t *consumed);
#endif
