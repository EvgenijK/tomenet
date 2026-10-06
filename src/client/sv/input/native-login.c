#include "input/native-login.h"

bool sv_native_login_command(const SDL_Event *event, SvLoginCommand *command)
{
    if (!event || !command || event->type != SDL_EVENT_KEY_DOWN ||
        event->key.repeat || !event->key.key) return false;
    SDL_Keycode key = event->key.key;
    if (key == SDLK_Q) *command = (SvLoginCommand){SV_LOGIN_COMMAND_QUIT, 0};
    else if (key >= SDLK_A && key <= SDLK_Z)
        *command = (SvLoginCommand){SV_LOGIN_COMMAND_CHOOSE,
                                    (size_t)(key - SDLK_A)};
    else *command = (SvLoginCommand){SV_LOGIN_COMMAND_ACK_MOTD, 0};
    return true;
}

void sv_native_login_clear_previous_keys(SvLoginTransition transition)
{
    if (transition != SV_LOGIN_TRANSITION_NONE)
        SDL_FlushEvents(SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
}
