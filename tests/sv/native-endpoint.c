#include "input/native-endpoint.h"
#include <assert.h>
#include <string.h>

static char *read_clipboard(void *context)
{
    return SDL_strdup((const char *)context);
}
static SDL_Event text_event(const char *text)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = text;
    return event;
}
static SDL_Event key_event(SDL_Keycode key, SDL_Keymod mods)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = key;
    switch (key) {
    case SDLK_L: event.key.scancode = SDL_SCANCODE_L; break;
    case SDLK_RETURN: event.key.scancode = SDL_SCANCODE_RETURN; break;
    case SDLK_ESCAPE: event.key.scancode = SDL_SCANCODE_ESCAPE; break;
    default: event.key.scancode = SDL_SCANCODE_A; break;
    }
    event.key.mod = mods;
    return event;
}
int main(void)
{
    SvTextField contact_field;
    sv_text_begin(&contact_field, "pw", 79, true);
    assert(sv_text_select(&contact_field, contact_field.length, contact_field.length));
    assert(sv_contact_field_insert(&contact_field, "\xc3\xa9") == SV_TEXT_ENCODING_ERROR);
    assert(contact_field.length == 2 && !strcmp(contact_field.bytes, "pw"));
    assert(sv_contact_field_insert(&contact_field, "A") == SV_TEXT_OK);
    assert(contact_field.length == 3 && !strcmp(contact_field.bytes, "pwA"));

    SvEndpoint endpoint;
    SvEndpointInput input;
    sv_endpoint_begin(&endpoint, 18348);
    sv_endpoint_input_begin(&input);
    SDL_Event event = text_event("q");
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase == SV_ENDPOINT_MANUAL);
    event = key_event(SDLK_L, SDL_KMOD_CTRL);
    assert(sv_endpoint_event(&input,&endpoint,&event,read_clipboard,"host.example:18349"));
    assert(!strcmp(endpoint.editor.bytes,"host.example:18349"));
    event = key_event(SDLK_RETURN,0);
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase == SV_ENDPOINT_SELECTED && endpoint.port == 18349);
    assert(!strcmp(endpoint.host,"host.example"));

    sv_endpoint_begin(&endpoint, 18348);
    sv_endpoint_manual(&endpoint);
    sv_endpoint_input_begin(&input);
    event = key_event(SDLK_L,SDL_KMOD_CTRL);
    assert(sv_endpoint_event(&input,&endpoint,&event,read_clipboard,"\xE2\x82\xAC"));
    assert(input.text_error == SV_TEXT_ENCODING_ERROR);
    assert(!strcmp(endpoint.editor.bytes,"europe.tomenet.eu"));
    event = key_event(SDLK_ESCAPE,0);
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase == SV_ENDPOINT_CANCELLED && !endpoint.host[0]);
    sv_endpoint_begin(&endpoint,18348);
    sv_endpoint_manual(&endpoint);
    sv_endpoint_input_begin(&input);
    event = key_event(SDLK_LCTRL,0);
    event.key.scancode = SDL_SCANCODE_LCTRL;
    assert(sv_endpoint_event(&input,&endpoint,&event,read_clipboard,"sticky.example"));
    event = key_event(SDLK_L,0);
    assert(sv_endpoint_event(&input,&endpoint,&event,read_clipboard,"sticky.example"));
#ifdef SDL3_STICKY_KEYS
    assert(!strcmp(endpoint.editor.bytes,"sticky.example"));
#else
    assert(!strcmp(endpoint.editor.bytes,"europe.tomenet.eu"));
#endif
    event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    assert(!sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(!input.latch);
    return 0;
}
