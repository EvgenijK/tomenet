/* Reuse the controlled contact/setup peer and native gestures from SV-B-023.
 * Its original main remains compiled, but this executable runs only this slice. */
#define main sv_birth_choices_main
#include "name-editor-submit.c"
#undef main
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "session/birth.h"

/* Observe real SDL/TTF output; no replacement Guide, scene or interaction. */
static pthread_mutex_t observations = PTHREAD_MUTEX_INITIALIZER;
static unsigned guide_frames, motd_frames, handoff_frames;
static unsigned frame_kind;
static bool acknowledged, skip, point_buy;
static unsigned last_frame_kind, guide_visits;
static const char *capture_prefix;
extern SDL_Surface * SDLCALL __real_TTF_RenderText_Blended(TTF_Font *, const char *, size_t, SDL_Color);
SDL_Surface * SDLCALL __wrap_TTF_RenderText_Blended(TTF_Font *font, const char *text, size_t length, SDL_Color color)
{
    if (!strcmp(text, "The guide is in development")) frame_kind = 1;
    if (!strcmp(text, "Peer MOTD: welcome Carol")) frame_kind = 2;
    if (!strcmp(text, "Waiting for live-session handoff (Stage C).")) frame_kind = 3;
    return __real_TTF_RenderText_Blended(font, text, length, color);
}
extern bool SDLCALL __real_SDL_RenderPresent(SDL_Renderer *);
bool SDLCALL __wrap_SDL_RenderPresent(SDL_Renderer *renderer)
{
    pthread_mutex_lock(&observations);
    unsigned *count = frame_kind == 1 ? &guide_frames : frame_kind == 2 ? &motd_frames :
                      frame_kind == 3 ? &handoff_frames : NULL;
    if (frame_kind >= 2) assert(acknowledged);
    bool capture = count && !(*count)++;
    if (frame_kind == 1 && last_frame_kind != 1) {
        ++guide_visits;
        capture = true;
    }
    if (capture) {
        char path[4096];
        assert(snprintf(path, sizeof(path), "%s-%u-%u.bmp", capture_prefix, frame_kind, guide_visits) < (int)sizeof(path));
        SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
        assert(surface && SDL_SaveBMP(surface, path));
        SDL_DestroySurface(surface);
        printf("native surface=%u guide-visit=%u presented\n", frame_kind, guide_visits);
    }
    last_frame_kind = frame_kind;
    frame_kind = 0;
    pthread_mutex_unlock(&observations);
    return __real_SDL_RenderPresent(renderer);
}
static void guide_return(void)
{
    push_guide(); SDL_Delay(50);
    push_key(SDLK_SPACE, 0); SDL_Delay(50);
}
static void guide_to_mode(void)
{
    push_key(SDLK_RETURN, 0); /* sex has no Guide */
#if SV_CLASS_BEFORE_RACE
    push_key(SDLK_C, 0); guide_return(); push_key(SDLK_RETURN, 0);
#endif
    push_key(SDLK_C, 0); guide_return(); push_key(SDLK_RETURN, 0);
    push_key(SDLK_B, 0); guide_return(); push_key(SDLK_RETURN, 0);
    push_key(SDLK_F, 0); guide_return(); push_key(SDLK_RETURN, 0);
#if !SV_CLASS_BEFORE_RACE
    push_key(SDLK_C, 0); guide_return(); push_key(SDLK_RETURN, 0);
#endif
    if (point_buy) {
        push_key(SDLK_RIGHT, 0); /* edit draft before opening Guide */
        guide_return();
        push_key(SDLK_RETURN, 0); push_key(SDLK_RETURN, 0);
    } else {
        push_guide(); /* stat ordering has no Guide */
        for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
            push_key(key, 0); push_key(SDLK_RETURN, 0);
        }
    }
    guide_return();
}

static unsigned char expected_play[32768], captured_play[32768];
static size_t expected_size;
static const char *evidence_path;
static int peer_version[6] = {4, 9, 4, 0, 0, 0};
static bool versioned;
static bool peer_newer(int a, int b, int c, int d, int e, int f)
{
    const int boundary[6] = {a, b, c, d, e, f};
    for (unsigned i = 0; i < 6; ++i)
        if (peer_version[i] != boundary[i]) return peer_version[i] > boundary[i];
    return false;
}
/* Controlled server's setup format, from baseline Net_setup. No play bytes
 * are generated here: the independent literal oracle is loaded from disk. */
static void versioned_setup(int socket)
{
    unsigned char setup[1024], *at = setup;
    bool traits = peer_newer(4, 4, 5, 10, 0, 0);
    put32(&at, 120); put16(&at, 20);
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
    put32(&size_at, (uint32_t)(at - setup) + 120);
    memset(at, 0, 120); memcpy(at, "Peer MOTD: welcome Carol", sizeof("Peer MOTD: welcome Carol")); at += 120;
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
    put32(&at, 0); put32(&at, point_buy ? 3 : 2);
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
    SDL_Delay(100);
    no_final_request(socket); /* Name acceptance is not final play. */
    guide_to_mode();
    SDL_Delay(100);
    no_final_request(socket); /* Committed stats alone do not complete birth. */
    push_key(SDLK_RETURN, 0);
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
    const unsigned char reply[] = {PKT_REPLY, PKT_PLAY, PKT_SUCCESS};
    if (strcmp(peer->mode, "waiting")) {
        assert(send(socket, reply, 2, 0) == 2);
        SDL_Delay(100);
        no_final_request(socket);
    }
    push_presentation_events();
    /* Partial replies keep the startup flow waiting; never insert another
     * packet between fragments of this authoritative three-byte reply. */
    if (!strcmp(peer->mode, "ack")) {
        pthread_mutex_lock(&observations);
        acknowledged = true;
        puts("controlled peer: authoritative acknowledgement = 02 03 01");
        assert(send(socket, reply + 2, 1, 0) == 1);
        pthread_mutex_unlock(&observations);
    }
    SDL_Delay(150);
    no_final_request(socket);
    char ack_path[4096];
    assert(snprintf(ack_path, sizeof(ack_path), "%s.reply", evidence_path) < (int)sizeof(ack_path));
    FILE *ack = fopen(ack_path, "wb");
    size_t reply_size = !strcmp(peer->mode, "waiting") ? 0 :
                        !strcmp(peer->mode, "partial") ? 2 : 3;
    assert(ack && fwrite(reply, 1, reply_size, ack) == reply_size);
    assert(!fclose(ack));
    pthread_mutex_lock(&observations);
    assert(guide_frames > 0);
    if (!strcmp(peer->mode, "ack")) {
        assert(skip ? (handoff_frames > 0 && motd_frames == 0) :
                      (motd_frames > 0 && handoff_frames == 0));
    } else assert(motd_frames == 0 && handoff_frames == 0);
    pthread_mutex_unlock(&observations);
    if (!strcmp(peer->mode, "ack") && !skip) {
        push_key(SDLK_A, 0); /* baseline: any key, including letters */
        SDL_Delay(100);
        pthread_mutex_lock(&observations);
        assert(handoff_frames > 0);
        pthread_mutex_unlock(&observations);
    }
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
    versioned = true;
    point_buy = getenv("SV_GUIDE_POINT") != NULL;
    skip = getenv("SV_MOTD_SKIP") != NULL;
    capture_prefix = argv[5];
    if (argc == 7) assert(sscanf(argv[6], "%d.%d.%d.%d.%d.%d", &peer_version[0],
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
        .skip_motd = skip, .real_name = "PLAYER", .outcome = &outcome});
    assert(!pthread_join(worker, NULL));
    close(peer.listener);
    assert(outcome.birth_placeholder_open_count == (point_buy ? 6u : 5u));
    assert(outcome.birth_placeholder_close_count == (point_buy ? 6u : 5u));
    assert(outcome.birth_placeholder_owner_mask == ((1u << SV_BIRTH_CLASS) |
        (1u << SV_BIRTH_RACE) | (1u << SV_BIRTH_TRAIT) | (1u << SV_BIRTH_BODY) |
        (1u << SV_BIRTH_MODE) | (point_buy ? (1u << SV_BIRTH_STATS) : 0)));
    assert(outcome.birth_placeholder_draft_preserved);
    assert(!strcmp(outcome.birth_placeholder_text, "The guide is in development"));
    assert(outcome.birth_mode_ready && outcome.birth_stats_confirmed);
    assert(outcome.phase == (!strcmp(argv[3], "ack") ?
           SV_PREGAME_LIVE_HANDOFF : SV_PREGAME_PLAY_PENDING));
    if (!strcmp(argv[3], "ack")) assert(result == 0);
    else assert(result != 0);
    assert(outcome.selected_character[0] && !strcmp(outcome.selected_character, "Carol"));
    assert(peer.interleaved_pongs == 1);
    assert(outcome.birth_sex == SV_BIRTH_SEX_MALE && outcome.birth_race == 2 &&
           outcome.birth_class == 2 && outcome.birth_trait ==
           (peer_newer(4, 4, 5, 10, 0, 0) ? 2 : SV_BIRTH_TRAIT_NONE) &&
           outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    static const int16_t expected_stats[] = {0, 1, 2, 3, 4, 5};
    static const int16_t point_stats[] = {11, 10, 10, 10, 10, 10};
    assert(!memcmp(outcome.birth_committed_stats,
        point_buy ? point_stats : expected_stats, sizeof(expected_stats)));

    printf("SV-B-024 %s captured %zu handshake bytes; phase=%s character=%s generation=%llu\n",
           argv[3], expected_size, sv_pregame_phase_name(outcome.phase),
           outcome.selected_character, (unsigned long long)outcome.generation);
    return 0;
}
