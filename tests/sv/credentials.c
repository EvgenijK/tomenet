#include "input/credentials.h"
#include <assert.h>
#include <string.h>

static SDL_Event text(const char *value)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = value;
    return event;
}
static SDL_Event key(SDL_Keycode code, SDL_Keymod mods)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = code;
    event.key.mod = mods;
    return event;
}

int main(int argc, char **argv)
{
    assert(SDL_Init(SDL_INIT_VIDEO));
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint, 18348);
    assert(sv_endpoint_argument(&endpoint, "raw.example:18348"));
    endpoint.protocol = 2;
    char account[80] = {0}, password[80] = {0}, status[180];
    SvCredentialInput input;
    sv_credentials_begin(&input, &endpoint, account, password);
    SDL_Event event = text("Test");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!strcmp(account, "Test"));
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(input.stage == 1 && input.lookup);
    event = text("secret");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!input.lookup && !strcmp(password, "secret"));
    event = text("\xc3\xa9");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(input.text_error == SV_TEXT_ENCODING_ERROR && !strcmp(password, "secret"));
    sv_credentials_status(&input, status);
    assert(!strstr(status, "secret") && strstr(status, "******"));
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = key(SDLK_BACKSPACE, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!strcmp(password, "secre"));
    assert(SDL_SetClipboardText("t*"));
    event = key(SDLK_V, SDL_KMOD_CTRL);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!strcmp(password, "secret*"));
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(input.incompatible_password);
    event = key(SDLK_BACKSPACE, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_ACCEPTED);
    sv_credentials_end(&input);

    account[0] = password[0] = 0;
    sv_credentials_begin(&input, &endpoint, account, password);
    event = text("a12345678901234567890");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(strlen(account) == 15);
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = text("private");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = key(SDLK_ESCAPE, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!input.stage && !password[0] && !input.lookup);
    assert(!strcmp(account, "A12345678901234"));
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_CANCELLED);
    sv_credentials_end(&input);

    account[0] = password[0] = 0;
    sv_credentials_begin(&input, &endpoint, account, password);
    event = text("9alpha?x");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!strcmp(account, "Alpha_x"));
    sv_credentials_end(&input);

    account[0] = password[0] = 0;
    sv_credentials_begin(&input, &endpoint, account, password);
    event = text("Other");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    for (int i = 0; i < 500 && input.lookup; ++i) {
        sv_credentials_poll(&input);
        SDL_Delay(10);
    }
    assert(!input.lookup);
    sv_credentials_status(&input, status);
    if (argc == 2) assert(strstr(status, "No saved password") && strstr(status, "manually"));
    else assert(strstr(status, "Vault unavailable") && strstr(status, "session"));
    event = text("manual");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = key(SDLK_RETURN, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_ACCEPTED);
    sv_credentials_end(&input);

    if (argc == 2 && !strcmp(argv[1], "provider")) {
        char identity[SV_VAULT_KEY_CAPACITY];
        assert(sv_vault_key(identity, endpoint.host, strlen(endpoint.host),
                            endpoint.port, "Test", 4));
        static const unsigned char raw[] = {'p', 0xe9};
        SvVaultRequest *store = sv_vault_store(identity, 70, raw, sizeof(raw));
        assert(store);
        SvVaultResult result = SV_VAULT_PENDING;
        for (int i = 0; i < 500 && result == SV_VAULT_PENDING; ++i) {
            result = sv_vault_poll(&store, 70, NULL, 0, NULL);
            SDL_Delay(10);
        }
        assert(result == SV_VAULT_SAVED);
        account[0] = password[0] = 0;
        sv_credentials_begin(&input, &endpoint, account, password);
        event = text("Test");
        assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
        event = key(SDLK_RETURN, 0);
        assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
        for (int i = 0; i < 500 && input.lookup; ++i) {
            sv_credentials_poll(&input);
            SDL_Delay(10);
        }
        assert(!input.lookup && input.restored_password);
        assert((unsigned char)password[0] == 'p' && (unsigned char)password[1] == 0xe9 && !password[2]);
        sv_credentials_status(&input, status);
        assert(!strstr(status, "p\xe9") && strstr(status, "Password: **"));
        event = key(SDLK_RETURN, 0);
        assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_ACCEPTED);
        sv_credentials_end(&input);
    }
    SDL_Quit();
    return 0;
}
