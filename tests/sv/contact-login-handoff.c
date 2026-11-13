#include "protocol/contact.h"
#include "protocol/login.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <string.h>

int main(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    unsigned char output[512];
    assert(contact);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    static const unsigned char response[] = {255,0, 0,0,0,0, 0,0,0,2,
        0,0,0,4, 0,0,0,9, 0,0,0,4, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    assert(sv_contact_receive(contact, response, sizeof(response)) == SV_OK);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    static const unsigned char setup_and_login[] = {
        2,1,122,6, 0,0,0x30,0x39,
        0,0,0,0, 0,20, 0,0,0, 0,0,0,13,
        PKT_SERVERDETAILS, 0,0,0,8, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        PKT_LOGIN, 0,0, 0, 'H','e','r','o',0, 0,1, 0,0, 0,0, 0,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    assert(sv_contact_receive(contact, setup_and_login, sizeof(setup_and_login)) == SV_OK);
    assert(sv_contact_state(contact) == SV_CONTACT_READY);
    SvOutput remaining = sv_contact_take_remaining(contact, output, sizeof(output));
    assert(remaining.result == SV_OK && remaining.size > 17);
    const unsigned char iaddr[6] = {0xf4, 0x43, 1, 2, 3, 4};
    SvLogin *login = sv_login_create(sv_contact_version(contact), iaddr);
    assert(login);
    assert(sv_login_receive(login, output, remaining.size) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_OVERVIEW);
    assert(sv_login_count(login) == 1);
    assert(!strcmp(sv_login_character(login, 0)->name, "Hero"));
    sv_login_destroy(login);
    sv_contact_destroy(contact);
    return 0;
}
