#ifndef SV_INPUT_NATIVE_LOGIN_H
#define SV_INPUT_NATIVE_LOGIN_H
#include "input/login-interaction.h"
#include <SDL3/SDL.h>

bool sv_native_login_command(const SDL_Event *event, SvLoginCommand *command);
void sv_native_login_clear_previous_keys(SvLoginTransition transition);
#endif
