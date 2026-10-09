/* Reuse the controlled contact/setup peer and native gestures from SV-B-023.
 * Its original main remains compiled, but this executable runs only this slice. */
#define main sv_birth_choices_main
#include "name-editor-submit.c"
#undef main
#include <poll.h>
#include <pthread.h>

static unsigned char expected_play[32768], captured_play[32768];
static size_t expected_size;
static const char *evidence_path;

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
    const uint32_t version[] = {4, 9, 4, 0, 0, 0};
    for (unsigned i = 0; i < 6; ++i) put32(&at, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == sizeof(contact));
    assert(exact(socket, number, 1) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && field(socket, text, sizeof(text)));
    const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == sizeof(verified));
    send_setup(socket, "complete");
    assert(exact(socket, number, 8));
    send_overview(socket, "complete");
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
    push_to_mode();
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
        assert(send(socket, reply + 2, 1, 0) == 1);
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
    SDL_Event quit = {.type = SDL_EVENT_QUIT};
    assert(SDL_PushEvent(&quit));
    unsigned char closed;
    assert(recv(socket, &closed, 1, 0) == 0);
    close(socket);
    return NULL;
}
int main(int argc, char **argv)
{
    assert(argc == 6);
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
        .real_name = "PLAYER", .outcome = &outcome});
    assert(!pthread_join(worker, NULL));
    close(peer.listener);
    assert(outcome.birth_mode_ready && outcome.birth_stats_confirmed);
    assert(outcome.phase == (!strcmp(argv[3], "ack") ?
           SV_PREGAME_PLAY_ACKNOWLEDGED : SV_PREGAME_PLAY_PENDING));
    if (!strcmp(argv[3], "ack")) assert(result == 0);
    else assert(result != 0);
    assert(outcome.selected_character[0] && !strcmp(outcome.selected_character, "Carol"));
    assert(peer.interleaved_pongs == 1);
    assert(outcome.birth_sex == SV_BIRTH_SEX_MALE && outcome.birth_race == 2 &&
           outcome.birth_class == 2 && outcome.birth_trait == 2 &&
           outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    static const int16_t expected_stats[] = {0, 1, 2, 3, 4, 5};
    assert(!memcmp(outcome.birth_committed_stats, expected_stats, sizeof(expected_stats)));

    printf("SV-B-024 %s captured %zu handshake bytes; phase=%s character=%s generation=%llu\n",
           argv[3], expected_size, sv_pregame_phase_name(outcome.phase),
           outcome.selected_character, (unsigned long long)outcome.generation);
    return 0;
}
