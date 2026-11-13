#include "login/characters/login-interaction.h"
#include "login/login-view.h"
#include "login/pregame.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <string.h>

static SvLogin *overview_with_limit(unsigned char limit)
{
    static const int version[6] = {4,9,4,0,0,0};
    static const unsigned char iaddr[6] = {0xf4,0x43,1,2,3,4};
    SvLogin *login = sv_login_create(version, iaddr);
    unsigned char output[32];
    assert(login && sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    const unsigned char reply[] = {
        PKT_SERVERDETAILS, 0,0,0,limit, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        PKT_LOGIN, 0,0, 0xff,'W',0, 'H','e','r','o',0, 0,42, 0,0, 0,0, 'H','o','m','e',0,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    assert(sv_login_receive(login, reply, sizeof(reply)) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_OVERVIEW);
    return login;
}

static SvLogin *overview(void) { return overview_with_limit(8); }

static void invocation_reincarnation_preserves_existing_and_creation_gates(void)
{
    SvLogin *login=overview();
    SvLoginInteraction input;
    unsigned char output[32];
    sv_login_interaction_begin(&input,login,NULL,false,false,0);
    assert(sv_login_interaction_request_reincarnation(&input,"hero"));
    assert(sv_login_interaction_sync(&input)==SV_LOGIN_TRANSITION_OVERVIEW);
    assert(sv_login_state(login)==SV_LOGIN_WAIT_STATUS && !sv_login_creating_character(login));
    assert(!input.pending_reincarnation && !sv_login_reincarnation_requested(login));
    assert(sv_login_take_output(login,output,sizeof(output)).result==SV_OK);
    assert(!strcmp((char *)output+1,"Hero"));
    sv_login_destroy(login);

    login=overview();
    sv_login_interaction_begin(&input,login,NULL,false,false,0);
    assert(sv_login_interaction_request_reincarnation(&input,"carol"));
    assert(sv_login_interaction_sync(&input)==SV_LOGIN_TRANSITION_OVERVIEW);
    assert(sv_login_state(login)==SV_LOGIN_OVERVIEW && !input.ordinary_name_active);
    assert(sv_login_take_output(login,output,sizeof(output)).result==SV_WAITING);
    assert(sv_login_interaction_command(&input,(SvLoginCommand){SV_LOGIN_COMMAND_CREATE_ORDINARY,0})==SV_LOGIN_INPUT_NAME_STARTED);
    assert(!input.pending_reincarnation && input.reincarnation_draft && !strcmp(input.character_name.bytes,"Carol"));
    assert(sv_login_interaction_name_key(&input,SV_EDIT_ACCEPT)==SV_LOGIN_INPUT_NAME_SUBMITTED);
    assert(sv_login_reincarnation_requested(login));
    assert(sv_login_take_output(login,output,sizeof(output)).result==SV_OK);
    assert(!strcmp((char *)output+1,"Carol"));
    sv_login_destroy(login);

    login=overview();
    sv_login_interaction_begin(&input,login,NULL,false,false,0);
    assert(sv_login_interaction_request_reincarnation(&input,"Abcdefghijklmno"));
    sv_login_interaction_sync(&input);
    assert(sv_login_interaction_command(&input,(SvLoginCommand){SV_LOGIN_COMMAND_CREATE_ORDINARY,0})==SV_LOGIN_INPUT_NAME_STARTED);
    assert(!input.name_input_truncated && !strcmp(input.character_name.bytes,"Abcdefghijklmno"));
    assert(sv_login_interaction_name_key(&input,SV_EDIT_ACCEPT)==SV_LOGIN_INPUT_NAME_SUBMITTED);
    assert(sv_login_reincarnation_requested(login));
    assert(sv_login_take_output(login,output,sizeof(output)).result==SV_OK);
    assert(!strcmp((char *)output+1,"Abcdefghijklmno"));
    sv_login_destroy(login);

    for (unsigned unsupported=0;unsupported<2;++unsupported) {
        login=overview_with_limit(unsupported?8:1);
        sv_login_interaction_begin(&input,login,NULL,false,false,unsupported?0x10u:0);
        assert(sv_login_interaction_request_reincarnation(&input,"Carol"));
        sv_login_interaction_sync(&input);
        assert(sv_login_interaction_command(&input,(SvLoginCommand){SV_LOGIN_COMMAND_CREATE_ORDINARY,0})==
            (unsupported?SV_LOGIN_INPUT_CREATION_UNSUPPORTED:SV_LOGIN_INPUT_IGNORED));
        assert(!input.ordinary_name_active && sv_login_state(login)==SV_LOGIN_OVERVIEW);
        assert(sv_login_take_output(login,output,sizeof(output)).result==SV_WAITING);
        sv_login_interaction_begin(&input,login,NULL,false,false,0);
        assert(!input.pending_reincarnation && !input.reincarnation_name[0]);
        assert(!sv_login_interaction_request_reincarnation(&input,""));
        sv_login_destroy(login);
    }
}

/* An existing account must not inherit new-installation creation restrictions. */
static void existing_account_on_new_profile_can_create_exclusive(void)
{
    const int version[6] = {4,9,4,0,0,0};
    const unsigned char iaddr[6] = {0xf4,0x43,1,2,3,4};
    const unsigned char reply[] = {
        PKT_SERVERDETAILS, 0,1,2,8, 0,0,0,0, 0,0,0,0, 0,0,0,0xc0,
        PKT_LOGIN, 0,0x44, 0xff,'W',0, 'H','e','r','o',0, 0,42, 0,0, 0,0, 'H','o','m','e',0,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    /* Check both populated and genuinely empty accounts on a fresh profile. */
    for (int populated = 0; populated < 2; ++populated) {
        SvLogin *login = sv_login_create(version, iaddr);
        unsigned char output[32];
        assert(login && sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
        assert(sv_login_receive(login, reply, 17) == SV_OK);
        size_t offset = populated ? 17 : sizeof(reply) - 12;
        assert(sv_login_receive(login, reply + offset, sizeof(reply) - offset) == SV_OK);
        SvLoginInteraction input;
        sv_login_interaction_begin(&input, login, NULL, false, true, 0);
        assert(sv_login_interaction_sync(&input) == SV_LOGIN_TRANSITION_OVERVIEW);
        assert(input.first_run == !populated);
        assert(sv_login_interaction_exclusive_modes(&input) == (populated ? 0xc0u : 0u));
        SvPregame pregame;
        SvContactSetup setup = {0};
        sv_pregame_begin(&pregame, 41, true);
        assert(sv_pregame_contact_ready(&pregame, 41) == SV_OK);
        assert(sv_pregame_sync_login(&pregame, 41, login, &setup, false) == SV_OK);
        assert(pregame.first_run == !populated);
        if (populated) {
            assert(sv_login_interaction_command(&input,
                (SvLoginCommand){SV_LOGIN_COMMAND_CREATE_EXCLUSIVE, 0}) == SV_LOGIN_INPUT_NAME_STARTED);
            assert(input.dedicated_creation && input.ordinary_name_active);
        }
        sv_login_destroy(login);
    }
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
    sv_pregame_begin(&pregame, 11, false);
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
    sv_pregame_begin(&pregame, 21, false);
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

    sv_pregame_begin(&pregame, 22, false);
    assert(sv_pregame_sync_login(&pregame, 21, login, &setup, false) == SV_STALE);
    assert(pregame.phase == SV_PREGAME_CONTACT && !pregame.authenticated);
    sv_login_destroy(login);
}

typedef SvResult (*TerminalTransition)(SvPregame *, uint64_t, const char *);

static void terminal_transition_preserves_public_outcome(
    TerminalTransition terminal, SvPregamePhase destination)
{
    SvPregame pregame;
    sv_pregame_begin(&pregame, 31, false);
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

    sv_pregame_begin(&pregame, 32, false);
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
    sv_pregame_begin(&pregame, 33, false);
    assert(sv_pregame_fail(&pregame, 33, "first terminal reason") == SV_OK);
    assert(sv_pregame_disconnect(&pregame, 33, "late disconnect") == SV_CLOSED);
    assert(pregame.phase == SV_PREGAME_FAILED && pregame.revision == 2);
    assert(!strcmp(pregame.reason, "first terminal reason"));

    sv_pregame_begin(&pregame, 34, false);
    assert(sv_pregame_disconnect(&pregame, 34, "first terminal reason") == SV_OK);
    assert(sv_pregame_fail(&pregame, 34, "late failure") == SV_CLOSED);
    assert(pregame.phase == SV_PREGAME_DISCONNECTED && pregame.revision == 2);
    assert(!strcmp(pregame.reason, "first terminal reason"));
}

static void creation_waits_for_play_ack_before_motd(void)
{
    for (unsigned skip = 0; skip < 2; ++skip) {
        SvLogin *login = overview();
        SvLoginInteraction input;
        sv_login_interaction_begin(&input, login, NULL, skip, false, 0);
        assert(sv_login_create_character(login, "Carol", false) == SV_OK);
        unsigned char output[32768], status = 0;
        assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
        assert(sv_login_receive(login, &status, 1) == SV_OK);
        assert(sv_login_interaction_sync(&input) == SV_LOGIN_TRANSITION_BIRTH);
        assert(sv_login_interaction_command(&input,
            (SvLoginCommand){SV_LOGIN_COMMAND_ACK_MOTD, 0}) == SV_LOGIN_INPUT_IGNORED);
        assert(!sv_login_interaction_complete(&input));
        const int16_t stats[6] = {0, 1, 2, 3, 4, 5};
        assert(sv_login_start_play(login, 0, 0, 0, 0, stats, "font.ttf") == SV_OK);
        assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
        const unsigned char reply[] = {PKT_REPLY, PKT_PLAY, PKT_SUCCESS};
        assert(sv_login_receive(login, reply, 2) == SV_OK);
        sv_login_interaction_sync(&input);
        assert(!sv_login_interaction_complete(&input));
        assert(sv_login_interaction_command(&input,
            (SvLoginCommand){SV_LOGIN_COMMAND_ACK_MOTD, 0}) == SV_LOGIN_INPUT_IGNORED);
        assert(sv_login_receive(login, reply + 2, 1) == SV_OK);
        assert(sv_login_interaction_sync(&input) == SV_LOGIN_TRANSITION_MOTD);
        assert(sv_login_interaction_complete(&input) == (bool)skip);
        assert(sv_login_interaction_command(&input,
            (SvLoginCommand){SV_LOGIN_COMMAND_ACK_MOTD, 0}) ==
            (skip ? SV_LOGIN_INPUT_IGNORED : SV_LOGIN_INPUT_HANDLED));
        assert(sv_login_interaction_complete(&input));
        sv_login_destroy(login);
    }
}

int main(void)
{
    invocation_reincarnation_preserves_existing_and_creation_gates();
    existing_account_on_new_profile_can_create_exclusive();
    creation_waits_for_play_ack_before_motd();
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
    sv_pregame_begin(&pregame, 7, false);
    assert(sv_pregame_contact_ready(&pregame, 7) == SV_OK);
    assert(sv_pregame_contact_ready(&pregame, 8) == SV_STALE);
    sv_login_interaction_begin(&input, login, NULL, false, false, 0);
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
        (SvLoginCommand){SV_LOGIN_COMMAND_CANCEL, 0}) == SV_LOGIN_INPUT_BACK);
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
    sv_pregame_begin(&pregame, 9, false);
    assert(sv_pregame_contact_ready(&pregame, 9) == SV_OK);
    sv_login_interaction_begin(&input, login, "hero", true, false, 0);
    sv_login_interaction_sync(&input);
    assert(sv_pregame_sync_login(&pregame, 9, login, &setup, false) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_WAIT_STATUS);
    assert(sv_login_receive(login, &status, 1) == SV_OK);
    sv_login_interaction_sync(&input);
    assert(sv_login_interaction_complete(&input));
    assert(sv_pregame_sync_login(&pregame, 9, login, &setup, true) == SV_OK);
    assert(pregame.phase == SV_PREGAME_LIVE_HANDOFF);
    sv_login_destroy(login);

    login = overview();
    sv_login_interaction_begin(&input, login, NULL, false, false, 0x10u);
    sv_login_interaction_sync(&input);
    assert(sv_login_interaction_command(&input,
        (SvLoginCommand){SV_LOGIN_COMMAND_CREATE_ORDINARY, 0}) ==
        SV_LOGIN_INPUT_CREATION_UNSUPPORTED);
    assert(sv_login_interaction_command(&input,
        (SvLoginCommand){SV_LOGIN_COMMAND_CREATE_EXCLUSIVE, 0}) ==
        SV_LOGIN_INPUT_CREATION_UNSUPPORTED);
    assert(!input.ordinary_name_active && !input.dedicated_creation &&
           !input.character_name.length);
    assert(sv_login_take_output(login, NULL, 0).result == SV_WAITING);
    sv_pregame_begin(&pregame, 10, false);
    assert(sv_pregame_contact_ready(&pregame, 10) == SV_OK);
    setup.creation_flags = 0x10u;
    assert(sv_pregame_sync_login(&pregame, 10, login, &setup, false) == SV_OK);
    assert(sv_pregame_creation_unsupported(&pregame, 10) == SV_OK);
    assert(pregame.phase == SV_PREGAME_OVERVIEW &&
           !strcmp(pregame.reason, SV_PREGAME_UNSUPPORTED_CREATION_MODE));
    sv_login_destroy(login);
    return 0;
}
