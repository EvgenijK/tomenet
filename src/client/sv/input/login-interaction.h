#ifndef SV_INPUT_LOGIN_INTERACTION_H
#define SV_INPUT_LOGIN_INTERACTION_H
#include "input/text-field.h"
#include "protocol/login.h"

typedef enum { SV_LOGIN_INPUT_IGNORED, SV_LOGIN_INPUT_HANDLED,
               SV_LOGIN_INPUT_QUIT, SV_LOGIN_INPUT_NAME_STARTED,
               SV_LOGIN_INPUT_NAME_CANCELLED,
               SV_LOGIN_INPUT_NAME_SUBMITTED,
               SV_LOGIN_INPUT_NAME_SUGGESTED } SvLoginInputResult;
typedef enum { SV_LOGIN_COMMAND_QUIT, SV_LOGIN_COMMAND_CHOOSE,
               SV_LOGIN_COMMAND_ACK_MOTD, SV_LOGIN_COMMAND_CREATE_ORDINARY,
               SV_LOGIN_COMMAND_CANCEL } SvLoginCommandKind;
typedef struct { SvLoginCommandKind kind; size_t slot; } SvLoginCommand;
typedef enum { SV_LOGIN_TRANSITION_NONE, SV_LOGIN_TRANSITION_OVERVIEW,
               SV_LOGIN_TRANSITION_MOTD, SV_LOGIN_TRANSITION_BIRTH } SvLoginTransition;
typedef struct {
    SvLogin *login;
    const char *default_character;
    bool skip_motd, overview_seen, selected_seen, motd_complete;
    bool ordinary_name_active;
    bool name_input_truncated, name_input_method_used, name_keyboard_edited;
    bool name_mouse_selected, name_paste_used;
    size_t name_maximum_draft_bytes;
    SvTextField character_name;
} SvLoginInteraction;

void sv_login_interaction_begin(SvLoginInteraction *input, SvLogin *login,
                                const char *default_character, bool skip_motd);
SvLoginTransition sv_login_interaction_sync(SvLoginInteraction *input);
SvLoginInputResult sv_login_interaction_command(SvLoginInteraction *input,
                                                 SvLoginCommand command);
SvTextResult sv_login_interaction_name_insert(SvLoginInteraction *input,
                                               const char *utf8);
SvLoginInputResult sv_login_interaction_name_key(SvLoginInteraction *input,
                                                  SvEditKey key);
SvLoginInputResult sv_login_interaction_restore_name(SvLoginInteraction *input,
                                                      const char *draft);
bool sv_login_interaction_complete(const SvLoginInteraction *input);
bool sv_login_interaction_ordinary_available(const SvLoginInteraction *input);
#endif
