#include "protocol/login.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void feed_one_at_a_time(SvLogin *login, const unsigned char *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        assert(sv_login_receive(login, bytes + i, 1) == SV_OK);
}
static void birth_completion(void)
{
    const int version[6] = {4, 9, 4, 0, 0, 0};
    const unsigned char iaddr[6] = {0};
    const int16_t stats[6] = {0, 1, 2, 3, 4, 5};
    /* Independent baseline fixture, also used by the controlled endpoint peer. */
    unsigned char expected[32768], output[32768];
    size_t expected_size = 0;
    unsigned value;
    FILE *fixture = fopen("tests/sv/fixtures/birth-current.hex", "r");
    assert(fixture);
    while (fscanf(fixture, "%2x", &value) == 1) {
        assert(expected_size < sizeof(expected));
        expected[expected_size++] = (unsigned char)value;
    }
    assert(!fclose(fixture));
    static const unsigned char overview[] = {
        PKT_SERVERDETAILS, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        PKT_LOGIN, 0,0, 0, 0, 0,0, 0,0, 0,0, 0
    };
    const unsigned char replies[][3] = {
        {PKT_REPLY, PKT_PLAY, PKT_SUCCESS},
        {PKT_REPLY, PKT_PLAY, 0},
        {PKT_REPLY, PKT_LOGIN, PKT_SUCCESS}
    };
    for (unsigned test = 0; test < 4; ++test) {
        SvLogin *login = sv_login_create(version, iaddr);
        assert(login);
        assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
        assert(sv_login_start_play(login, 0x0300, 2, 2, 2, stats,
                                  "CascadiaMono-Regular.ttf") == SV_INVALID);
        feed_one_at_a_time(login, overview, sizeof(overview));
        assert(sv_login_create_character(login, "Carol", false) == SV_OK);
        assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
        assert(sv_login_start_play(login, 0x0300, 2, 2, 2, stats,
                                  "CascadiaMono-Regular.ttf") == SV_INVALID);
        /* New names receive E_NEED_INFO, not existing-character SUCCESS. */
        const unsigned char accepted = E_NEED_INFO;
        assert(sv_login_receive(login, &accepted, 1) == SV_OK);
        assert(sv_login_start_play(login, 0x0300, 2, 2, 2, stats,
                                  "CascadiaMono-Regular.ttf") == SV_OK);
        assert(sv_login_state(login) == SV_LOGIN_WAIT_PLAY);
        assert(sv_login_start_play(login, 0x0300, 2, 2, 2, stats,
                                  "CascadiaMono-Regular.ttf") == SV_INVALID);
        output[0] = 0xa5;
        SvOutput packet = sv_login_take_output(login, output, expected_size - 1);
        assert(packet.result == SV_OUTPUT_TOO_SMALL && packet.size == expected_size);
        assert(output[0] == 0xa5);
        packet = sv_login_take_output(login, output, sizeof(output));
        assert(packet.result == SV_OK && packet.size == expected_size);
        assert(!memcmp(output, expected, expected_size));
        assert(sv_login_take_output(login, output, sizeof(output)).result == SV_WAITING);
        if (test == 3) {
            const unsigned char end = PKT_END;
            assert(sv_login_receive(login, &end, 1) == SV_DECODE_ERROR);
            assert(sv_login_state(login) == SV_LOGIN_WAIT_PLAY);
            sv_login_destroy(login);
            continue;
        }
        feed_one_at_a_time(login, replies[test], 2);
        assert(sv_login_state(login) == SV_LOGIN_WAIT_PLAY);
        /* nserver.c sends VERSION and gameplay data immediately after END.
         * TCP may coalesce those bytes with the authoritative play reply. */
        const unsigned char completed[] = {PKT_SUCCESS, PKT_END, PKT_VERSION, 42};
        SvResult result = test == 0 ?
            sv_login_receive(login, completed, sizeof(completed)) :
            sv_login_receive(login, replies[test] + 2, 1);
        assert(result == (test == 2 ? SV_DECODE_ERROR : SV_OK));
        assert(sv_login_state(login) == (test == 0 ? SV_LOGIN_PLAY_ACKNOWLEDGED :
            test == 1 ? SV_LOGIN_REJECTED : SV_LOGIN_WAIT_PLAY));
        if (test == 0) {
            assert(sv_login_play_stream_complete(login));
            /* The live decoder owns both the coalesced and later bytes. */
            const unsigned char later[] = {43, 44};
            assert(sv_login_receive(login, later, sizeof(later)) == SV_OK);
            const unsigned char live[] = {PKT_VERSION, 42, 43, 44};
            assert(sv_login_take_remaining(login, output, sizeof(live) - 1).result ==
                   SV_OUTPUT_TOO_SMALL);
            SvOutput remaining = sv_login_take_remaining(login, output, sizeof(output));
            assert(remaining.result == SV_OK && remaining.size == sizeof(live));
            assert(!memcmp(output, live, sizeof(live)));
            assert(sv_login_state(login) == SV_LOGIN_PLAY_ACKNOWLEDGED);
        }
        assert(sv_login_start_play(login, 0x0300, 2, 2, 2, stats,
                                  "CascadiaMono-Regular.ttf") == SV_INVALID);
        assert(sv_login_take_output(login, output, sizeof(output)).result == SV_WAITING);
        sv_login_destroy(login);
    }
}
static void resource_report_startup(void)
{
    const int version[6] = {4,9,4,0,0,0};
    const unsigned char iaddr[6] = {0};
    unsigned char bytes[32768], overview[29] = {PKT_SERVERDETAILS};
    SvLogin *login = sv_login_create(version,iaddr); assert(login);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result == SV_OK);
    overview[17] = PKT_LOGIN;
    assert(sv_login_receive(login,overview,sizeof(overview)) == SV_OK);
    assert(sv_login_create_character(login,"Reporting",false) == SV_OK);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result == SV_OK);
    unsigned char accepted = E_NEED_INFO;
    assert(sv_login_receive(login,&accepted,1) == SV_OK);
    SvSettingsAudioSnapshot audio = {.status=SV_AUDIO_READY,.sfx=1,.music=2};
    memset(audio.sound_name,'s',80); audio.sound_name[80] = 0;
    assert(sv_login_set_audio(login,&audio) == SV_OK);
    const int16_t stats[6] = {0};
    assert(sv_login_start_play(login,0,0,0,0,stats,"") == SV_OK);
    SvOutput out = sv_login_take_output(login,bytes,sizeof(bytes));
    const unsigned char resource[] = {0,0,'N','O','_','G','R','A','P','H','I','C','S',0,0};
    assert(out.result == SV_OK && !memcmp(bytes+13,resource,sizeof(resource)));
    assert(sv_login_send_font(login) == SV_INVALID);
    const unsigned char ack[] = {PKT_REPLY,PKT_PLAY,PKT_SUCCESS};
    assert(sv_login_receive(login,ack,sizeof(ack)) == SV_OK);
    assert(sv_login_send_font(login) == SV_OK);
    out = sv_login_take_output(login,bytes,sizeof(bytes));
    assert(out.size == sizeof(resource)+1 && bytes[0] == PKT_FONT &&
        !memcmp(bytes+1,resource,sizeof(resource)));
    /* A request while startup still owns the stream uses the same serializer. */
    const unsigned char request = PKT_VERSION;
    assert(sv_login_receive(login,&request,1) == SV_OK);
    out = sv_login_take_output(login,bytes,sizeof(bytes));
    assert(out.result == SV_OK && bytes[0] == PKT_VERSION);
    size_t at = 1 + strlen((const char *)bytes+1)+1;
    at += strlen((const char *)bytes+at)+1; /* version, OS */
    assert(!memcmp(bytes+at,"\xff\xff\xff\xff\xff\xff\xff\xff",8));
    at += 8; at += strlen((const char *)bytes+at)+1; at += 4; /* tag, language */
    assert(strlen((const char *)bytes+at) == 79 && strlen(audio.sound_name) == 80);
    assert(!memcmp(bytes+at,audio.sound_name,79));
    sv_login_destroy(login);
}

int main(void)
{
    birth_completion();
    resource_report_startup();
    const int modern[6] = {4, 9, 2, 1, 0, 2};
    const unsigned char iaddr[6] = {0xf4, 0x43, 1, 2, 3, 4};
    SvLogin *login = sv_login_create(modern, iaddr);
    assert(login);
    unsigned char output[128];
    memset(output, 0xa5, sizeof(output));
    SvOutput short_output = sv_login_take_output(login, output, 7);
    assert(short_output.result == SV_OUTPUT_TOO_SMALL && short_output.size == 8 &&
           output[0] == 0xa5);
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
    static const unsigned char controls[] = {
        PKT_KEEPALIVE,
        PKT_PING, 0, 0,0,0,7, 0,0,0,8, 0,0,0,9, 0
    };
    feed_one_at_a_time(login, controls, sizeof(controls));
    sent = sv_login_take_output(login, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == sizeof(controls) - 1);
    assert(output[0] == PKT_PING && output[1] == 1);
    assert(!memcmp(output + 2, controls + 3, sizeof(controls) - 3));
    assert(sv_login_take_output(login, output, sizeof(output)).result == SV_WAITING);
    assert(sv_login_keepalive(login) == SV_OK);
    sent = sv_login_take_output(login, output, sizeof(output));
    assert(sent.result == SV_OK && sent.size == 1 && output[0] == PKT_KEEPALIVE);
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
    memset(output, 0xa5, sizeof(output));
    short_output = sv_login_take_output(login, output, 1);
    assert(short_output.result == SV_OUTPUT_TOO_SMALL && short_output.size == 2 &&
           output[0] == 0xa5);
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
    const unsigned char invalid_status = E_PASSWORD;
    assert(sv_login_receive(login, &invalid_status, 1) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_REJECTED);
    assert(strstr(sv_login_reason(login), "status 5"));
    sv_login_destroy(login);

    /* Server authority also covers a listed character removed since overview. */
    login = sv_login_create(modern, iaddr);
    assert(sv_login_take_output(login, output, sizeof(output)).result == SV_OK);
    assert(sv_login_receive(login, overview, sizeof(overview)) == SV_OK);
    assert(sv_login_choose(login, 0) == SV_OK);
    const unsigned char need_info = E_NEED_INFO;
    assert(sv_login_receive(login, &need_info, 1) == SV_OK);
    assert(sv_login_state(login) == SV_LOGIN_SELECTED && sv_login_creating_character(login));
    assert(!strcmp(sv_login_selected_character(login)->name, "Hero"));
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
