#ifndef SV_INPUT_H
#define SV_INPUT_H
#include <stdbool.h>
#include "session/session.h"
typedef enum { SV_CONTEXT_GAME, SV_CONTEXT_KEY_REQUEST } SvInputContext;
typedef enum { SV_MACRO_NONE, SV_MACRO_NORMAL, SV_MACRO_HYBRID, SV_MACRO_COMMAND } SvMacroKind;
typedef struct { unsigned char action; SvMacroKind kind; } SvMacro;
#define SV_PHYSICAL_BINDINGS 32
#define SV_PHYSICAL_BYTES 32
typedef struct {
    unsigned char bytes[SV_PHYSICAL_BYTES];
    size_t size;
    unsigned char action;
    SvMacroKind kind;
} SvPhysicalBinding;
/* Physical trigger bytes share the production binding table with ASCII keys;
 * actions remain single-byte until the later macro editor owner. */
typedef struct {
    SvMacro keys[128];
    SvPhysicalBinding physical[SV_PHYSICAL_BINDINGS];
    size_t physical_count;
} SvInputBindings;
#define SV_KEY_PENDING 32
typedef struct { uint64_t sequence; unsigned char key; } SvQueuedKey;
typedef struct {
    SvInputContext context, parent;
    uint64_t sequence;
    uint64_t pause_sequence;
    bool paused;
    uint64_t confirmation_sequence;
    unsigned char confirmations[32];
    size_t confirmation_head, confirmation_count;
    bool confirmation_waiting;
    SvQueuedKey pending[SV_KEY_PENDING];
    size_t head, count;
} SvInputRouter;
typedef struct { uint64_t sequence; int id; unsigned char key; int cancelled; } SvKeyReply;
/* Owns semantic context, not request text or renderer state.
 * Sync returns OK with one prepared reply, or WAITING without a command. */
SvResult sv_input_sync(SvInputRouter *router, SvKeyRequest request, SvKeyReply *reply);
SvResult sv_input_key(const SvInputRouter *router, SvKeyRequest request,
                      uint64_t sequence, unsigned char key, SvKeyReply *reply);
SvResult sv_input_bind(SvInputBindings *bindings, unsigned char trigger,
                       unsigned char action, SvMacroKind kind);
SvResult sv_input_bind_physical(SvInputBindings *bindings, const unsigned char *bytes,
                                size_t size, unsigned char action, SvMacroKind kind);
SvResult sv_input_physical(const SvInputBindings *bindings, const unsigned char *bytes,
                           size_t size, bool prompt, unsigned char *key);
SvResult sv_input_accept(SvInputRouter *router, const SvInputBindings *bindings,
                         SvKeyRequest request, uint64_t sequence, unsigned char key);
SvResult sv_input_next(SvInputRouter *router, SvKeyRequest request, SvKeyReply *reply);
uint64_t sv_input_pause(SvInputRouter *router);
SvResult sv_input_ack_pause(SvInputRouter *router, uint64_t sequence);
SvResult sv_input_begin_confirmation(SvInputRouter *router, uint64_t *sequence);
SvResult sv_input_confirm(SvInputRouter *router, unsigned char command);
SvResult sv_input_take_confirmation(SvInputRouter *router, uint64_t sequence,
                                    unsigned char *command);
SvResult sv_input_end_confirmation(SvInputRouter *router, uint64_t sequence);
#endif
