#ifndef SV_PROTOCOL_STARTUP_PREFIX_H
#define SV_PROTOCOL_STARTUP_PREFIX_H
#include "result.h"
#include <stdbool.h>

#define SV_STARTUP_PREFIX_LIMIT (128u * 1024u)
typedef struct {
    int version[6];
    unsigned graphics, char_transfer_bytes;
} SvStartupPrefixConfig;
/* Framing only for known startup packets whose gameplay owners come later.
 * Successful callers retain the exact original frame in wire order; this API
 * applies no gameplay, rendering or acknowledgement effects. LINE_INFO always
 * describes 80 protocol columns, independent of the map viewport's 66 columns.
 * Config is the actual sent PLAY setup, including effective glyph byte width. */
bool sv_startup_prefix_supports(unsigned char type);
/* OK consumes one whole frame; WAITING consumes nothing. Unsupported is
 * INVALID, malformed is DECODE_ERROR, and a frame beyond the retention budget
 * is INPUT_OVERFLOW. None of those outcomes permits searching/skipping bytes. */
SvResult sv_startup_prefix_decode(const SvStartupPrefixConfig *, const void *,
    size_t available, size_t *consumed);
#endif
