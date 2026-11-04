#ifndef SV_INPUT_CREDENTIALS_H
#define SV_INPUT_CREDENTIALS_H
#include "credential/vault.h"
#include "login/servers/endpoint.h"
#include "input/text-field.h"
#include <SDL3/SDL.h>

typedef enum { SV_CREDENTIAL_EDITING, SV_CREDENTIAL_ACCEPTED,
               SV_CREDENTIAL_CANCELLED } SvCredentialAction;
typedef struct SvCredentialInput {
    SvEndpoint *endpoint;
    char *account, *password; /* Caller owns 80-byte buffers. */
    int stage; /* Active editor: account (0), password (1). */
    int focus; /* Account, password, reveal, submit, back. */
    size_t cursor[2], anchor[2];
    bool reveal, account_checked;
    uint64_t generation;
    SvVaultRequest *lookup;
    bool restored_password, incompatible_password, clipboard_unavailable;
    SvTextResult text_error;
    const char *vault_status;
} SvCredentialInput;

typedef struct {
    char account[80], password_mask[80];
    size_t cursor[2], anchor[2];
    int active_field, focus;
    bool reveal, lookup_pending, restored_password;
    bool incompatible_password, clipboard_unavailable;
    SvTextResult text_error;
    const char *vault_status;
} SvCredentialPresentation;

void sv_credentials_view(const SvCredentialInput *input, SvCredentialPresentation *view);
/* Borrowed only during synchronous rendering; never retain or cache the secret. */
const char *sv_credentials_private_password(const SvCredentialInput *input);
void sv_credentials_focus(SvCredentialInput *input, int focus);
void sv_credentials_select(SvCredentialInput *input, int field, size_t cursor, bool extend);
void sv_credentials_toggle_reveal(SvCredentialInput *input);
SvCredentialAction sv_credentials_submit(SvCredentialInput *input);
SvCredentialAction sv_credentials_cancel(SvCredentialInput *input);
void sv_credentials_begin(SvCredentialInput *input, SvEndpoint *endpoint,
                          char account[80], char password[80]);
void sv_credentials_poll(SvCredentialInput *input);
void sv_credentials_status(const SvCredentialInput *input, char out[180]);
SvCredentialAction sv_credentials_event(SvCredentialInput *input, const SDL_Event *event);
void sv_credentials_end(SvCredentialInput *input);
#endif
