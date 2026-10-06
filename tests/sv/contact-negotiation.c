#include "protocol/contact.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void contact_and_verify_bytes_are_exact_and_atomic(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "test",
                                  .host_name = "localhost", .password = "pw"};
    static const unsigned char modern_contact[] = {
        0,0,0x30,0x39, 'P','L','A','Y','E','R',0, 0,0, 0xff,
        'T','e','s','t',0, 'l','o','c','a','l','h','o','s','t',0, 0xff,0xff,
        0,0,0,4, 0,0,0,9, 0,0,0,4, 0,0,0,0, 0,0,0,0,
        0x17,0xf6,0x08,0x80
    };
    static const unsigned char old_contact[] = {
        0,0,0x30,0x39, 'P','L','A','Y','E','R',0, 0,0, 0xff,
        'T','e','s','t',0, 'l','o','c','a','l','h','o','s','t',0, 0x49,0x40
    };
    static const unsigned char modern_reply[] = {255,0, 0,0,0,0, 0,0,0,2,
        0,0,0,4, 0,0,0,9, 0,0,0,4, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    static const unsigned char old_reply[] = {255,0, 0,0,0,0, 0,0,0,0};
    static const unsigned char modern_verify[] = {
        1, 'P','L','A','Y','E','R',0, 'T','e','s','t',0, 'Z',']',0
    };
    static const unsigned char old_verify[] = {
        1, 'P','L','A','Y','E','R',0, 'T','e','s','t',0, 'p','w',0
    };
    const unsigned char *contacts[] = {old_contact, modern_contact};
    const size_t contact_sizes[] = {sizeof(old_contact), sizeof(modern_contact)};
    const unsigned char *replies[] = {old_reply, modern_reply};
    const size_t reply_sizes[] = {sizeof(old_reply), sizeof(modern_reply)};
    const unsigned char *verifies[] = {old_verify, modern_verify};
    const size_t verify_sizes[] = {sizeof(old_verify), sizeof(modern_verify)};
    for (int protocol = 1; protocol <= 2; ++protocol) {
        unsigned char output[SV_CONTACT_OUTPUT_CAPACITY] = {0xa5};
        SvContact *contact = sv_contact_create(protocol, &identity);
        assert(contact);
        SvOutput sent = sv_contact_take_output(contact, output,
                                                contact_sizes[protocol - 1] - 1);
        assert(sent.result == SV_OUTPUT_TOO_SMALL &&
               sent.size == contact_sizes[protocol - 1] && output[0] == 0xa5);
        sent = sv_contact_take_output(contact, output, sizeof(output));
        assert(sent.result == SV_OK && sent.size == contact_sizes[protocol - 1]);
        assert(!memcmp(output, contacts[protocol - 1], sent.size));
        assert(sv_contact_receive(contact, replies[protocol - 1],
                                  reply_sizes[protocol - 1]) == SV_OK);
        memset(output, 0xa5, sizeof(output));
        sent = sv_contact_take_output(contact, output, verify_sizes[protocol - 1] - 1);
        assert(sent.result == SV_OUTPUT_TOO_SMALL &&
               sent.size == verify_sizes[protocol - 1] && output[0] == 0xa5);
        sent = sv_contact_take_output(contact, output, sizeof(output));
        assert(sent.result == SV_OK && sent.size == verify_sizes[protocol - 1]);
        assert(!memcmp(output, verifies[protocol - 1], sent.size));
        sv_contact_destroy(contact);
    }
}

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

static void raw_password_bytes_reach_verify_unchanged_before_protocol_xor(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "\xe9"};
    SvContact *contact = sv_contact_create(2, &identity);
    unsigned char output[256];
    assert(contact);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char response[] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        0, 0, 0, 4, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    assert(sv_contact_receive(contact, response, sizeof(response)) == SV_OK);
    SvOutput sent = sv_contact_take_output(contact, output, sizeof(output));
    const unsigned char expected[] = {1, 'P', 'L', 'A', 'Y', 'E', 'R', 0,
                                      'T', 'e', 's', 't', 0, 0xc3, 0};
    assert(sent.result == SV_OK && sent.size == sizeof(expected));
    assert(!memcmp(output, expected, sizeof(expected)));
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

static void setup_is_available_only_after_complete_input(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    unsigned char output[256];
    assert(contact);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char response[] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        0, 0, 0, 4, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    assert(sv_contact_receive(contact, response, sizeof(response)) == SV_OK);
    assert(sv_contact_creation_flags(contact) == 0);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char setup[] = {
        2, 1, 122, 6, 0, 0, 0x30, 0x39,
        0, 0, 0, 5, 0, 30, 1, 1, 1, 0, 0, 0, 13,
        50, 51, 49, 52, 48, 50, 'H', 'u', 'm', 'a', 'n', 0, 1, 2, 3, 4,
        50, 50, 50, 50, 50, 50, '1', ' ', 'H', 'i', 'd', 'd', 'e', 'n', 0,
        1, 2, 3, 4, 5, 6,
        'T', 'r', 'a', 'i', 't', 0, 9, 8, 7, 6,
        'H', 'e', 'l', 'l', 'o'};
    for (size_t i = 0; i < sizeof(setup); ++i) {
        assert(sv_contact_receive(contact, &setup[i], 1) == SV_OK);
        if (i + 1 < sizeof(setup)) assert(!sv_contact_setup(contact));
    }
    const SvContactSetup *data = sv_contact_setup(contact);
    assert(data && data->race_count == 1 && data->class_count == 1 && data->trait_count == 1);
    assert(data->frames_per_second == 30 && data->motd_size == 5);
    assert(!strcmp(data->races[0].title, "Human") && data->races[0].adjustment[2] == -1);
    assert(data->races[0].choice == 0x01020304);
    assert(data->classes[0].hidden == 1 && data->classes[0].base_class == 1);
    assert(!strcmp(data->classes[0].title, "Hidden") && data->classes[0].recommendation[5] == 6);
    assert(!strcmp(data->traits[0].title, "Trait") && data->traits[0].choice == 0x09080706);
    assert(!memcmp(data->motd, "Hello", 5));
    sv_contact_destroy(contact);
}

static void invalid_contact_marker_does_not_send_verify(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    unsigned char output[256], invalid = 2;
    assert(contact);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    assert(sv_contact_receive(contact, &invalid, 1) == SV_INVALID);
    assert(sv_contact_state(contact) == SV_CONTACT_FAILED);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_WAITING);
    sv_contact_destroy(contact);
}

static void older_server_uses_pre_trait_setup_layout(void)
{
    SvContactIdentity identity = {.real_name = "PLAYER", .account = "Test",
                                  .host_name = "localhost", .password = "pw"};
    SvContact *contact = sv_contact_create(2, &identity);
    unsigned char output[256];
    assert(contact);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char response[] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 18,
        0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0};
    assert(sv_contact_receive(contact, response, sizeof(response)) == SV_OK);
    assert(sv_contact_creation_flags(contact) == 16);
    assert(sv_contact_take_output(contact, output, sizeof(output)).result == SV_OK);
    const unsigned char setup[] = {
        2, 1, 122, 6, 0, 0, 0x30, 0x39,
        0, 0, 0, 0, 0, 20, 1, 1, 0, 0, 0, 12,
        50, 50, 50, 50, 50, 50, 'R', 0, 0, 0, 0, 1,
        50, 50, 50, 50, 50, 50, 'C', 0};
    assert(sv_contact_receive(contact, setup, sizeof(setup)) == SV_OK);
    const SvContactSetup *data = sv_contact_setup(contact);
    assert(data && data->race_count == 1 && data->class_count == 1 &&
           data->trait_count == 0 && data->creation_flags == 16);
    assert(!strcmp(data->classes[0].title, "C"));
    sv_contact_destroy(contact);
}

int main(void) {
    contact_and_verify_bytes_are_exact_and_atomic();
    split_negotiation_reaches_setup();
    rejected_contact_never_sends_verify();
    raw_password_bytes_reach_verify_unchanged_before_protocol_xor();
    malformed_setup_releases_contact();
    setup_is_available_only_after_complete_input();
    invalid_contact_marker_does_not_send_verify();
    older_server_uses_pre_trait_setup_layout();
}
