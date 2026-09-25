#ifndef SV_ENDPOINT_SCENE_H
#define SV_ENDPOINT_SCENE_H
#include "input/native-endpoint.h"
#include "ui/font.h"
bool sv_endpoint_draw(SDL_Renderer *renderer, SvFont *font,
                      const SvEndpoint *endpoint, const SvEndpointInput *input,
                      float user_scale);
bool sv_endpoint_render(SDL_Renderer *renderer, SvFont *font,
                        const SvEndpoint *endpoint, const SvEndpointInput *input,
                        float user_scale);
#endif
