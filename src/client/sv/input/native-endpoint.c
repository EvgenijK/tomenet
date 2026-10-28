#include "input/native-endpoint.h"
#include "input/physical.h"
#include "ui/login-layout.h"
#include <string.h>

static bool modifier(SDL_Scancode code)
{
    return code == SDL_SCANCODE_LCTRL || code == SDL_SCANCODE_RCTRL ||
           code == SDL_SCANCODE_LSHIFT || code == SDL_SCANCODE_RSHIFT ||
           code == SDL_SCANCODE_LALT || code == SDL_SCANCODE_RALT;
}

void sv_endpoint_input_begin(SvEndpointInput *input)
{
    *input = (SvEndpointInput){.since_ns = SDL_GetTicksNS(), .hover = SIZE_MAX};
}

SvTextResult sv_contact_field_insert(SvTextField *field, const char *utf8)
{
    if (!field || !utf8) return SV_TEXT_INVALID;
    for (const unsigned char *byte = (const unsigned char *)utf8; *byte; ++byte)
        if (*byte < 0x20 || *byte > 0x7e) return SV_TEXT_ENCODING_ERROR;
    return sv_text_insert_utf8(field, utf8);
}

size_t sv_contact_draft_limit(bool password)
{
    (void)password;
    return 15;
}


static void move_row(SvEndpoint *endpoint, int direction)
{
    if (!endpoint->server_count) return;
    if (endpoint->selected >= endpoint->server_count) endpoint->selected = 0;
    else if (direction < 0 && endpoint->selected) --endpoint->selected;
    else if (direction > 0 && endpoint->selected + 1 < endpoint->server_count) ++endpoint->selected;
}

bool sv_endpoint_event(SvEndpointInput *input, SvEndpoint *endpoint,
                       const SDL_Event *event, SvClipboardRead clipboard, void *context)
{
    if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        input->latch = 0;
        return false;
    }
    if (event->common.timestamp && event->common.timestamp < input->since_ns) return true;
    if (event->type == SDL_EVENT_MOUSE_MOTION && endpoint->phase == SV_ENDPOINT_LIST) {
        SDL_Window *window=SDL_GetWindowFromID(event->motion.windowID);int w,h;
        float scale=window?SDL_GetWindowDisplayScale(window):0,density=window?SDL_GetWindowPixelDensity(window):0;
        if(scale>0 && density>0 && SDL_GetWindowSizeInPixels(window,&w,&h)) {
            SvLoginLayout l=sv_login_layout(w/scale,h/scale,endpoint->server_count,endpoint->selected);
            float x=event->motion.x*density/scale,y=event->motion.y*density/scale;
            input->hover=SIZE_MAX;input->hover_manual=sv_login_contains(l.manual,x,y);
            for(size_t i=l.first;i<endpoint->server_count && i<l.first+l.rows;++i)
                if(sv_login_contains(sv_login_row(l,i),x,y))input->hover=i;
        }
        return true;
    }
    if(event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button==SDL_BUTTON_LEFT && endpoint->phase==SV_ENDPOINT_MANUAL) {
        SDL_Window *window=SDL_GetWindowFromID(event->button.windowID);int w,h;
        float scale=window?SDL_GetWindowDisplayScale(window):0,density=window?SDL_GetWindowPixelDensity(window):0;
        if(scale>0 && density>0 && SDL_GetWindowSizeInPixels(window,&w,&h)) {
            SvLoginLayout l=sv_login_layout(w/scale,h/scale,endpoint->server_count,endpoint->selected);
            SvLoginFormLayout f=sv_login_form_layout(l,h/scale);
            float x=event->button.x*density/scale,y=event->button.y*density/scale;
            if(sv_login_contains(f.cancel,x,y))sv_endpoint_key(endpoint,SV_EDIT_CANCEL);
            else if(sv_login_contains(f.connect,x,y))sv_endpoint_key(endpoint,SV_EDIT_ACCEPT);
            else if(sv_login_contains(f.address,x,y)) {
                float cell=input->text_cell>0?input->text_cell:11;
                size_t visible=(size_t)((f.address.w-22)/cell);
                size_t start=endpoint->editor.cursor>=visible?endpoint->editor.cursor-visible+1:0;
                size_t cursor=start+(x>f.address.x+10?(size_t)((x-f.address.x-10)/cell+.5f):0);
                if(cursor>endpoint->editor.length)cursor=endpoint->editor.length;
                size_t anchor=(SDL_GetModState()&SDL_KMOD_SHIFT)?endpoint->editor.anchor:cursor;
                endpoint->editor.anchor=anchor;endpoint->editor.cursor=cursor;input->manual_focus=0;
            }
        }
        return true;
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event->button.button == SDL_BUTTON_LEFT && endpoint->phase == SV_ENDPOINT_LIST) {
        SDL_Window *window = SDL_GetWindowFromID(event->button.windowID);
        int width, height;
        if (!window || !SDL_GetWindowSizeInPixels(window, &width, &height)) return false;
        float scale = SDL_GetWindowDisplayScale(window);
        float density = SDL_GetWindowPixelDensity(window);
        if (scale <= 0 || density <= 0) return false;
        SvLoginLayout layout = sv_login_layout(width/scale,height/scale,
                                               endpoint->server_count,endpoint->selected);
        float x=event->button.x*density/scale, y=event->button.y*density/scale;
        if (sv_login_contains(layout.manual,x,y)) {input->manual_focus=0;sv_endpoint_manual(endpoint); }
        else for (size_t i=layout.first;i<endpoint->server_count && i<layout.first+layout.rows;++i)
            if (sv_login_contains(sv_login_row(layout,i),x,y)) {
                (void)sv_endpoint_choose(endpoint,i); break;
            }
        return true;
    }
    if (event->type != SDL_EVENT_KEY_DOWN && event->type != SDL_EVENT_TEXT_INPUT) return false;
    if (endpoint->phase == SV_ENDPOINT_SELECTED || endpoint->phase == SV_ENDPOINT_CANCELLED) return true;
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        if (endpoint->phase == SV_ENDPOINT_LIST) {
            if (!strcmp(event->text.text, "q") || !strcmp(event->text.text, "Q")) {input->manual_focus=0;sv_endpoint_manual(endpoint); }
            else if (strlen(event->text.text) == 1 && event->text.text[0] >= 'a' &&
                     (size_t)(event->text.text[0] - 'a') < endpoint->server_count)
                (void)sv_endpoint_choose(endpoint, (size_t)(event->text.text[0] - 'a'));
        } else if (!input->manual_focus) {
            input->text_error = sv_text_insert_utf8(&endpoint->editor, event->text.text);
            input->clipboard_unavailable = false;
        }
        return true;
    }
    if (event->key.repeat) return true;
    SDL_Scancode scancode = event->key.scancode;
#ifdef SDL3_STICKY_KEYS
    if (modifier(scancode)) {
        if (scancode == SDL_SCANCODE_LCTRL || scancode == SDL_SCANCODE_RCTRL) input->latch |= SDL_KMOD_CTRL;
        if (scancode == SDL_SCANCODE_LSHIFT || scancode == SDL_SCANCODE_RSHIFT) input->latch |= SDL_KMOD_SHIFT;
        if (scancode == SDL_SCANCODE_LALT || scancode == SDL_SCANCODE_RALT) input->latch |= SDL_KMOD_ALT;
        return true;
    }
#else
    if (modifier(scancode)) return true;
#endif
    SvPhysicalKey physical;
    if (!sv_physical_key(&event->key,input->latch,&physical)) return true;
    int symbol = sv_physical_symbol(physical.bytes, physical.size);
    SDL_Keymod mods = physical.modifiers;
    input->latch = 0;
    SDL_Keycode key = physical.key;
    if (endpoint->phase == SV_ENDPOINT_LIST) {
        if(key==SDLK_TAB) { input->list_focus_manual=!input->list_focus_manual;return true; }
        if (key == SDLK_ESCAPE) sv_endpoint_key(endpoint, SV_EDIT_CANCEL);
        else if (symbol == 0xff52 || symbol == 0xff97) move_row(endpoint, -1);
        else if (symbol == 0xff54 || symbol == 0xff99) move_row(endpoint, 1);
        else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            if(input->list_focus_manual) {input->manual_focus=0;sv_endpoint_manual(endpoint); }
            else if (endpoint->selected < endpoint->server_count)
                (void)sv_endpoint_choose(endpoint, endpoint->selected);
        }
        return true;
    }
    if(key==SDLK_TAB) {
        input->manual_focus=(input->manual_focus+((mods&SDL_KMOD_SHIFT)?2:1))%3;
        return true;
    }
    if((key==SDLK_RETURN || key==SDLK_KP_ENTER || key==SDLK_SPACE) && input->manual_focus) {
        sv_endpoint_key(endpoint,input->manual_focus==1?SV_EDIT_CANCEL:SV_EDIT_ACCEPT);return true;
    }
    SvEditKey action;
    bool handled = true;
    if (key == SDLK_ESCAPE) action = SV_EDIT_CANCEL;
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) action = SV_EDIT_ACCEPT;
    else if (key == SDLK_BACKSPACE) action = (mods & SDL_KMOD_CTRL) ? SV_EDIT_WORD_DELETE : SV_EDIT_BACKSPACE;
    else if (key == SDLK_DELETE) action = SV_EDIT_DELETE;
    else if (symbol == 0xff51 || symbol == 0xff96) action = (mods & SDL_KMOD_CTRL) ? SV_EDIT_WORD_LEFT : SV_EDIT_LEFT;
    else if (symbol == 0xff53 || symbol == 0xff98) action = (mods & SDL_KMOD_CTRL) ? SV_EDIT_WORD_RIGHT : SV_EDIT_RIGHT;
    else if (symbol == 0xff50 || symbol == 0xff95) action = SV_EDIT_HOME;
    else if (symbol == 0xff57 || symbol == 0xff9c) action = SV_EDIT_END;
    else if (symbol == 0xff52 || symbol == 0xff97) action = SV_EDIT_HISTORY_BACK;
    else if (symbol == 0xff54 || symbol == 0xff99) action = SV_EDIT_HISTORY_FORWARD;
    else if (mods & SDL_KMOD_CTRL) {
        switch (key) {
        case SDLK_A: action = SV_EDIT_LEFT; break;
        case SDLK_S: action = SV_EDIT_RIGHT; break;
        case SDLK_Q: action = SV_EDIT_WORD_LEFT; break;
        case SDLK_W: action = SV_EDIT_WORD_RIGHT; break;
        case SDLK_V: action = SV_EDIT_HOME; break;
        case SDLK_B: action = SV_EDIT_END; break;
        case SDLK_E: action = SV_EDIT_WORD_DELETE; break;
        case SDLK_P: action = SV_EDIT_HISTORY_BACK; break;
        case SDLK_N: action = SV_EDIT_HISTORY_FORWARD; break;
        case SDLK_C: action = SV_EDIT_SEARCH; break;
        case SDLK_L: {
            char *value = clipboard ? clipboard(context) : SDL_GetClipboardText();
            if (!value) input->clipboard_unavailable = true;
            else {
                input->clipboard_unavailable = false;
                input->text_error = sv_text_paste(&endpoint->editor, value, strlen(value));
                SDL_free(value);
            }
            return true;
        }
        default: handled = false;
        }
    } else handled = false;
    if (handled) {
        bool selecting=(mods&SDL_KMOD_SHIFT) &&
            (action==SV_EDIT_LEFT || action==SV_EDIT_RIGHT || action==SV_EDIT_HOME ||
             action==SV_EDIT_END || action==SV_EDIT_WORD_LEFT || action==SV_EDIT_WORD_RIGHT);
        size_t anchor=endpoint->editor.anchor;
        if(selecting)endpoint->editor.anchor=endpoint->editor.cursor;
        sv_endpoint_key(endpoint, action);
        if(selecting)endpoint->editor.anchor=anchor;
    }
    return true;
}
