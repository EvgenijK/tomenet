/* Reuse the controlled contact/setup peer and native gestures from SV-B-023.
 * Its original main remains compiled, but this executable runs only this slice. */
#define main sv_birth_choices_main
#include "name-editor-submit.c"
#undef main
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include "files/dna.h"

static unsigned char expected_play[32768], captured_play[32768];
static size_t expected_size;
static const char *evidence_path;
static int peer_version[6] = {4, 9, 4, 0, 0, 0};
static bool versioned;
static size_t option_offset, option_count;
static bool peer_newer(int a, int b, int c, int d, int e, int f)
{
    const int boundary[6] = {a, b, c, d, e, f};
    for (unsigned i = 0; i < 6; ++i)
        if (peer_version[i] != boundary[i]) return peer_version[i] > boundary[i];
    return false;
}
static bool peer_atleast(int a, int b, int c, int d, int e, int f)
{
    const int boundary[6] = {a, b, c, d, e, f};
    for (unsigned i = 0; i < 6; ++i)
        if (peer_version[i] != boundary[i]) return peer_version[i] > boundary[i];
    return true;
}
/* Controlled server's setup format, from baseline Net_setup. No play bytes
 * are generated here: the independent literal oracle is loaded from disk. */
static void versioned_setup(int socket)
{
    unsigned char setup[1024], *at = setup;
    bool traits = peer_newer(4, 4, 5, 10, 0, 0);
    put32(&at, 0); put16(&at, 20);
    *at++ = 3; *at++ = 3;
    if (traits) *at++ = 3;
    unsigned char *size_at = at; at += 4;
    const char *races[] = {"Human", "Elf", "Dwarf"};
    const char *classes[] = {"Warrior", "Ranger", "Mage"};
    const char *names[] = {"N/A", "Dragon", "Phoenix"};
    for (unsigned i = 0; i < 3; ++i) {
        memset(at, 50, 6); at += 6;
        memcpy(at, races[i], strlen(races[i]) + 1); at += strlen(races[i]) + 1;
        put32(&at, 7);
    }
    for (unsigned i = 0; i < 3; ++i) {
        memset(at, 50, 6); at += 6;
        memcpy(at, classes[i], strlen(classes[i]) + 1); at += strlen(classes[i]) + 1;
        if (peer_newer(4, 4, 3, 1, 0, 0)) { memset(at, 0, 6); at += 6; }
    }
    if (traits) for (unsigned i = 0; i < 3; ++i) {
        memcpy(at, names[i], strlen(names[i]) + 1); at += strlen(names[i]) + 1;
        put32(&at, i ? 7 : 0);
    }
    put32(&size_at, (uint32_t)(at - setup));
    assert(send(socket, setup, (size_t)(at - setup), 0) == (ssize_t)(at - setup));
}
static void versioned_overview(int socket)
{
    unsigned char packet[64], *at = packet;
    *at++ = PKT_SERVERDETAILS;
    put32(&at, 2u | (1u << 8) | (1u << 16));
    put32(&at, 0); put32(&at, 0); put32(&at, 0);
    *at++ = PKT_LOGIN;
    if (peer_newer(4, 4, 9, 2, 0, 0)) put16(&at, 0);
    *at++ = 0; *at++ = 0;
    put16(&at, 0); put16(&at, 0); put16(&at, 0);
    if (peer_newer(4, 5, 7, 0, 0, 0)) *at++ = 0;
    assert(send(socket, packet, (size_t)(at - packet), 0) == (ssize_t)(at - packet));
}
/* Observe the real rendered semantic view on its owner thread. Never retain
 * borrowed view pointers or mutate the interaction from the controlled peer. */
#include "login/ui/endpoint-scene.h"
static pthread_mutex_t driver_lock = PTHREAD_MUTEX_INITIALIZER;
static struct {
    bool enabled, mode_ready, permit_final, final_sent;
    unsigned submitted_owners, stat_step;
    bool stat_selected;
    SvBirthOwner actual_owner;
    unsigned actual_stats;
} driver;

bool __real_sv_endpoint_draw(SDL_Renderer *, SvFont *, const SvEndpoint *,
                            const SvEndpointView *, SvEndpointRenderMetrics *);
bool __wrap_sv_endpoint_draw(SDL_Renderer *renderer, SvFont *font,
                            const SvEndpoint *endpoint, const SvEndpointView *view,
                            SvEndpointRenderMetrics *metrics)
{
    if (!__real_sv_endpoint_draw(renderer, font, endpoint, view, metrics)) return false;
    pthread_mutex_lock(&driver_lock);
    const SvBirthView *birth = view->birth_view;
    if (!driver.enabled || !birth) goto done;
    driver.actual_owner = birth->owner;
    driver.actual_stats = birth->stats_order_count;
    SDL_Keycode key = SDLK_UNKNOWN;
    if (birth->owner == SV_BIRTH_MODE) {
        assert(birth->stats_confirmed && birth->stats_order_count == 6);
        driver.mode_ready = true;
        if (driver.permit_final && !driver.final_sent) {
            key = SDLK_RETURN;
            driver.final_sent = true;
        }
    } else if (birth->owner == SV_BIRTH_STATS) {
        assert(!birth->stats_point_buy);
        if (birth->stats_order_count > driver.stat_step) {
            assert(birth->stats_order_count == driver.stat_step + 1);
            ++driver.stat_step;
            driver.stat_selected = false;
        }
        if (!driver.stat_selected) {
            key = SDLK_A + driver.stat_step;
            driver.stat_selected = true;
        } else if (birth->highlighted_stat == (int)driver.stat_step) {
            /* Wait for the next count before submitting another confirmation. */
            if (!(driver.submitted_owners & (1u << (16 + driver.stat_step)))) {
                driver.submitted_owners |= 1u << (16 + driver.stat_step);
                key = SDLK_RETURN;
            }
        }
    } else if (!(driver.submitted_owners & (1u << birth->owner))) {
        switch (birth->owner) {
        case SV_BIRTH_SEX: key = SDLK_RETURN; break;
        case SV_BIRTH_CLASS: key = versioned ? SDLK_C : SDLK_B; break;
        case SV_BIRTH_RACE: key = SDLK_C; break;
        case SV_BIRTH_TRAIT: key = SDLK_B; break;
        case SV_BIRTH_BODY: key = SDLK_F; break;
        default: break;
        }
        driver.submitted_owners |= 1u << birth->owner;
    }
    if (key != SDLK_UNKNOWN) push_key(key, 0);
done:
    pthread_mutex_unlock(&driver_lock);
    return true;
}

static void wait_for_mode(void)
{
    Uint64 deadline = SDL_GetTicks() + 3000;
    for (;;) {
        pthread_mutex_lock(&driver_lock);
        bool ready = driver.mode_ready;
        if (!ready && SDL_GetTicks() >= deadline) {
            fprintf(stderr, "birth driver: expected MODE after 6 stats; actual owner=%d stats=%u step=%u selected=%d\n",
                    driver.actual_owner, driver.actual_stats, driver.stat_step, driver.stat_selected);
            abort();
        }
        pthread_mutex_unlock(&driver_lock);
        if (ready) return;
        SDL_Delay(1);
    }
}

static void no_final_request(int socket)
{
    struct pollfd ready = {.fd = socket, .events = POLLIN};
    assert(poll(&ready, 1, 120) == 0);
}
static void *complete_peer(void *opaque)
{
    Peer *peer = opaque;
    int socket = accept(peer->listener, NULL, NULL);
    assert(socket >= 0);
    struct timeval timeout = {.tv_sec = 3};
    assert(!setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
    unsigned char number[40];
    char text[80];
    assert(exact(socket, number, 4) && field(socket, text, sizeof(text)) &&
           exact(socket, number + 4, 3) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && exact(socket, number + 7, 26));
    unsigned char contact[34], *at = contact;
    *at++ = 255; *at++ = 0;
    put32(&at, 0); put32(&at, 2);
    for (unsigned i = 0; i < 6; ++i) put32(&at, (uint32_t)peer_version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == sizeof(contact));
    assert(exact(socket, number, 1) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && field(socket, text, sizeof(text)));
    const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == sizeof(verified));
    if (versioned) versioned_setup(socket);
    else send_setup(socket, "complete");
    size_t login_size = peer_newer(4, 9, 2, 1, 0, 1) ? 8 : 2;
    assert(exact(socket, number, login_size));
    assert(number[0] == PKT_LOGIN && number[1] == 0);
    if (versioned) versioned_overview(socket);
    else send_overview(socket, "complete");
    SDL_Delay(200);
    push_key(SDLK_N, SDL_KMOD_SHIFT);
    SDL_Delay(80);
    push_text("Carol"); push_key(SDLK_RETURN, 0);
    assert(exact(socket, number, 7));
    assert(number[0] == PKT_LOGIN && !memcmp(number + 1, "Carol\0", 6));
    no_final_request(socket);
    const unsigned char permitted = 0;
    assert(send(socket, &permitted, 1, 0) == 1);
    unsigned char early_options[200];
    assert(exact(socket, early_options, option_count + 1));
    assert(early_options[0] == PKT_OPTIONS);
    assert(!memcmp(early_options + 1, expected_play + option_offset, option_count));
    SDL_Delay(100);
    no_final_request(socket); /* Options sent; name acceptance is not final play. */
    pthread_mutex_lock(&driver_lock);
    driver.enabled = true;
    pthread_mutex_unlock(&driver_lock);
    wait_for_mode();
    if (!strcmp(peer->mode,"cancel")) {
        /* Production native quit while a draft exists must not publish DNA. */
        push_key(SDLK_Q,SDL_KMOD_CTRL);
        unsigned char closed; assert(recv(socket,&closed,1,0)==0);
        close(socket); return NULL;
    }
    no_final_request(socket); /* Committed stats alone do not complete birth. */
    pthread_mutex_lock(&driver_lock);
    driver.permit_final = true;
    pthread_mutex_unlock(&driver_lock);
    struct pollfd final_ready = {.fd = socket, .events = POLLIN};
    assert(poll(&final_ready, 1, 800) == 1);
    assert(exact(socket, captured_play, expected_size));
    assert(!memcmp(captured_play, expected_play, expected_size));
    FILE *capture = fopen(evidence_path, "wb");
    assert(capture && fwrite(captured_play, 1, expected_size, capture) == expected_size);
    assert(!fclose(capture));
    push_key(SDLK_RETURN, 0); push_key(SDLK_RETURN, 0);
    push_presentation_events();
    interleave_ping(peer, socket, 7); /* Loop still services the peer while waiting. */
    no_final_request(socket);
    const unsigned char reply[] = {PKT_REPLY, PKT_PLAY, PKT_SUCCESS, PKT_END};
    if (strcmp(peer->mode, "waiting")) {
        assert(send(socket, reply, 2, 0) == 2);
        SDL_Delay(100);
        no_final_request(socket);
    }
    push_presentation_events();
    /* Partial replies keep the startup flow waiting; never insert another
     * packet between fragments of this authoritative three-byte reply. */
    if (!strcmp(peer->mode, "ack")) {
        assert(send(socket, reply + 2, 2, 0) == 2);
    }
    SDL_Delay(150);
    if (!strcmp(peer->mode, "ack") && peer_atleast(4, 7, 3, 0, 0, 0)) {
        /* The actual disabled audio outcome still sends the baseline report. */
        unsigned char audio[9] = {0};
        size_t audio_size = peer_atleast(4, 9, 2, 1, 0, 1) ? 9 : 5;
        assert(exact(socket, audio, audio_size));
        assert(audio[0] == PKT_AUDIO);
        for (size_t i = 1; i < audio_size; ++i) assert(audio[i] == 0);
    }
    if (!strcmp(peer->mode,"ack") && peer_atleast(4,8,1,2,0,0)) {
        unsigned char font[16]={0};
        assert(exact(socket,font,16));
        assert(font[0]==PKT_FONT && !font[1] && !font[2] &&
            !memcmp(font+3,"NO_GRAPHICS\0\0",13));
    }
    no_final_request(socket);
    char ack_path[4096];
    assert(snprintf(ack_path, sizeof(ack_path), "%s.reply", evidence_path) < (int)sizeof(ack_path));
    FILE *ack = fopen(ack_path, "wb");
    size_t reply_size = !strcmp(peer->mode, "waiting") ? 0 :
                        !strcmp(peer->mode, "partial") ? 2 : sizeof(reply);
    assert(ack && fwrite(reply, 1, reply_size, ack) == reply_size);
    assert(!fclose(ack));
    SDL_Event quit = {.type = SDL_EVENT_QUIT};
    assert(SDL_PushEvent(&quit));
    unsigned char closed;
    assert(recv(socket, &closed, 1, 0) == 0);
    close(socket);
    return NULL;
}
int main(int argc, char **argv)
{
    assert(argc == 6 || argc == 7);
    versioned = argc == 7;
    if (versioned) assert(sscanf(argv[6], "%d.%d.%d.%d.%d.%d", &peer_version[0],
        &peer_version[1], &peer_version[2], &peer_version[3], &peer_version[4],
        &peer_version[5]) == 6);
    FILE *fixture = fopen(argv[4], "r");
    assert(fixture);
    unsigned value;
    while (fscanf(fixture, "%2x", &value) == 1) {
        assert(expected_size < sizeof(expected_play));
        expected_play[expected_size++] = (unsigned char)value;
    }
    fclose(fixture);
    /* C003 corrects the fixture's former UI-font-as-map identity. Keep the
     * independent literal remainder; absent map consumer reports empty font. */
    if (peer_atleast(4,8,1,2,0,0)) {
        size_t map_font=1+6+(peer_newer(4,4,5,10,0,0)?6:0)+2;
        map_font+=strlen((char *)expected_play+map_font)+1;
        size_t old_font_size=strlen((char *)expected_play+map_font);
        memmove(expected_play+map_font,expected_play+map_font+old_font_size,
            expected_size-map_font-old_font_size);
        expected_size-=old_font_size;
    }
    option_count = peer_newer(4,9,1,0,0,0) ? 199 :
        peer_newer(4,5,8,1,0,1) ? 154 : peer_newer(4,5,5,0,0,0) ? 128 : 96;
    option_offset = 1 + 6 + (peer_newer(4,4,5,10,0,0) ? 6 : 0);
    if (peer_atleast(4,8,1,2,0,0)) {
        option_offset += 2;
        option_offset += strlen((char *)expected_play + option_offset) + 1;
        option_offset += strlen((char *)expected_play + option_offset) + 1;
    }
    option_offset += 12;
    assert(option_offset + option_count < expected_size);
    /* Literal baseline oracle plus intentional C002 settings from real files:
     * wide map projects big_map and 66x44, character values override globals. */
    expected_play[option_offset] = 1;
    expected_play[option_offset + 2] = 0;
    expected_play[option_offset + 7] = peer_newer(4,4,9,1,0,1);
    if (peer_newer(4,4,9,1,0,1)) expected_play[option_offset + option_count + 7] = 44;
    char config_path[4096];
    assert(snprintf(config_path, sizeof(config_path), "%s/sv/tomenet.cfg", argv[1]) < (int)sizeof(config_path));
    FILE *config = fopen(config_path, "w"); assert(config);
    assert(fputs("svSchemaVersion 1\nsound 0\nsvLayout wide\n", config) >= 0 && !fclose(config));
    assert(snprintf(config_path, sizeof(config_path), "%s/sv/global.opt", argv[1]) < (int)sizeof(config_path));
    config = fopen(config_path, "w"); assert(config);
    assert(fputs("X:rogue_like_commands\nY:censor_swearing\n", config) >= 0 && !fclose(config));
    assert(snprintf(config_path, sizeof(config_path), "%s/sv/Carol.opt", argv[1]) < (int)sizeof(config_path));
    config = fopen(config_path, "w"); assert(config);
    assert(fputs("Y:rogue_like_commands\nX:censor_swearing\n", config) >= 0 && !fclose(config));
    evidence_path = argv[5];
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0), .mode = argv[3]};
    assert(peer.listener >= 0);
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 1));
    socklen_t size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &size));
    pthread_t worker;
    assert(!pthread_create(&worker, NULL, complete_peer, &peer));
    SvEndpointOutcome outcome = {0};
    int result = sv_endpoint_run((SvEndpointOptions){
        .root = argv[1], .library = argv[2], .server = "127.0.0.1",
        .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port), .account = "Test", .password = "pw",
        .skip_motd = true, .real_name = "PLAYER", .outcome = &outcome});
    assert(!pthread_join(worker, NULL));
    close(peer.listener);
    if (!strcmp(argv[3],"cancel")) {
        SvDna absent;
        assert(result==0 && outcome.birth_quit && !outcome.birth_mode_ready && !driver.final_sent);
        assert(sv_dna_load(&absent,argv[1],argv[2],"Carol",NULL)==SV_FILE_MISSING);
        puts("SV C003 production birth cancel: no DNA published");
        return 0;
    }
    assert(outcome.birth_mode_ready && outcome.birth_stats_confirmed);
    assert(outcome.phase == (!strcmp(argv[3], "ack") ?
           SV_PREGAME_LIVE_HANDOFF : SV_PREGAME_PLAY_PENDING));
    /* The peer ends every case with SDL_EVENT_QUIT: normal process exit is
     * successful even while play remains pending. The phase above, not the
     * exit code, proves whether the authoritative reply completed handoff. */
    assert(result == 0);
    assert(driver.mode_ready && driver.final_sent);
    assert(outcome.selected_character[0] && !strcmp(outcome.selected_character, "Carol"));
    assert(peer.interleaved_pongs == 1);
    assert(outcome.birth_sex == SV_BIRTH_SEX_MALE && outcome.birth_race == 2 &&
           outcome.birth_class == 2 && outcome.birth_trait ==
           (peer_newer(4, 4, 5, 10, 0, 0) ? 2 : SV_BIRTH_TRAIT_NONE) &&
           outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    static const int16_t expected_stats[] = {0, 1, 2, 3, 4, 5};
    assert(!memcmp(outcome.birth_committed_stats, expected_stats, sizeof(expected_stats)));

    SvDna actual_dna;
    assert(sv_dna_load(&actual_dna,argv[1],argv[2],"Carol",NULL)==SV_FILE_OK);
    assert(actual_dna.class_id==2 && actual_dna.race==2 && actual_dna.mode==0x300);
    assert(!memcmp(actual_dna.stats,expected_stats,sizeof(expected_stats)));
    printf("SV-B-024 %s captured %zu handshake bytes; phase=%s character=%s generation=%llu\n",
           argv[3], expected_size, sv_pregame_phase_name(outcome.phase),
           outcome.selected_character, (unsigned long long)outcome.generation);
    return 0;
}
