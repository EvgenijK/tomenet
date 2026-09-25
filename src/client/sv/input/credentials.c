#include "input/credentials.h"
#include "input/native-endpoint.h"
#include <string.h>

static void wipe(void *memory, size_t size)
{
    volatile unsigned char *bytes = memory;
    while (size--) *bytes++ = 0;
}

void sv_credentials_begin(SvCredentialInput *input, SvEndpoint *endpoint,
                          char account[80], char password[80])
{
    *input = (SvCredentialInput){.endpoint = endpoint, .account = account,
                                 .password = password, .generation = SDL_GetTicksNS()};
}

void sv_credentials_end(SvCredentialInput *input)
{
    sv_vault_cancel(&input->lookup);
}

static void manual_edit(SvCredentialInput *input)
{
    if (!input->lookup && !input->restored_password) return;
    sv_vault_cancel(&input->lookup);
    input->restored_password = false;
    input->vault_status = "Manual password is session-only in this build.";
}

void sv_credentials_poll(SvCredentialInput *input)
{
    if (!input->lookup) return;
    size_t count = 0;
    SvVaultResult result = sv_vault_poll(&input->lookup, input->generation,
                                         input->password, 15, &count);
    if (result == SV_VAULT_FOUND) {
        if (memchr(input->password, 0, count)) {
            wipe(input->password, 80);
            input->vault_status = "Stored password is invalid for contact; enter it manually for this session.";
        } else {
            input->password[count] = 0;
            input->restored_password = true;
            input->vault_status = "Stored password loaded. Enter connects; edit to correct it.";
        }
    } else if (result == SV_VAULT_MISSING)
        input->vault_status = "No saved password. Manual password is session-only in this build.";
    else if (result != SV_VAULT_PENDING)
        input->vault_status = "Vault unavailable or invalid; enter a password for this session only.";
}

void sv_credentials_status(const SvCredentialInput *input, char out[180])
{
    char mask[80];
    size_t length = strlen(input->password);
    memset(mask, '*', length);
    mask[length] = 0;
    if (input->incompatible_password) SDL_snprintf(out, 180,
        "Password contains '*' (unsupported by server). Edit it; Escape cancels.");
    else if (!input->stage) SDL_snprintf(out, 180,
        "Account: %s  (Enter continues, Escape cancels)", input->account);
    else if (input->lookup) SDL_snprintf(out, 180,
        "Looking up saved password... Escape cancels; typing uses manual entry.");
    else if (input->vault_status) SDL_snprintf(out, 180,
        "%s  Password: %s", input->vault_status, mask);
    else SDL_snprintf(out, 180,
        "Password: %s  (Enter connects, Escape cancels)", mask);
}

static void insert(SvCredentialInput *input, const char *source)
{
    if (input->stage) manual_edit(input);
    char *field = input->stage ? input->password : input->account;
    SvTextField editor;
    sv_text_begin(&editor, field, sv_contact_draft_limit(input->stage != 0), true);
    (void)sv_text_select(&editor, editor.length, editor.length);
    input->text_error = sv_contact_field_insert(&editor, source);
    memcpy(field, editor.bytes, editor.length + 1);
    input->incompatible_password = false;
    wipe(&editor, sizeof(editor));
}

SvCredentialAction sv_credentials_event(SvCredentialInput *input, const SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        return SV_CREDENTIAL_CANCELLED;
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        insert(input, event->text.text);
        return SV_CREDENTIAL_EDITING;
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SV_CREDENTIAL_EDITING;
    if (event->key.key == SDLK_ESCAPE) return SV_CREDENTIAL_CANCELLED;
    char *field = input->stage ? input->password : input->account;
    size_t length = strlen(field);
    if ((event->key.mod & SDL_KMOD_CTRL) &&
        (event->key.key == SDLK_V || event->key.key == SDLK_L)) {
        char *clipboard = SDL_GetClipboardText();
        if (!clipboard) input->clipboard_unavailable = true;
        else {
            insert(input, clipboard);
            wipe(clipboard, strlen(clipboard));
            SDL_free(clipboard);
            input->clipboard_unavailable = false;
        }
        return SV_CREDENTIAL_EDITING;
    }
    if (event->key.key == SDLK_BACKSPACE && length) {
        if (input->stage) manual_edit(input);
        field[length - 1] = 0;
        input->incompatible_password = false;
        input->text_error = SV_TEXT_OK;
    } else if ((event->key.key == SDLK_RETURN || event->key.key == SDLK_KP_ENTER) && length) {
        if (input->text_error == SV_TEXT_ENCODING_ERROR) return SV_CREDENTIAL_EDITING;
        if (input->stage) {
            if (input->endpoint->protocol >= 2 && strchr(input->password, '*')) {
                input->incompatible_password = true;
                return SV_CREDENTIAL_EDITING;
            }
            return SV_CREDENTIAL_ACCEPTED;
        }
        input->stage = 1;
        input->text_error = SV_TEXT_OK;
        char key[SV_VAULT_KEY_CAPACITY];
        if (sv_vault_key(key, input->endpoint->host, strlen(input->endpoint->host),
                         input->endpoint->port, input->account, strlen(input->account)))
            input->lookup = sv_vault_lookup(key, input->generation);
        if (!input->lookup)
            input->vault_status = "Vault unavailable; enter a password for this session only.";
    }
    return SV_CREDENTIAL_EDITING;
}
