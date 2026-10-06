#ifndef SV_INPUT_CREDENTIALS_H
#define SV_INPUT_CREDENTIALS_H
#include "credential/vault.h"
#include "input/endpoint.h"
#include "input/text-field.h"
#include <SDL3/SDL.h>

typedef enum { SV_CREDENTIAL_EDITING, SV_CREDENTIAL_ACCEPTED,
               SV_CREDENTIAL_CANCELLED } SvCredentialAction;
typedef struct {
    SvEndpoint *endpoint;
    char *account, *password; /* Caller owns 80-byte buffers. */
    int stage;
    uint64_t generation;
    SvVaultRequest *lookup;
    bool restored_password, incompatible_password, clipboard_unavailable;
    SvTextResult text_error;
    const char *vault_status;
} SvCredentialInput;

void sv_credentials_begin(SvCredentialInput *input, SvEndpoint *endpoint,
                          char account[80], char password[80]);
void sv_credentials_poll(SvCredentialInput *input);
void sv_credentials_status(const SvCredentialInput *input, char out[180]);
SvCredentialAction sv_credentials_event(SvCredentialInput *input, const SDL_Event *event);
void sv_credentials_end(SvCredentialInput *input);
#endif
