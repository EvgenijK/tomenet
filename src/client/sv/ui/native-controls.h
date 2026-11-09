#ifndef SV_NATIVE_CONTROLS_H
#define SV_NATIVE_CONTROLS_H
#include "input/native-input.h"
/* Native executor: draw reads only this coherent view; this module owns Present,
 * input freshness and pacing. Borrowed renderer/input/context outlive binding. */
typedef struct {
    SDL_Renderer *renderer;
    SvNativeInput *input;
    void *context;
    bool (*draw)(void *context, SvAppView view);
    uint64_t generation, due_ns, frame_ms;
    bool batch;
    size_t remaining;
} SvNativeControls;
SvResult sv_native_controls_bind(SvNativeControls *controls, SvApp *app);
/* One bounded native iteration; a pacing deadline yields immediately. Transport
 * receive/output, keepalive and window events remain owned by the outer loop. */
SvStep sv_native_controls_step(SvNativeControls *controls, SvApp *app, size_t budget);
#endif
