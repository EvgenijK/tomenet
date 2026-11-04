#include "login/ui/native-endpoint.h"
#include "login/ui/login-layout.h"
#include <assert.h>
#include <string.h>

static unsigned opened_links;
static char opened_url[128];
/* Keep the OS browser outside this production input-path regression. */
bool SDLCALL SDL_OpenURL(const char *url)
{
    ++opened_links;
    SDL_strlcpy(opened_url,url,sizeof(opened_url));
    return true;
}

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
    assert(sv_contact_draft_limit(false) == 15);
    assert(sv_contact_draft_limit(true) == 15);
    sv_text_begin(&contact_field, "pw", sv_contact_draft_limit(true), true);
    assert(sv_text_select(&contact_field, contact_field.length, contact_field.length));
    assert(sv_contact_field_insert(&contact_field, "\xc3\xa9") == SV_TEXT_ENCODING_ERROR);
    assert(contact_field.length == 2 && !strcmp(contact_field.bytes, "pw"));
    assert(sv_contact_field_insert(&contact_field, "A") == SV_TEXT_OK);
    assert(contact_field.length == 3 && !strcmp(contact_field.bytes, "pwA"));
    SvTextHistory history = {0};
    sv_text_begin_history(&contact_field, &history, "", sv_contact_draft_limit(true), true);
    assert(sv_contact_field_insert(&contact_field, "1234567890123456") == SV_TEXT_TRUNCATED);
    assert(contact_field.length == 15);
    sv_text_remember(&contact_field);
    assert(history.count == 0);
    sv_text_begin(&contact_field, "", sv_contact_draft_limit(false), true);
    char long_account[81];
    memset(long_account, 'a', 80);
    long_account[80] = 0;
    assert(sv_contact_field_insert(&contact_field, long_account) == SV_TEXT_TRUNCATED);
    assert(contact_field.length == 15);

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
    assert(endpoint.phase == SV_ENDPOINT_LIST && !endpoint.host[0] && !endpoint.port);
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
    /* The same production rectangles drive the native mouse adapter. */
    assert(SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy"));
    assert(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window=SDL_CreateWindow("endpoint geometry",1024,768,0);
    assert(window);
    SvServer server={.host="mouse.example",.port=18348,.ping_ms=-1};
    sv_endpoint_begin(&endpoint,18348);
    assert(sv_endpoint_servers(&endpoint,&server,1));
    sv_endpoint_input_begin(&input);
    input.since_ns=100;
    SvLoginLayout layout=sv_login_server_layout(1024,768,&endpoint);
    SvLoginRect row=sv_login_row(layout,0);
    event=(SDL_Event){.type=SDL_EVENT_MOUSE_BUTTON_DOWN};
    event.button.button=SDL_BUTTON_LEFT;
    event.button.windowID=SDL_GetWindowID(window);
    event.button.x=row.x+4;event.button.y=row.y+4;
    event.button.timestamp=99;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_LIST);
    event.button.timestamp=101;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_LIST && endpoint.selected==0);
    event.button.clicks=2;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_SELECTED && !strcmp(endpoint.host,"mouse.example"));
    sv_endpoint_begin(&endpoint,18348);
    assert(sv_endpoint_servers(&endpoint,&server,1));
    event.button.clicks=1;
    SvLoginRect door=sv_login_server_door(row);
    event.button.x=door.x+4;event.button.y=door.y+4;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_SELECTED && !strcmp(endpoint.host,"mouse.example"));
    sv_endpoint_begin(&endpoint,18348);
    assert(sv_endpoint_servers(&endpoint,&server,1));
    event.button.x=layout.links.x+20;event.button.y=layout.links.y+20;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(opened_links==1 && !strcmp(opened_url,"https://www.tomenet.eu/"));
    assert(endpoint.phase==SV_ENDPOINT_LIST);
    assert(SDL_SetWindowSize(window,1600,1000));
    SvLoginLayout wide=sv_login_server_layout(1600,1000,&endpoint);
    static const char *urls[]={"https://www.tomenet.eu/","https://discord.gg/rASkhCg",
        "https://web.libera.chat/?channels=%23tomenet","https://www.tomenet.eu/guide.php?",
        "https://igroglaz.com/tomenet","http://github.com/TomenetGame/"};
    for(size_t i=0;i<6;++i) {
        SvLoginRect hit=sv_login_link_hit(wide,i);
        for(int label=0;label<2;++label) {
            event.button.x=hit.x+(label?70:20);event.button.y=hit.y+20;
            unsigned before=opened_links;
            assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
            assert(opened_links==before+1 && !strcmp(opened_url,urls[i]));
            assert(endpoint.phase==SV_ENDPOINT_LIST);
        }
        SDL_Event motion={.type=SDL_EVENT_MOUSE_MOTION};
        motion.motion.windowID=SDL_GetWindowID(window);
        motion.motion.x=hit.x+20;motion.motion.y=hit.y+20;
        assert(sv_endpoint_event(&input,&endpoint,&motion,NULL,NULL));
        assert(input.hover_link==i);
        motion.motion.x=0;motion.motion.y=0;
        assert(sv_endpoint_event(&input,&endpoint,&motion,NULL,NULL));
        assert(input.hover_link==SIZE_MAX);
    }
    assert(SDL_SetWindowSize(window,1024,768));
    event.button.x=layout.manual.x+4;event.button.y=layout.manual.y+4;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_MANUAL);
    unsigned before=opened_links;
    event.button.x=layout.links.x+20;event.button.y=layout.links.y+20;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(opened_links==before && endpoint.phase==SV_ENDPOINT_MANUAL);
    assert(input.manual_focus==0);
    SvLoginFormLayout form=sv_login_form_layout(layout,768);
    input.text_cell=10;
    endpoint.editor.cursor=endpoint.editor.anchor=8;
    SDL_SetModState(SDL_KMOD_SHIFT);
    event.button.x=form.address.x+10+2*input.text_cell;
    event.button.y=form.address.y+10;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.editor.cursor==2 && endpoint.editor.anchor==8);
    SDL_SetModState(SDL_KMOD_NONE);
    event.button.x=form.connect.x+4;event.button.y=form.connect.y+4;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_SELECTED && !strcmp(endpoint.host,"europe.tomenet.eu"));
    sv_endpoint_begin(&endpoint,18348);
    input.manual_focus=2;
    event=text_event("q");
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_MANUAL && input.manual_focus==0);
    event=(SDL_Event){.type=SDL_EVENT_MOUSE_BUTTON_DOWN};
    event.button.button=SDL_BUTTON_LEFT;
    event.button.windowID=SDL_GetWindowID(window);
    event.button.x=form.cancel.x+4;event.button.y=form.cancel.y+4;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_LIST && !endpoint.host[0] && !endpoint.port);
    /* Holding navigation repeats selection, but never repeats activation. */
    SvServer navigation_rows[]={
        {.host="one.example",.port=18348},
        {.host="two.example",.port=18348},
        {.host="three.example",.port=18348}};
    sv_endpoint_begin(&endpoint,18348);
    assert(sv_endpoint_servers(&endpoint,navigation_rows,3));endpoint.selected=0;
    sv_endpoint_input_begin(&input);
    event=key_event(SDLK_UNKNOWN,0);event.key.scancode=SDL_SCANCODE_DOWN;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));assert(endpoint.selected==1);
    event.key.scancode=SDL_SCANCODE_UP;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));assert(endpoint.selected==0);
    event=key_event(SDLK_DOWN,0);event.key.scancode=SDL_SCANCODE_DOWN;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));assert(endpoint.selected==1);
    event.key.repeat=true;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));assert(endpoint.selected==2);
    event=key_event(SDLK_UP,0);event.key.scancode=SDL_SCANCODE_UP;event.key.repeat=true;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));assert(endpoint.selected==1);
    event=key_event(SDLK_RETURN,0);event.key.repeat=true;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));assert(endpoint.phase==SV_ENDPOINT_LIST);
    /* Freshly queued keys can carry an older native clock than SDL ticks.
     * Queue ownership, rather than that clock, separates list and editor. */
    input.since_ns=SDL_GetTicksNS()+1000000;
    event=key_event(SDLK_DOWN,0);event.key.scancode=SDL_SCANCODE_DOWN;
    event.common.timestamp=1;
    assert(SDL_PushEvent(&event));
    assert(SDL_PeepEvents(&event,1,SDL_GETEVENT,SDL_EVENT_KEY_DOWN,SDL_EVENT_KEY_DOWN)==1);
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.selected==2);
    SDL_Event queued=key_event(SDLK_RETURN,0);
    assert(SDL_PushEvent(&queued));
    queued=text_event("opening text");assert(SDL_PushEvent(&queued));
    queued=(SDL_Event){.type=SDL_EVENT_USER};assert(SDL_PushEvent(&queued));
    event=text_event("q");event.common.timestamp=1;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(endpoint.phase==SV_ENDPOINT_MANUAL);
    assert(!SDL_HasEvent(SDL_EVENT_KEY_DOWN) && !SDL_HasEvent(SDL_EVENT_TEXT_INPUT));
    assert(SDL_HasEvent(SDL_EVENT_USER));
    event=text_event("Test");event.common.timestamp=1;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    event=key_event(SDLK_BACKSPACE,0);event.common.timestamp=1;
    assert(sv_endpoint_event(&input,&endpoint,&event,NULL,NULL));
    assert(!strcmp(endpoint.editor.bytes,"Tes"));
    SDL_DestroyWindow(window);SDL_Quit();
    return 0;
}
