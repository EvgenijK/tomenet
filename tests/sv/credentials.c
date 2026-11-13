#include "login/account/credentials.h"
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

    sv_credentials_begin(&input, &endpoint, account, password);
    sv_credentials_focus(&input,1);
    sv_credentials_toggle_reveal(&input);
    sv_credentials_focus(&input,4);
    event=key(SDLK_RETURN,0);
    assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_CANCELLED);
    assert(!input.reveal && !input.lookup && !password[0]);
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


    /* Both visible fields share the production editor; focus retains selection. */
    strcpy(account, "Player");
    strcpy(password, "secret");
    sv_credentials_begin(&input, &endpoint, account, password);
    sv_credentials_select(&input, 1, 1, false);
    sv_credentials_select(&input, 1, 4, true);
    event = text("X");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!strcmp(password, "sXet"));
    event = key(SDLK_LEFT, SDL_KMOD_SHIFT);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(input.cursor[1] == 1 && input.anchor[1] == 2);
    event = key(SDLK_TAB, SDL_KMOD_SHIFT);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(input.focus == 0);
    event = key(SDLK_TAB, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(input.focus == 1 && input.cursor[1] == 1 && input.anchor[1] == 2);
    SvCredentialPresentation view;
    sv_credentials_toggle_reveal(&input);
    sv_credentials_view(&input, &view);
    assert(!strcmp(view.password_mask, "****") && !strcmp(view.account, "Player"));
    assert(!strcmp(sv_credentials_private_password(&input), "sXet"));
    assert(sv_credentials_submit(&input) == SV_CREDENTIAL_ACCEPTED);
    assert(!sv_credentials_private_password(&input));
    sv_credentials_focus(&input, 0);
    event = key(SDLK_A, SDL_KMOD_CTRL);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = text("other");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!strcmp(account, "Other") && !strcmp(password, "sXet") && !input.restored_password);
    sv_credentials_focus(&input, 1);
    event = key(SDLK_A, SDL_KMOD_CTRL);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    event = key(SDLK_BACKSPACE, 0);
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    sv_credentials_select(&input, 0, strlen(account), false);
    event = text("2");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    sv_credentials_focus(&input, 1);
    assert(input.lookup);
    uint64_t generation = input.generation;
    sv_credentials_select(&input, 0, strlen(account), false);
    event = text("2");
    assert(sv_credentials_event(&input, &event) == SV_CREDENTIAL_EDITING);
    assert(!input.lookup && input.generation != generation && !input.account_checked);
    sv_credentials_poll(&input);
    assert(!password[0]);
    sv_credentials_end(&input);

    /* SDL may expose Tab through its physical scancode or LEFT_TAB symbol. */
    sv_credentials_begin(&input,&endpoint,account,password);
    event=key(SDLK_UNKNOWN,0);event.key.scancode=SDL_SCANCODE_TAB;
    assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
    assert(input.focus==1 && input.stage==1);
    event=key(SDLK_LEFT_TAB,0);event.key.scancode=SDL_SCANCODE_TAB;
    assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
    assert(input.focus==0 && input.stage==0);
    sv_credentials_end(&input);

    /* Fixed editing keys must work before SDL has a logical key mapping. */
    for (int field = 0; field < 2; ++field) {
        strcpy(account,"Alpha");strcpy(password,"Gamma");
        sv_credentials_begin(&input,&endpoint,account,password);
        sv_credentials_focus(&input,field);
        char *draft=field?password:account;
        event=key(SDLK_UNKNOWN,0);event.key.scancode=SDL_SCANCODE_BACKSPACE;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(!strcmp(draft,field?"Gamm":"Alph"));
        event.key.scancode=SDL_SCANCODE_HOME;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(input.cursor[field]==0);
        event.key.scancode=SDL_SCANCODE_DELETE;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(!strcmp(draft,field?"amm":"lph"));
        event.key.scancode=SDL_SCANCODE_END;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(input.cursor[field]==3);
        event.key.scancode=SDL_SCANCODE_LEFT;event.key.mod=SDL_KMOD_SHIFT;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(input.cursor[field]==2 && input.anchor[field]==3);
        event.key.scancode=SDL_SCANCODE_A;event.key.mod=SDL_KMOD_CTRL;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(input.cursor[field]==3 && input.anchor[field]==0);
        event.key.scancode=SDL_SCANCODE_BACKSPACE;event.key.mod=0;
        assert(sv_credentials_event(&input,&event)==SV_CREDENTIAL_EDITING);
        assert(!draft[0]);
        sv_credentials_end(&input);
    }

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
