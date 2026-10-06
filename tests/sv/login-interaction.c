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

int main(void)
{
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
