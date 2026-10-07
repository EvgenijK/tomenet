#include "input/native-name.h"
#include "input/native-login.h"
#include "input/physical.h"

SvLoginInputResult sv_native_name_event(SvLoginInteraction *input,
                                        const SDL_Event *event,
                                        SvTextResult *text_result,
                                        bool *clipboard_unavailable)
{
    if (!input || !event) return SV_LOGIN_INPUT_IGNORED;
    if (!input->ordinary_name_active) {
        SvLoginCommand command;
        return sv_native_login_command(event, &command) ?
            sv_login_interaction_command(input, command) : SV_LOGIN_INPUT_IGNORED;
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event->button.button == SDL_BUTTON_LEFT && event->button.x >= 40.0f &&
        event->button.y >= 670.0f && event->button.y <= 740.0f) {
        if (event->button.clicks >= 2)
            (void)sv_text_select(&input->character_name, 0,
                                 input->character_name.length);
        else {
            size_t position = event->button.x < 220.0f ? 0 :
                input->character_name.length;
            (void)sv_text_select(&input->character_name, position, position);
        }
        input->name_mouse_selected = true;
        return SV_LOGIN_INPUT_HANDLED;
    }
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        for (const unsigned char *byte = (const unsigned char *)event->text.text;
             byte && *byte; ++byte)
            if (*byte >= 0x80) input->name_input_method_used = true;
        if (text_result)
            *text_result = sv_login_interaction_name_insert(input, event->text.text);
        return SV_LOGIN_INPUT_HANDLED;
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SV_LOGIN_INPUT_IGNORED;
    if (event->key.key == SDLK_ESCAPE)
        return sv_login_interaction_name_key(input, SV_EDIT_CANCEL);
    if (event->key.key == SDLK_RETURN || event->key.key == SDLK_KP_ENTER)
        return sv_login_interaction_name_key(input, SV_EDIT_ACCEPT);
    SvPhysicalKey physical;
    if (!sv_physical_key(&event->key, 0, &physical)) return SV_LOGIN_INPUT_HANDLED;
    int symbol = sv_physical_symbol(physical.bytes, physical.size);
    SDL_Keycode key = physical.key;
    SDL_Keymod mods = physical.modifiers;
    if ((mods & SDL_KMOD_CTRL) && key == SDLK_L) {
        char *value = SDL_GetClipboardText();
        if (!value) {
            if (clipboard_unavailable) *clipboard_unavailable = true;
        } else {
            if (clipboard_unavailable) *clipboard_unavailable = false;
            if (text_result)
                *text_result = sv_login_interaction_name_insert(input, value);
            SDL_free(value);
            input->name_paste_used = true;
        }
        return SV_LOGIN_INPUT_HANDLED;
    }
    SvEditKey edit;
    if (key == SDLK_BACKSPACE)
        edit = (mods & SDL_KMOD_CTRL) ? SV_EDIT_WORD_DELETE : SV_EDIT_BACKSPACE;
    else if (key == SDLK_DELETE) edit = SV_EDIT_DELETE;
    else if (symbol == 0xff51 || symbol == 0xff96)
        edit = (mods & SDL_KMOD_CTRL) ? SV_EDIT_WORD_LEFT : SV_EDIT_LEFT;
    else if (symbol == 0xff53 || symbol == 0xff98)
        edit = (mods & SDL_KMOD_CTRL) ? SV_EDIT_WORD_RIGHT : SV_EDIT_RIGHT;
    else if (symbol == 0xff50 || symbol == 0xff95) edit = SV_EDIT_HOME;
    else if (symbol == 0xff57 || symbol == 0xff9c) edit = SV_EDIT_END;
    else return SV_LOGIN_INPUT_HANDLED;
    if (edit != SV_EDIT_ACCEPT && edit != SV_EDIT_CANCEL)
        input->name_keyboard_edited = true;
    return sv_login_interaction_name_key(input, edit);
}
