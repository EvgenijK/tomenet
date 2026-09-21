#ifndef SV_INPUT_H
#define SV_INPUT_H
#include "session.h"
typedef enum { SV_CONTEXT_GAME, SV_CONTEXT_KEY_REQUEST } SvInputContext;
typedef struct {
    SvInputContext context, parent;
    uint64_t sequence;
} SvInputRouter;
typedef struct { uint64_t sequence; int id; unsigned char key; } SvKeyReply;
/* Owns semantic context, not request text or renderer state. */
void sv_input_sync(SvInputRouter *router, SvKeyRequest request);
SvResult sv_input_key(const SvInputRouter *router, SvKeyRequest request,
                      uint64_t sequence, unsigned char key, SvKeyReply *reply);
#endif
