#include "input/native-birth.h"

static SvBirthInputResult apply(SvBirth *birth, uint64_t generation,
                                SvBirthCommand command, bool keyboard, bool mouse)
{
    SvBirthInputResult result = sv_birth_command(birth, generation, command);
    if (result != SV_BIRTH_INPUT_IGNORED) {
        birth->keyboard_navigated |= keyboard;
        birth->mouse_navigated |= mouse;
    }
    return result;
}

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                          const SDL_Event *event)
{
    if (!birth || !event) return SV_BIRTH_INPUT_IGNORED;
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event->button.button == SDL_BUTTON_LEFT) {
        SvBirthSex sex;
        if (event->button.y >= 300.0f && event->button.y < 350.0f)
            sex = SV_BIRTH_SEX_MALE;
        else if (event->button.y >= 350.0f && event->button.y < 400.0f)
            sex = SV_BIRTH_SEX_FEMALE;
        else return SV_BIRTH_INPUT_IGNORED;
        SvBirthInputResult selected = apply(birth, generation,
            (SvBirthCommand){SV_BIRTH_SELECT_SEX, sex}, false, true);
        if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
            return selected;
        return apply(birth, generation,
                     (SvBirthCommand){SV_BIRTH_CONFIRM, SV_BIRTH_SEX_NONE},
                     false, true);
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SV_BIRTH_INPUT_IGNORED;
    SDL_Keycode key = event->key.key;
    SvBirthCommand command;
    if (key == SDLK_UP || key == SDLK_LEFT || key == SDLK_KP_8 || key == SDLK_KP_4)
        command = (SvBirthCommand){SV_BIRTH_MOVE_PREVIOUS, SV_BIRTH_SEX_NONE};
    else if (key == SDLK_DOWN || key == SDLK_RIGHT || key == SDLK_KP_2 || key == SDLK_KP_6)
        command = (SvBirthCommand){SV_BIRTH_MOVE_NEXT, SV_BIRTH_SEX_NONE};
    else if (key == SDLK_M)
        command = (SvBirthCommand){SV_BIRTH_SELECT_SEX, SV_BIRTH_SEX_MALE};
    else if (key == SDLK_F)
        command = (SvBirthCommand){SV_BIRTH_SELECT_SEX, SV_BIRTH_SEX_FEMALE};
    else if (key == SDLK_ASTERISK || key == SDLK_KP_MULTIPLY)
        command = (SvBirthCommand){SV_BIRTH_RANDOM, SV_BIRTH_SEX_NONE};
    else if (key == SDLK_HASH)
        command = (SvBirthCommand){SV_BIRTH_DNA, SV_BIRTH_SEX_NONE};
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        command = (SvBirthCommand){SV_BIRTH_CONFIRM, SV_BIRTH_SEX_NONE};
    else if (key == SDLK_Q || ((event->key.mod & SDL_KMOD_CTRL) && key == SDLK_Q))
        command = (SvBirthCommand){SV_BIRTH_QUIT, SV_BIRTH_SEX_NONE};
    else {
        birth->illegal_ignored = true;
        return SV_BIRTH_INPUT_IGNORED;
    }
    return apply(birth, generation, command, true, false);
}
