#include "protocol/contact.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void split_negotiation_reaches_setup(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    assert(contact);
    unsigned char output[256];
    SvOutput sent = sv_contact_take_output(contact, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size > 40);
    assert(!memcmp(output, "\0\0\x30\x39PLAYER\0", 11));
    const unsigned char response[] = {
        255, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        0, 0, 0, 4, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    for (size_t i = 0; i < sizeof(response); ++i) {
        assert(sv_contact_receive(contact, &response[i], 1) == SV_OK);
        assert(sv_contact_state(contact) != SV_CONTACT_READY);
    }
    sent = sv_contact_take_output(contact, output, sizeof(output));
    assert(sent.result == SV_OK && output[0] == 1);
    assert(!memcmp(output + 1, "PLAYER\0Test\0", 12));
    const unsigned char setup[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39,
        0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 13};
    for (size_t i = 0; i < sizeof(setup); ++i) {
        SvResult result = sv_contact_receive(contact, &setup[i], 1);
        if (result != SV_OK) fprintf(stderr, "at=%zu state=%d result=%d\n", i,
                                     sv_contact_state(contact), result);
        assert(result == SV_OK);
    }
    assert(sv_contact_state(contact) == SV_CONTACT_READY);
    assert(sv_contact_version(contact)[0] == 4);
    sv_contact_destroy(contact);
}

static void rejected_contact_never_sends_verify(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    assert(contact);
    unsigned char output[256];
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char rejected[] = {255, 12, 0, 0, 0, 0, 0, 0, 0, 0};
    assert(sv_contact_receive(contact, rejected, sizeof(rejected)) == SV_INVALID);
    assert(sv_contact_state(contact) == SV_CONTACT_FAILED);
    assert(sv_contact_rejection(contact) == 12);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_WAITING);
    sv_contact_destroy(contact);
    identity.password = "a*b";
    assert(!sv_contact_create(2, &identity));
    contact = sv_contact_create(1, &identity);
    assert(contact);
    sv_contact_destroy(contact);
}

static void malformed_setup_releases_contact(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    assert(contact);
    unsigned char output[256];
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char response[] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        0, 0, 0, 4, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    assert(sv_contact_receive(contact, response, sizeof(response)) == SV_OK);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char malformed[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39,
        0, 0, 12, 0, 0, 20, 0, 0, 0, 0, 0, 0, 13};
    assert(sv_contact_receive(contact, malformed, sizeof(malformed)) == SV_DECODE_ERROR);
    assert(sv_contact_state(contact) == SV_CONTACT_FAILED);
    sv_contact_destroy(contact);
}

int main(void) {
    split_negotiation_reaches_setup();
    rejected_contact_never_sends_verify();
    malformed_setup_releases_contact();
}
