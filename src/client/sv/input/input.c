#include "input/input.h"
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
