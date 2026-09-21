#include "input.h"
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
    *reply = (SvKeyReply){sequence, request.id, key == 27 ? 0 : key};
    return SV_OK;
}
