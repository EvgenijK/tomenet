#ifndef SV_INPUT_NATIVE_LOGIN_H
#define SV_INPUT_NATIVE_LOGIN_H
#include "login/characters/login-interaction.h"
#include <SDL3/SDL.h>
#include "login/ui/login-layout.h"

typedef struct { size_t selected, first, rows; float list_height; int focus; } SvCharacterViewport;
void sv_character_viewport_sync(SvCharacterViewport *v, SvCharacterLayout c, size_t count, size_t total);
bool sv_native_character_command(const SDL_Event *event, SvCharacterLayout c,
    SvCharacterViewport *v, size_t count, size_t total, SvLoginCommand *command);

bool sv_native_login_command(const SDL_Event *event, SvLoginCommand *command);
void sv_native_login_clear_previous_keys(SvLoginTransition transition);
#endif
