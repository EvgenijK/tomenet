#include "input/login-interaction.h"
#include <ctype.h>
#include <string.h>

void sv_login_interaction_begin(SvLoginInteraction *input, SvLogin *login,
                                const char *default_character, bool skip_motd)
{
    *input = (SvLoginInteraction){.login = login, .default_character = default_character,
                                   .skip_motd = skip_motd};
}

SvLoginTransition sv_login_interaction_sync(SvLoginInteraction *input)
{
    SvLoginState state = sv_login_state(input->login);
    if (state == SV_LOGIN_OVERVIEW && !input->overview_seen) {
        input->overview_seen = true;
        if (input->default_character) {
            char desired[SV_LOGIN_NAME_CAPACITY];
            size_t length = strlen(input->default_character);
            if (length >= sizeof(desired)) return SV_LOGIN_TRANSITION_OVERVIEW;
            memcpy(desired, input->default_character, length + 1);
            desired[0] = (char)toupper((unsigned char)desired[0]);
            for (size_t slot = 0; slot < sv_login_count(input->login); ++slot)
                if (!strcmp(desired, sv_login_character(input->login, slot)->name)) {
                    (void)sv_login_choose(input->login, slot);
                    break;
                }
        }
        return SV_LOGIN_TRANSITION_OVERVIEW;
    }
    if (state == SV_LOGIN_SELECTED && !input->selected_seen) {
        input->selected_seen = true;
        input->motd_complete = input->skip_motd;
        return SV_LOGIN_TRANSITION_MOTD;
    }
    return SV_LOGIN_TRANSITION_NONE;
}

SvLoginInputResult sv_login_interaction_command(SvLoginInteraction *input,
                                                 SvLoginCommand command)
{
    SvLoginState state = sv_login_state(input->login);
    if (state == SV_LOGIN_OVERVIEW) {
        if (command.kind == SV_LOGIN_COMMAND_QUIT) return SV_LOGIN_INPUT_QUIT;
        if (command.kind == SV_LOGIN_COMMAND_CHOOSE &&
            sv_login_choose(input->login, command.slot) == SV_OK)
            return SV_LOGIN_INPUT_HANDLED;
    } else if (state == SV_LOGIN_SELECTED && !input->motd_complete) {
        input->motd_complete = true;
        return SV_LOGIN_INPUT_HANDLED;
    }
    return SV_LOGIN_INPUT_IGNORED;
}

bool sv_login_interaction_complete(const SvLoginInteraction *input)
{
    return input->motd_complete && sv_login_state(input->login) == SV_LOGIN_SELECTED;
}
