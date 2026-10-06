#include "input/login-interaction.h"
#include "session/login-view.h"
#include "session/pregame.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <string.h>

static SvLogin *overview(void)
{
    static const int version[6] = {4,9,4,0,0,0};
    static const unsigned char iaddr[6] = {0xf4,0x43,1,2,3,4};
    SvLogin *login = sv_login_create(version, iaddr);
    unsigned char output[32];
    assert(login && sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    static const unsigned char reply[] = {
        PKT_SERVERDETAILS, 0,0,0,8, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        PKT_LOGIN, 0,0, 0xff,'W',0, 'H','e','r','o',0, 0,42, 0,0, 0,0, 'H','o','m','e',0,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    assert(sv_login_receive(login, reply, sizeof(reply)) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_OVERVIEW);
    return login;
}

static void new_account_overview_is_published_atomically(void)
{
    static const int version[6] = {4,9,4,0,0,0};
    static const unsigned char iaddr[6] = {0xf4,0x43,1,2,3,4};
    static const unsigned char reply[] = {
        PKT_SERVERDETAILS, 0x10,0x20,0x30,0x40, 0,0,0,3,
        0,0,0,2, 0,0,0,1,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    SvContactSetup setup = {.creation_flags = 0x01020304};
    SvLogin *login = sv_login_create(version, iaddr);
    unsigned char output[32];
    assert(login && sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    SvPregame pregame;
    sv_pregame_begin(&pregame, 11);
    assert(sv_pregame_contact_ready(&pregame, 11) == SV_OK);

    for (size_t i = 0; i + 1 < sizeof(reply); ++i) {
        assert(sv_login_receive(login, reply + i, 1) == SV_OK);
        assert(sv_pregame_sync_login(&pregame, 11, login, &setup, false) == SV_OK);
        assert(pregame.phase == SV_PREGAME_AUTHENTICATING);
        assert(!pregame.authenticated && !pregame.character_count);
        assert(!pregame.creation_flags && !pregame.server_flags[0]);
    }
    assert(sv_login_receive(login, reply + sizeof(reply) - 1, 1) == SV_OK);
    assert(sv_pregame_sync_login(&pregame, 11, login, &setup, false) == SV_OK);
    assert(pregame.phase == SV_PREGAME_OVERVIEW && pregame.authenticated);
    assert(!pregame.character_count && pregame.creation_flags == 0x01020304);
    assert(pregame.server_flags[0] == 0x10203040 && pregame.server_flags[1] == 3 &&
           pregame.server_flags[2] == 2 && pregame.server_flags[3] == 1);
    sv_login_destroy(login);
}

static void disconnected_generation_rejects_late_login_results(void)
{
    SvContactSetup setup = {0};
    SvLogin *login = overview();
    SvPregame pregame;
    sv_pregame_begin(&pregame, 21);
    assert(sv_pregame_contact_ready(&pregame, 21) == SV_OK);
    assert(sv_pregame_disconnect(&pregame, 21, "Connection closed during login.") == SV_OK);
    uint64_t terminal_revision = pregame.revision;

    assert(sv_pregame_disconnect(&pregame, 21, "late transport error") == SV_CLOSED);
    assert(pregame.phase == SV_PREGAME_DISCONNECTED);
    assert(pregame.revision == terminal_revision);
    assert(!strcmp(pregame.reason, "Connection closed during login."));
    assert(sv_pregame_sync_login(&pregame, 21, login, &setup, false) == SV_CLOSED);
    assert(pregame.phase == SV_PREGAME_DISCONNECTED && !pregame.authenticated);
    assert(!pregame.character_count && !pregame.creation_flags);

    sv_pregame_begin(&pregame, 22);
    assert(sv_pregame_sync_login(&pregame, 21, login, &setup, false) == SV_STALE);
    assert(pregame.phase == SV_PREGAME_CONTACT && !pregame.authenticated);
    sv_login_destroy(login);
}

typedef SvResult (*TerminalTransition)(SvPregame *, uint64_t, const char *);

static void terminal_transition_preserves_public_outcome(
    TerminalTransition terminal, SvPregamePhase destination)
{
    SvPregame pregame;
    sv_pregame_begin(&pregame, 31);
    assert(terminal(NULL, 31, "ignored") == SV_INVALID);
    assert(terminal(&pregame, 0, "ignored") == SV_INVALID);
    assert(terminal(&pregame, 30, "ignored") == SV_STALE);
    assert(pregame.phase == SV_PREGAME_CONTACT && pregame.revision == 1);
    assert(!pregame.reason[0] && !pregame.authenticated && !pregame.character_count);

    assert(terminal(&pregame, 31, NULL) == SV_OK);
    assert(pregame.phase == destination && pregame.revision == 2);
    assert(!pregame.reason[0] && !pregame.authenticated && !pregame.character_count);
    assert(terminal(&pregame, 31, NULL) == SV_CLOSED);
    assert(pregame.phase == destination && pregame.revision == 2);
    assert(!pregame.reason[0]);

    sv_pregame_begin(&pregame, 32);
    assert(terminal(&pregame, 32, "terminal reason") == SV_OK);
    assert(pregame.phase == destination && pregame.revision == 2);
    assert(!strcmp(pregame.reason, "terminal reason"));
    assert(!pregame.authenticated && !pregame.character_count);
    assert(terminal(&pregame, 32, "terminal reason") == SV_CLOSED);
    assert(pregame.phase == destination && pregame.revision == 2);
    assert(!strcmp(pregame.reason, "terminal reason"));
}

static void terminal_transitions_share_the_public_state_rule(void)
{
    terminal_transition_preserves_public_outcome(sv_pregame_fail,
                                                 SV_PREGAME_FAILED);
    terminal_transition_preserves_public_outcome(sv_pregame_disconnect,
                                                 SV_PREGAME_DISCONNECTED);

    SvPregame pregame;
    sv_pregame_begin(&pregame, 33);
    assert(sv_pregame_fail(&pregame, 33, "first terminal reason") == SV_OK);
    assert(sv_pregame_disconnect(&pregame, 33, "late disconnect") == SV_CLOSED);
    assert(pregame.phase == SV_PREGAME_FAILED && pregame.revision == 2);
    assert(!strcmp(pregame.reason, "first terminal reason"));

    sv_pregame_begin(&pregame, 34);
    assert(sv_pregame_disconnect(&pregame, 34, "first terminal reason") == SV_OK);
    assert(sv_pregame_fail(&pregame, 34, "late failure") == SV_CLOSED);
    assert(pregame.phase == SV_PREGAME_DISCONNECTED && pregame.revision == 2);
    assert(!strcmp(pregame.reason, "first terminal reason"));
}

int main(void)
{
    new_account_overview_is_published_atomically();
    disconnected_generation_rejects_late_login_results();
    terminal_transitions_share_the_public_state_rule();
    SvContactSetup setup = {0};
    setup.race_count = setup.class_count = 1;
    strcpy(setup.races[0].title, "Human");
    strcpy(setup.classes[0].title, "Warrior");
    memcpy(setup.motd, "Hello", 5);
    setup.motd_size = 5;
    SvLogin *login = overview();
    SvLoginInteraction input;
    SvLoginView view;
    SvPregame pregame;
    sv_pregame_begin(&pregame, 7);
    assert(sv_pregame_contact_ready(&pregame, 7) == SV_OK);
    assert(sv_pregame_contact_ready(&pregame, 8) == SV_STALE);
    sv_login_interaction_begin(&input, login, NULL, false);
    sv_login_interaction_sync(&input);
    assert(sv_pregame_sync_login(&pregame, 7, login, &setup, false) == SV_OK);
    uint64_t overview_revision = pregame.revision;
    assert(sv_pregame_sync_login(&pregame, 7, login, &setup, false) == SV_OK);
    assert(pregame.revision == overview_revision);
    sv_login_view_prepare(&view, &pregame);
    assert(view.overview && view.count == 1);
    assert(!strcmp(view.rows[0].name, "Hero"));
    assert(!strcmp(view.rows[0].race, "Human"));
    assert(sv_login_interaction_command(&input,
        (SvLoginCommand){SV_LOGIN_COMMAND_QUIT, 0}) == SV_LOGIN_INPUT_QUIT);
    assert(sv_login_interaction_command(&input,
        (SvLoginCommand){SV_LOGIN_COMMAND_CHOOSE, 0}) == SV_LOGIN_INPUT_HANDLED);
    unsigned char sent[32], status = 0;
    SvOutput packet = sv_login_take_output(login, sent, sizeof(sent));
    assert(packet.result == SV_OK && !strcmp((char *)sent + 1, "Hero"));
    assert(sv_login_receive(login, &status, 1) == SV_OK);
    sv_login_interaction_sync(&input);
    assert(!sv_login_interaction_complete(&input));
    assert(sv_pregame_sync_login(&pregame, 7, login, &setup, false) == SV_OK);
    sv_login_view_prepare(&view, &pregame);
    assert(view.motd_size == 5 && !memcmp(view.motd, "Hello", 5));
    assert(sv_login_interaction_command(&input,
        (SvLoginCommand){SV_LOGIN_COMMAND_QUIT, 0}) == SV_LOGIN_INPUT_HANDLED);
    assert(sv_login_interaction_complete(&input));
    assert(sv_pregame_sync_login(&pregame, 7, login, &setup, true) == SV_OK);
    assert(pregame.phase == SV_PREGAME_LIVE_HANDOFF);
    assert(!strcmp(pregame.selected_character, "Hero"));
    sv_login_view_prepare(&view, &pregame);
    assert(view.live_handoff && !view.overview && !view.motd);
    assert(sv_pregame_disconnect(&pregame, 6, "stale") == SV_STALE);
    sv_login_destroy(login);

    login = overview();
    sv_pregame_begin(&pregame, 9);
    assert(sv_pregame_contact_ready(&pregame, 9) == SV_OK);
    sv_login_interaction_begin(&input, login, "hero", true);
    sv_login_interaction_sync(&input);
    assert(sv_pregame_sync_login(&pregame, 9, login, &setup, false) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_WAIT_STATUS);
    assert(sv_login_receive(login, &status, 1) == SV_OK);
    sv_login_interaction_sync(&input);
    assert(sv_login_interaction_complete(&input));
    assert(sv_pregame_sync_login(&pregame, 9, login, &setup, true) == SV_OK);
    assert(pregame.phase == SV_PREGAME_LIVE_HANDOFF);
    sv_login_destroy(login);
    return 0;
}
