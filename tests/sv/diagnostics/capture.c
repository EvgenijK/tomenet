#include "diagnostics/capture.h"
#include "ui/endpoint-scene.h"
#include <string.h>

static bool capture_frame(void *context, SDL_Renderer *renderer)
{
    const char *capture = context;
    SDL_PropertiesID properties = SDL_GetRendererProperties(renderer);
    const char *counter = "sv.login.capture.frame";
    Sint64 frame = SDL_GetNumberProperty(properties, counter, 0) + 1;
    if (!SDL_SetNumberProperty(properties, counter, frame)) return false;
    const char *delay = SDL_getenv("SV_B2_CAPTURE_FRAME");
    int target = delay ? SDL_atoi(delay) : 1;
    if (target < 1) target = 1;
    if (frame == target) {
        SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
        if (!surface) return false;
        bool saved = SDL_SaveBMP(surface, capture);
        SDL_DestroySurface(surface);
        if (!saved) return false;
    }
    return true;
}

void sv_diagnostic_capture_configure(void)
{
    const char *capture = SDL_getenv("SV_B2_CAPTURE");
    sv_endpoint_observe_frames((SvEndpointFrameObserver){
        .context = (void *)capture,
        .before_present = capture ? capture_frame : NULL});
}

SvLoginShellOptions sv_diagnostic_shell_options(void)
{
    const char *reduced = SDL_getenv("SV_B2_REDUCED_MOTION");
    const char *background = SDL_getenv("SV_B2_BACKGROUND");
    return (SvLoginShellOptions){
        .reduced_motion = reduced && strcmp(reduced, "0"),
        .background_selected = background != NULL,
        .background = background ? (unsigned)SDL_atoi(background) : 0};
}
