#include "input/input.h"

uint64_t sv_input_pause(SvInputRouter *router)
{
    ++router->pause_sequence;
    router->paused = true;
    router->head = router->count = 0;
    return router->pause_sequence;
}

SvResult sv_input_ack_pause(SvInputRouter *router, uint64_t sequence)
{
    if (!router->paused || router->pause_sequence != sequence) return SV_STALE;
    router->paused = false;
    router->head = router->count = 0;
    return SV_OK;
}
SvResult sv_input_begin_confirmation(SvInputRouter *router, uint64_t *sequence)
{
    if (!router || !sequence || router->confirmation_sequence == UINT64_MAX) return SV_INVALID;
    if (router->confirmation_waiting) return SV_BUSY;
    router->confirmation_waiting = true;
    router->confirmation_head = router->confirmation_count = 0;
    *sequence = ++router->confirmation_sequence;
    return SV_OK;
}
SvResult sv_input_confirm(SvInputRouter *router, unsigned char command)
{
    if (!router->confirmation_waiting) return SV_WAITING;
    if (router->confirmation_count == sizeof(router->confirmations)) return SV_EVENT_OVERFLOW;
    router->confirmations[(router->confirmation_head + router->confirmation_count) %
        sizeof(router->confirmations)] = command;
    ++router->confirmation_count;
    return SV_OK;
}
SvResult sv_input_take_confirmation(SvInputRouter *router, uint64_t sequence,
                                    unsigned char *command)
{
    if (!command) return SV_INVALID;
    if (!router->confirmation_waiting || sequence != router->confirmation_sequence)
        return SV_STALE;
    if (!router->confirmation_count) return SV_WAITING;
    *command = router->confirmations[router->confirmation_head];
    router->confirmations[router->confirmation_head] = 0;
    router->confirmation_head = (router->confirmation_head + 1) %
        sizeof(router->confirmations);
    --router->confirmation_count;
    return SV_OK;
}
SvResult sv_input_end_confirmation(SvInputRouter *router, uint64_t sequence)
{
    if (!router->confirmation_waiting || sequence != router->confirmation_sequence)
        return SV_STALE;
    router->confirmation_waiting = false;
    router->confirmation_head = router->confirmation_count = 0;
    return SV_OK;
}
#include <string.h>
static int supported(unsigned char key)
{
    return (key >= 32 && key < 127) || key == 8 || key == 9 || key == 13 || key == 27 ||
           (key >= 1 && key <= 26);
}
SvResult sv_input_bind(SvInputBindings *bindings, unsigned char trigger,
                       unsigned char action, SvMacroKind kind)
{
    /* ASCII 96 inside an action is baseline MACRO_WAIT, requiring operands and
     * timing semantics outside this one-key profile. Never reinterpret it. */
    if (!supported(trigger) || !supported(action) || action == '`' ||
        kind < SV_MACRO_NONE || kind > SV_MACRO_COMMAND)
        return SV_INVALID;
    bindings->keys[trigger] = (SvMacro){action, kind};
    return SV_OK;
}
SvResult sv_input_bind_physical(SvInputBindings *bindings, const unsigned char *bytes,
                                size_t size, unsigned char action, SvMacroKind kind)
{
    if (!bindings || !bytes || !size || size > SV_PHYSICAL_BYTES || !supported(action) ||
        action == '`' || kind < SV_MACRO_NORMAL || kind > SV_MACRO_COMMAND) return SV_INVALID;
    for (size_t i = 0; i < bindings->physical_count; ++i) {
        SvPhysicalBinding *binding = &bindings->physical[i];
        if (binding->size != size || memcmp(binding->bytes, bytes, size)) continue;
        binding->action = action;
        binding->kind = kind;
        return SV_OK;
    }
    if (bindings->physical_count == SV_PHYSICAL_BINDINGS) return SV_INPUT_OVERFLOW;
    SvPhysicalBinding *binding = &bindings->physical[bindings->physical_count++];
    memcpy(binding->bytes, bytes, size);
    binding->size = size;
    binding->action = action;
    binding->kind = kind;
    return SV_OK;
}
SvResult sv_input_physical(const SvInputBindings *bindings, const unsigned char *bytes,
                           size_t size, bool prompt, unsigned char *key)
{
    if (!bindings || !bytes || !size || size > SV_PHYSICAL_BYTES || !key) return SV_INVALID;
    /* A physical trigger is matched as one event. Its interior bytes cannot
     * leak into the gameplay packet stream on a failed macro match. */
    for (size_t i = bindings->physical_count; i; --i) {
        const SvPhysicalBinding *binding = &bindings->physical[i - 1];
        if (binding->size != size || memcmp(binding->bytes, bytes, size)) continue;
        if (prompt && binding->kind == SV_MACRO_COMMAND) break;
        *key = binding->action;
        return SV_OK;
    }
    if (size != 1) return SV_WAITING;
    *key = bytes[0];
    return supported(*key) ? SV_OK : SV_INVALID;
}
SvResult sv_input_accept(SvInputRouter *router, const SvInputBindings *bindings,
                         SvKeyRequest request, uint64_t sequence, unsigned char key)
{
    if (!request.pending || request.sequence != sequence || router->sequence != sequence)
        return SV_STALE;
    if (!supported(key)) return SV_INVALID;
    if (router->count == SV_KEY_PENDING) return SV_KEY_OVERFLOW;
    /* c-util.c macro_ready/get_com: normal and hybrid are active in this
     * non-shopping, non-message prompt; command macros require inkey_flag.
     * inkey_aux never rematches expansion bytes. inkey maps backquote last. */
    SvMacro macro = bindings->keys[key];
    if (macro.kind == SV_MACRO_NORMAL || macro.kind == SV_MACRO_HYBRID) key = macro.action;
    if (key == '`') key = 27;
    router->pending[(router->head + router->count) % SV_KEY_PENDING] = (SvQueuedKey){sequence, key};
    ++router->count;
    return SV_OK;
}
SvResult sv_input_next(SvInputRouter *router, SvKeyRequest request, SvKeyReply *reply)
{
    if (!router->count) return SV_WAITING;
    SvQueuedKey key = router->pending[router->head];
    router->pending[router->head] = (SvQueuedKey){0};
    router->head = (router->head + 1) % SV_KEY_PENDING;
    --router->count;
    return sv_input_key(router, request, key.sequence, key.key, reply);
}
SvResult sv_input_sync(SvInputRouter *router, SvKeyRequest request, SvKeyReply *reply)
{
    if (request.pending && !router->sequence) {
        router->parent = router->context;
        router->context = SV_CONTEXT_KEY_REQUEST;
        router->sequence = request.sequence;
    } else if (!request.pending && router->sequence) {
        router->context = router->parent;
        router->sequence = 0;
    }
    /* This owner follows get_com/inkey: server abort completes with key zero. */
    if (request.pending && request.aborted)
        return sv_input_key(router, request, request.sequence, 27, reply);
    return SV_WAITING;
}
SvResult sv_input_key(const SvInputRouter *router, SvKeyRequest request,
                      uint64_t sequence, unsigned char key, SvKeyReply *reply)
{
    if (!request.pending || request.sequence != sequence || router->sequence != sequence)
        return SV_STALE;
    *reply = (SvKeyReply){sequence, request.id, key == 27 ? 0 : key, key == 27};
    return SV_OK;
}
