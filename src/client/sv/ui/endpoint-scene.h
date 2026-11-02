#ifndef SV_ENDPOINT_SCENE_H
#define SV_ENDPOINT_SCENE_H
#include "input/native-endpoint.h"
#include "ui/font.h"
/* Optional owner-thread observer after rendering and before submission. Private
 * fields must be masked before an observer may inspect the frame. */
typedef struct {
    void *context;
    bool (*before_present)(void *context, SDL_Renderer *renderer);
} SvEndpointFrameObserver;
void sv_endpoint_observe_frames(SvEndpointFrameObserver observer);
bool sv_endpoint_draw(SDL_Renderer *renderer, SvFont *font,
                      const SvEndpoint *endpoint, const SvEndpointInput *input);
bool sv_endpoint_render(SDL_Renderer *renderer, SvFont *font,
                        const SvEndpoint *endpoint, const SvEndpointInput *input);
#endif
