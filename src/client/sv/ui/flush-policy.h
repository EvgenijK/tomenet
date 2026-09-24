#ifndef SV_FLUSH_POLICY_H
#define SV_FLUSH_POLICY_H
#include <stdint.h>

typedef struct { int disable_flush, thin_down_flush; } SvFlushOptions;
typedef struct {
    SvFlushOptions options;
    unsigned count;
    uint64_t frame_ms;
} SvFlushPolicy;

static inline void sv_flush_frame(SvFlushPolicy *policy, uint64_t now_ms)
{
    policy->frame_ms = now_ms;
    policy->count = 0;
}

static inline uint64_t sv_flush_request(SvFlushPolicy *policy)
{
    ++policy->count;
    return policy->frame_ms + (!policy->options.disable_flush &&
        (!policy->options.thin_down_flush || policy->count <= 10));
}
#endif
