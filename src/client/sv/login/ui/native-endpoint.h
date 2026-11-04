#ifndef SV_NATIVE_ENDPOINT_H
#define SV_NATIVE_ENDPOINT_H
#include "login/servers/endpoint.h"
#include <SDL3/SDL.h>
#include "login/ui/view.h"
typedef struct {
    uint64_t since_ns;
    SDL_Keymod latch;
    SvTextResult text_error;
    bool clipboard_unavailable;
    size_t hover, hover_link;
    bool hover_enter;
    float chronicle_scroll; /* Normalized 0..1 of current scrollable news height. */
    bool hover_manual, list_focus_manual;
    int manual_focus;
    float text_cell;
} SvEndpointInput;
/* The provider boundary is the OS clipboard. NULL selects SDL's real clipboard. */
typedef char *(*SvClipboardRead)(void *context);
void sv_endpoint_input_begin(SvEndpointInput *input);
void sv_endpoint_view_native(SvEndpointView *view, const SvEndpointInput *input);
/* Contact credentials have no established Unicode-to-wire mapping. Keep the
 * private draft intact until the source bytes are unambiguous ASCII. */
SvTextResult sv_contact_field_insert(SvTextField *field, const char *utf8);
size_t sv_contact_draft_limit(bool password);
bool sv_endpoint_links_event(SvEndpointInput *input,const SDL_Event *event);
bool sv_endpoint_chronicle_event(SvEndpointInput *input,const SDL_Event *event);
bool sv_endpoint_event(SvEndpointInput *input, SvEndpoint *endpoint,
                       const SDL_Event *event, SvClipboardRead clipboard, void *context);
#endif
