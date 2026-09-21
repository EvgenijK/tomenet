#ifndef SV_MESSAGE_TEXT_H
#define SV_MESSAGE_TEXT_H
#include "session.h"
/* Derived ASCII visual profile. Unmapped byte symbols use one '?' cell.
 * Raw field bytes remain solely authoritative in SvMessage. */
typedef struct {
    char text[SV_MESSAGE_BYTES];
    unsigned char colors[SV_MESSAGE_BYTES];
    size_t length;
} SvMessageText;
SvMessageText sv_message_text(const SvMessage *message);
typedef struct {
    uint64_t generation, revision;
    int valid;
    size_t count;
    SvMessageText lines[SV_MESSAGE_LINES];
} SvMessageCache;
/* Returns nonzero only when the derived text changes. Geometry is owned by UI. */
int sv_messages_prepare(SvMessageCache *cache, uint64_t generation, SvMessages messages);
#endif
