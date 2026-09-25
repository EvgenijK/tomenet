#include "protocol/login.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <string.h>

static void feed_one_at_a_time(SvLogin *login, const unsigned char *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        assert(sv_login_receive(login, bytes + i, 1) == SV_OK);
}
int main(void)
{
    const int modern[6] = {4, 9, 2, 1, 0, 2};
    const unsigned char iaddr[6] = {0xf4, 0x43, 1, 2, 3, 4};
    SvLogin *login = sv_login_create(modern, iaddr);
    assert(login);
    unsigned char output[128];
    SvOutput sent = sv_login_take_output(login, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == 8 && output[0] == PKT_LOGIN && !output[1]);
    assert(!memcmp(output + 2, iaddr, 6));
    static const unsigned char overview[] = {
        PKT_SERVERDETAILS, 0,0,0,8, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        PKT_LOGIN, 0,1, 0xff,'W',0, 'H','e','r','o',0, 0,42, 0,2, 0,3,
        'a','t',' ','h','o','m','e',0,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    feed_one_at_a_time(login, overview, sizeof(overview));
    assert(sv_login_state(login) == SV_LOGIN_OVERVIEW);
    assert(sv_login_count(login) == 1);
    assert(!strcmp(sv_login_character(login, 0)->name, "Hero"));
    assert(sv_login_character(login, 0)->level == 42);
    assert(sv_login_flags(login)[0] == 8);
    assert(sv_login_choose(login, 1) == SV_INVALID);
    assert(sv_login_choose(login, 0) == SV_OK);
    sent = sv_login_take_output(login, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == 6);
    assert(output[0] == PKT_LOGIN && !strcmp((char *)output + 1, "Hero"));
    const unsigned char status = 0;
    assert(sv_login_receive(login, &status, 1) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_SELECTED);
    static const unsigned char update[] = {PKT_SFLAGS,
        0,0,0,1, 0,0,0,2, 0,0,0,3, 0,0,0,4};
    feed_one_at_a_time(login, update, sizeof(update));
    assert(sv_login_flags(login)[0] == 4 && sv_login_flags(login)[3] == 1);
    static const unsigned char disconnect[] = {PKT_QUIT, 'S','e','r','v','e','r',0};
    feed_one_at_a_time(login, disconnect, sizeof(disconnect));
    assert(sv_login_state(login) == SV_LOGIN_REJECTED);
    assert(!strcmp(sv_login_reason(login), "Server"));
    assert(sv_login_choose(login, 0) == SV_INVALID);
    sv_login_destroy(login);

    const int old[6] = {4, 4, 9, 2, 0, 0};
    login = sv_login_create(old, iaddr);
    sent = sv_login_take_output(login, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == 2);
    static const unsigned char rejected[] = {PKT_QUIT, 'W','r','o','n','g',0};
    feed_one_at_a_time(login, rejected, sizeof(rejected));
    assert(sv_login_state(login) == SV_LOGIN_REJECTED);
    assert(!strcmp(sv_login_reason(login), "Wrong"));
    sv_login_destroy(login);

    login = sv_login_create(modern, iaddr);
    assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    assert(sv_login_receive(login, overview, sizeof(overview)) == SV_OK);
    assert(sv_login_choose(login, 0) == SV_OK);
    assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    const unsigned char invalid_status = 3;
    assert(sv_login_receive(login, &invalid_status, 1) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_REJECTED);
    assert(strstr(sv_login_reason(login), "status 3"));
    sv_login_destroy(login);

    login = sv_login_create(modern, iaddr);
    assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    assert(sv_login_receive(login, overview, 17) == SV_OK);
    const size_t row_size = sizeof(overview) - 17 - 12;
    for (int i = 0; i < SV_LOGIN_MAX_CHARACTERS; ++i)
        assert(sv_login_receive(login, overview + 17, row_size) == SV_OK);
    assert(sv_login_receive(login, overview + 17, row_size) == SV_INPUT_OVERFLOW);
    sv_login_destroy(login);
    return 0;
}
