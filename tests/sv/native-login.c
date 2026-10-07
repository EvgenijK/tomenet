#include "input/native-login.h"
#include <assert.h>

int main(void)
{
    assert(SDL_Init(0));
    SDL_Event event = {0};
    SvLoginCommand command;
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_A;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CHOOSE && command.slot == 0);
    event.key.key = SDLK_N;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CHOOSE && command.slot == 13);
    event.key.mod = SDL_KMOD_SHIFT;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CREATE_ORDINARY);
    event.key.key = SDLK_E;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CREATE_EXCLUSIVE);
    event.key.mod = 0;
    event.key.key = SDLK_ESCAPE;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CANCEL);
    event.key.key = SDLK_Q;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_QUIT);
    event.key.key = SDLK_SPACE;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_ACK_MOTD);
    event.key.key = 0;
    assert(!sv_native_login_command(&event, &command));
    sv_native_login_clear_previous_keys(SV_LOGIN_TRANSITION_OVERVIEW);
    SDL_Quit();
    return 0;
}
