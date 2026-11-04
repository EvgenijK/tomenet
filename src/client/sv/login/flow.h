#ifndef SV_LOGIN_FLOW_H
#define SV_LOGIN_FLOW_H
#include "login/options.h"
#include "login/login-view.h"
typedef struct SvLoginFlow SvLoginFlow;
/* All mutation is on the main thread. View borrows last until the next mutation. */
typedef struct {
    uint64_t generation, revision;
    SvPregamePhase phase;
    const char *status;
    bool failed, retry_available, text_input;
    bool has_login, has_birth, birth_pending;
    SvLoginView characters;
    SvBirthView birth;
} SvLoginFlowView;
typedef enum { SV_LOGIN_FLOW_EXIT, SV_LOGIN_FLOW_RETRY,
               SV_LOGIN_FLOW_BACK, SV_LOGIN_FLOW_EDIT_NAME } SvLoginFlowCommand;
typedef enum { SV_LOGIN_FLOW_FINISHED, SV_LOGIN_FLOW_SERVERS,
               SV_LOGIN_FLOW_CREDENTIALS, SV_LOGIN_FLOW_CONNECT } SvLoginFlowStep;
typedef enum { SV_LOGIN_CREDENTIAL_ACCEPTED, SV_LOGIN_CREDENTIAL_BACK,
               SV_LOGIN_CREDENTIAL_QUIT, SV_LOGIN_CREDENTIAL_ERROR } SvLoginCredentialResult;
/* Derived next work, not a second state machine. Selected follows endpoint
 * acceptance; credentials_result follows the native editor; next follows attempt
 * teardown. The native runner executes returned work without choosing transitions.
 * create copies account/password; other option strings live through destroy. */
SvLoginFlowStep sv_login_flow_selected(SvLoginFlow *flow);
SvLoginFlowStep sv_login_flow_credentials_result(SvLoginFlow *flow, SvLoginCredentialResult result);
SvLoginFlowStep sv_login_flow_next(SvLoginFlow *flow);
SvLoginFlow *sv_login_flow_create(SvEndpointOptions options);
void sv_login_flow_destroy(SvLoginFlow *flow);
/* Begins a generation only after the previous attempt has been torn down.
 * The module's native adapter tears down after exit/error/retry. Only the flow's
 * recovery draft survives retry; views expire on poll, command, or teardown.
 * READY remains the startup milestone: destroy still closes the connection;
 * ownership transfer requires a future live-runtime consumer and separate work. */
bool sv_login_flow_begin(SvLoginFlow *flow, const char *profile_root,
                          bool first_run, const char *font_resource);
void sv_login_flow_poll(SvLoginFlow *flow);
void sv_login_flow_command(SvLoginFlow *flow, SvLoginFlowCommand command);
void sv_login_flow_view(const SvLoginFlow *flow, SvLoginFlowView *view);
#endif
