#ifndef SV_NATIVE_ENDPOINT_H
#define SV_NATIVE_ENDPOINT_H
#include "input/endpoint.h"
#include <SDL3/SDL.h>
typedef struct {
    uint64_t since_ns;
    SDL_Keymod latch;
    SvTextResult text_error;
    bool clipboard_unavailable;
    const char *contact_status;
} SvEndpointInput;
/* The provider boundary is the OS clipboard. NULL selects SDL's real clipboard. */
typedef char *(*SvClipboardRead)(void *context);
void sv_endpoint_input_begin(SvEndpointInput *input);
/* Contact credentials have no established Unicode-to-wire mapping. Keep the
 * private draft intact until the source bytes are unambiguous ASCII. */
SvTextResult sv_contact_field_insert(SvTextField *field, const char *utf8);
bool sv_endpoint_event(SvEndpointInput *input, SvEndpoint *endpoint,
                       const SDL_Event *event, SvClipboardRead clipboard, void *context);
#endif
