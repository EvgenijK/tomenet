#ifndef SV_INPUT_LOGIN_INTERACTION_H
#define SV_INPUT_LOGIN_INTERACTION_H
#include "protocol/login.h"

typedef enum { SV_LOGIN_INPUT_IGNORED, SV_LOGIN_INPUT_HANDLED,
               SV_LOGIN_INPUT_QUIT } SvLoginInputResult;
typedef enum { SV_LOGIN_COMMAND_QUIT, SV_LOGIN_COMMAND_CHOOSE,
               SV_LOGIN_COMMAND_ACK_MOTD } SvLoginCommandKind;
typedef struct { SvLoginCommandKind kind; size_t slot; } SvLoginCommand;
typedef enum { SV_LOGIN_TRANSITION_NONE, SV_LOGIN_TRANSITION_OVERVIEW,
               SV_LOGIN_TRANSITION_MOTD } SvLoginTransition;
typedef struct {
    SvLogin *login;
    const char *default_character;
    bool skip_motd, overview_seen, selected_seen, motd_complete;
} SvLoginInteraction;

void sv_login_interaction_begin(SvLoginInteraction *input, SvLogin *login,
                                const char *default_character, bool skip_motd);
SvLoginTransition sv_login_interaction_sync(SvLoginInteraction *input);
SvLoginInputResult sv_login_interaction_command(SvLoginInteraction *input,
                                                 SvLoginCommand command);
bool sv_login_interaction_complete(const SvLoginInteraction *input);
#endif
