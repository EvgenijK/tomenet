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
    input->cursor[0] = input->anchor[0] = strlen(account);
    input->cursor[1] = input->anchor[1] = strlen(password);
}

void sv_credentials_end(SvCredentialInput *input)
{
    sv_vault_cancel(&input->lookup);
    input->reveal = false;
}

static void manual_edit(SvCredentialInput *input)
{
    if (!input->lookup && !input->restored_password) return;
    sv_vault_cancel(&input->lookup);
    input->restored_password = false;
    input->vault_status = "Manual password entered; it will be saved after account confirmation.";
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
            input->cursor[1] = input->anchor[1] = count;
            input->vault_status = "Stored password loaded. Enter connects; edit to correct it.";
        }
    } else if (result == SV_VAULT_MISSING)
        input->vault_status = "No saved password. Enter one manually.";
    else if (result != SV_VAULT_PENDING)
        input->vault_status = "Vault unavailable or invalid; enter a password for this session.";
}

void sv_credentials_status(const SvCredentialInput *input, char out[180])
{
    char mask[80];
    size_t length = strlen(input->password);
    memset(mask, '*', length);
    mask[length] = 0;
    if (input->incompatible_password) SDL_snprintf(out, 180,
        "Password contains '*' (unsupported by server). Edit it; Escape returns to account.");
    else if (!input->stage) SDL_snprintf(out, 180,
        "Account: %s  (Enter continues, Escape cancels)", input->account);
    else if (input->lookup) SDL_snprintf(out, 180,
        "Looking up saved password... Escape returns to account; typing uses manual entry.");
    else if (input->vault_status) SDL_snprintf(out, 180,
        "%s  Password: %s", input->vault_status, mask);
    else SDL_snprintf(out, 180,
        "Password: %s  (Enter connects, Escape returns to account)", mask);
}

void sv_credentials_view(const SvCredentialInput *input, SvCredentialPresentation *view)
{
    *view = (SvCredentialPresentation){.active_field = input->stage, .focus = input->focus,
        .reveal = input->reveal, .lookup_pending = input->lookup != NULL,
        .restored_password = input->restored_password,
        .incompatible_password = input->incompatible_password,
        .clipboard_unavailable = input->clipboard_unavailable,
        .text_error = input->text_error, .vault_status = input->vault_status};
    SDL_strlcpy(view->account, input->account, sizeof(view->account));
    memset(view->password_mask, '*', strlen(input->password));
    memcpy(view->cursor, input->cursor, sizeof(view->cursor));
    memcpy(view->anchor, input->anchor, sizeof(view->anchor));
}

const char *sv_credentials_private_password(const SvCredentialInput *input)
{
    return input->reveal ? input->password : NULL;
}

static void lookup_password(SvCredentialInput *input)
{
    if (input->account_checked || !input->account[0]) return;
    input->account_checked = true;
    /* A manually supplied password must never be replaced by a late lookup. */
    if (input->password[0]) return;
    char key[SV_VAULT_KEY_CAPACITY];
    if (sv_vault_key(key, input->endpoint->host, strlen(input->endpoint->host),
                     input->endpoint->port, input->account, strlen(input->account)))
        input->lookup = sv_vault_lookup(key, input->generation);
    if (!input->lookup)
        input->vault_status = "Vault unavailable; enter a password for this session.";
}

void sv_credentials_focus(SvCredentialInput *input, int focus)
{
    if (focus < 0 || focus > 4) return;
    input->focus = focus;
    if (focus < 2) {
        input->stage = focus;
        if (focus == 1) lookup_password(input);
    }
}

void sv_credentials_select(SvCredentialInput *input, int field, size_t cursor, bool extend)
{
    if (field < 0 || field > 1) return;
    sv_credentials_focus(input, field);
    size_t length = strlen(field ? input->password : input->account);
    input->cursor[field] = cursor > length ? length : cursor;
    if (!extend) input->anchor[field] = input->cursor[field];
}

void sv_credentials_toggle_reveal(SvCredentialInput *input)
{
    input->reveal = !input->reveal;
}

SvCredentialAction sv_credentials_submit(SvCredentialInput *input)
{
    if (!input->account[0]) {
        sv_credentials_focus(input, 0);
        return SV_CREDENTIAL_EDITING;
    }
    if (!input->password[0]) {
        sv_credentials_focus(input, 1);
        return SV_CREDENTIAL_EDITING;
    }
    if (input->text_error == SV_TEXT_ENCODING_ERROR) return SV_CREDENTIAL_EDITING;
    if (input->endpoint->protocol >= 2 && strchr(input->password, '*')) {
        input->incompatible_password = true;
        sv_credentials_focus(input, 1);
        return SV_CREDENTIAL_EDITING;
    }
    input->reveal = false;
    return SV_CREDENTIAL_ACCEPTED;
}

static void account_edited(SvCredentialInput *input)
{
    sv_vault_cancel(&input->lookup);
    ++input->generation;
    input->account_checked = false;
    if (input->restored_password) {
        wipe(input->password, 80);
        input->cursor[1] = input->anchor[1] = 0;
    }
    input->restored_password = false;
    input->vault_status = NULL;
    input->reveal = false;
}

static void editor_begin(SvCredentialInput *input, SvTextField *editor)
{
    int field = input->stage;
    sv_text_begin(editor, field ? input->password : input->account,
                  sv_contact_draft_limit(field != 0), true);
    editor->cursor = input->cursor[field];
    editor->anchor = input->anchor[field];
}

static void editor_end(SvCredentialInput *input, SvTextField *editor)
{
    int field = input->stage;
    char *draft = field ? input->password : input->account;
    if (strcmp(draft, editor->bytes)) {
        if (field) manual_edit(input);
        else account_edited(input);
    }
    wipe(draft, 80);
    memcpy(draft, editor->bytes, editor->length + 1);
    input->cursor[field] = editor->cursor;
    input->anchor[field] = editor->anchor;
    wipe(editor, sizeof(*editor));
}

static void insert(SvCredentialInput *input, const char *source)
{
    if (input->stage) manual_edit(input);
    SvTextField editor;
    editor_begin(input, &editor);
    if (input->stage) input->text_error = sv_contact_field_insert(&editor, source);
    else {
        /* Baseline ASKFOR_LIVETRIM applies to the interactive account producer. */
        size_t length = strlen(source), count = 0;
        char trimmed[16];
        bool overflow = false, encodable = true;
        for (size_t i = 0; i < length; ++i) {
            unsigned char byte = (unsigned char)source[i];
            if (byte < 0x20 || byte > 0x7e) { encodable = false; break; }
            size_t start = editor.cursor < editor.anchor ? editor.cursor : editor.anchor;
            if (!start && !count) {
                if (byte >= 'a' && byte <= 'z') byte = (unsigned char)(byte - 'a' + 'A');
                if (byte < 'A' || byte > 'Z') continue;
            } else if (!((byte >= 'A' && byte <= 'Z') ||
                         (byte >= 'a' && byte <= 'z') ||
                         (byte >= '0' && byte <= '9') ||
                         strchr(" .,-'&_$%~#", byte))) byte = '_';
            if (count < sizeof(trimmed) - 1) trimmed[count++] = (char)byte;
            else overflow = true;
        }
        trimmed[count] = 0;
        input->text_error = !encodable ? SV_TEXT_ENCODING_ERROR :
            sv_contact_field_insert(&editor, trimmed);
        if (encodable && overflow) input->text_error = SV_TEXT_TRUNCATED;
        wipe(trimmed, sizeof(trimmed));
    }
    editor_end(input, &editor);
    input->incompatible_password = false;
}

SvCredentialAction sv_credentials_cancel(SvCredentialInput *input)
{
    sv_credentials_end(input);
    wipe(input->password,80);
    return SV_CREDENTIAL_CANCELLED;
}

static SvCredentialAction back(SvCredentialInput *input)
{
    input->reveal = false;
    if (input->stage) {
        sv_vault_cancel(&input->lookup);
        wipe(input->password, 80);
        input->cursor[1] = input->anchor[1] = 0;
        input->account_checked = false;
        input->restored_password = false;
        input->incompatible_password = false;
        input->vault_status = NULL;
        input->text_error = SV_TEXT_OK;
        sv_credentials_focus(input, 0);
        return SV_CREDENTIAL_EDITING;
    }
    return SV_CREDENTIAL_CANCELLED;
}

/* Fixed editor keys do not require a text-layout mapping. SDL can deliver
 * their scancode before a logical key is available on a newly opened form. */
static SDL_Keycode editor_key(const SDL_KeyboardEvent *event)
{
    if (event->key != SDLK_UNKNOWN) return event->key;
    switch (event->scancode) {
    case SDL_SCANCODE_BACKSPACE: return SDLK_BACKSPACE;
    case SDL_SCANCODE_DELETE: return SDLK_DELETE;
    case SDL_SCANCODE_LEFT: return SDLK_LEFT;
    case SDL_SCANCODE_RIGHT: return SDLK_RIGHT;
    case SDL_SCANCODE_HOME: return SDLK_HOME;
    case SDL_SCANCODE_END: return SDLK_END;
    case SDL_SCANCODE_TAB: return SDLK_TAB;
    case SDL_SCANCODE_RETURN: return SDLK_RETURN;
    case SDL_SCANCODE_KP_ENTER: return SDLK_KP_ENTER;
    case SDL_SCANCODE_ESCAPE: return SDLK_ESCAPE;
    case SDL_SCANCODE_SPACE: return SDLK_SPACE;
    default: break;
    }
    if (event->mod & SDL_KMOD_CTRL) switch (event->scancode) {
    case SDL_SCANCODE_A: return SDLK_A;
    case SDL_SCANCODE_V: return SDLK_V;
    case SDL_SCANCODE_L: return SDLK_L;
    default: break;
    }
    return SDLK_UNKNOWN;
}

SvCredentialAction sv_credentials_event(SvCredentialInput *input, const SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        input->reveal = false;
        return SV_CREDENTIAL_CANCELLED;
    }
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        if (input->focus < 2) insert(input, event->text.text);
        return SV_CREDENTIAL_EDITING;
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SV_CREDENTIAL_EDITING;
    SDL_Keycode key = editor_key(&event->key);
    bool ctrl = (event->key.mod & SDL_KMOD_CTRL) != 0;
    bool shift = (event->key.mod & SDL_KMOD_SHIFT) != 0;
    if (key == SDLK_ESCAPE) return back(input);
    if (key == SDLK_TAB || key == SDLK_LEFT_TAB || event->key.scancode == SDL_SCANCODE_TAB) {
        bool reverse=shift || key==SDLK_LEFT_TAB;
        sv_credentials_focus(input, (input->focus + (reverse ? 4 : 1)) % 5);
        return SV_CREDENTIAL_EDITING;
    }
    bool accept = key == SDLK_RETURN || key == SDLK_KP_ENTER;
    if (input->focus >= 2) {
        if (accept || key == SDLK_SPACE) {
            if (input->focus == 2) sv_credentials_toggle_reveal(input);
            else if (input->focus == 3) return sv_credentials_submit(input);
            else return sv_credentials_cancel(input);
        }
        return SV_CREDENTIAL_EDITING;
    }
    if (ctrl && (key == SDLK_V || key == SDLK_L)) {
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
    if (accept) {
        if (input->text_error == SV_TEXT_ENCODING_ERROR) return SV_CREDENTIAL_EDITING;
        if (input->stage) return sv_credentials_submit(input);
        if (input->account[0]) {
            input->text_error = SV_TEXT_OK;
            sv_credentials_focus(input, 1);
        }
        return SV_CREDENTIAL_EDITING;
    }
    SvTextField editor;
    editor_begin(input, &editor);
    size_t anchor = editor.anchor;
    if (ctrl && key == SDLK_A) { editor.anchor = 0; editor.cursor = editor.length; }
    else if (key == SDLK_LEFT) {
        if (shift) editor.anchor = editor.cursor;
        sv_text_key(&editor, ctrl ? SV_EDIT_WORD_LEFT : SV_EDIT_LEFT);
        if (shift) editor.anchor = anchor;
    } else if (key == SDLK_RIGHT) {
        if (shift) editor.anchor = editor.cursor;
        sv_text_key(&editor, ctrl ? SV_EDIT_WORD_RIGHT : SV_EDIT_RIGHT);
        if (shift) editor.anchor = anchor;
    } else if (key == SDLK_HOME || key == SDLK_END) {
        sv_text_key(&editor, key == SDLK_HOME ? SV_EDIT_HOME : SV_EDIT_END);
        if (shift) editor.anchor = anchor;
    } else {
        if (key == SDLK_BACKSPACE || key == SDLK_DELETE) {
            sv_text_key(&editor, key == SDLK_DELETE ? SV_EDIT_DELETE :
                        ctrl ? SV_EDIT_WORD_DELETE : SV_EDIT_BACKSPACE);
            input->incompatible_password = false;
            input->text_error = SV_TEXT_OK;
        }
    }
    editor_end(input, &editor);
    return SV_CREDENTIAL_EDITING;
}
