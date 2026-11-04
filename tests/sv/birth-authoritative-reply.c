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
static unsigned retry_attempt;
static int retired_socket = -1;

static void no_final_request(int socket)
{
    struct pollfd ready = {.fd = socket, .events = POLLIN};
    assert(poll(&ready, 1, 120) == 0);
}
static void *complete_peer(void *opaque)
{
    Peer *peer = opaque;
    bool retrying = !strcmp(peer->mode, "ack-retry");
    unsigned attempt = retry_attempt++;
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
    const int version[6] = {4, 9, 4, 0, 0, 0};
    for (unsigned i = 0; i < 6; ++i) put32(&at, (uint32_t)version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == sizeof(contact));
    assert(exact(socket, number, 1) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && field(socket, text, sizeof(text)));
    const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == sizeof(verified));
    send_setup(socket, "complete");
    size_t login_size = 8;
    assert(exact(socket, number, login_size));
    assert(number[0] == PKT_LOGIN && number[1] == 0);
    send_overview(socket, "complete");
    SDL_Delay(200);
    if (retrying && attempt) {
        /* Results still addressed to the closed transport cannot enter its replacement. */
        const unsigned char old_ack[] = {2, 3, 122};
        (void)send(retired_socket, old_ack, sizeof(old_ack), MSG_NOSIGNAL);
        close(retired_socket); retired_socket = -1;
        SDL_Event stale = {.type = SDL_EVENT_TEXT_INPUT};
        stale.common.timestamp = 1; stale.text.text = "Ghost";
        assert(SDL_PushEvent(&stale));
        push_stale_key(SDLK_RETURN);
        SDL_Delay(80);
        no_final_request(socket); /* Neither stale input nor old reply completes a request. */
        push_key(SDLK_RETURN, 0); /* Restored current-generation Carol draft. */
    } else {
        push_key(SDLK_N, SDL_KMOD_SHIFT);
        SDL_Delay(80);
        push_text("Carol"); push_key(SDLK_RETURN, 0);
    }
    assert(exact(socket, number, 7));
    assert(number[0] == PKT_LOGIN && !memcmp(number + 1, "Carol\0", 6));
    no_final_request(socket);
    if (retrying && !attempt) {
        const unsigned char rejected_name[] = {1, 2, 3, 122};
        assert(send(socket, rejected_name, sizeof(rejected_name), 0) == sizeof(rejected_name));
        SDL_Delay(150);
        retired_socket = socket;
        push_key(SDLK_R, 0);
        return complete_peer(peer);
    }
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
    /* Baseline Net_start / Receive_reply literal wire fixtures. */
    const unsigned char rejected[] = {2, 3, 0};
    const unsigned char peer_quit[] = {4, 'B', 'y', 'e', 0};
    const unsigned char wrong_type[] = {2, 1, 122};
    const unsigned char wrong_packet[] = {255};
    const unsigned char success[] = {2, 3, 122};
    const unsigned char rejected_trailing[] = {2, 3, 0, 2, 3, 122};
    const unsigned char ack_quit[] = {2, 3, 122, 4, 'B', 'y', 'e', 0};
    unsigned char malformed_quit[258];
    memset(malformed_quit, 'X', sizeof(malformed_quit)); malformed_quit[0] = 4;
    const unsigned char *reply = rejected;
    size_t reply_size = sizeof(rejected);
    bool successful = !strncmp(peer->mode, "ack", 3) && strcmp(peer->mode, "ack-quit");
    bool partial = !strncmp(peer->mode, "partial", 7);
    bool truncated = !strncmp(peer->mode, "truncated", 9);
    if (!strcmp(peer->mode, "peer-quit")) { reply = peer_quit; reply_size = sizeof(peer_quit); }
    if (!strcmp(peer->mode, "wrong-type")) { reply = wrong_type; reply_size = sizeof(wrong_type); }
    if (!strcmp(peer->mode, "wrong-packet")) { reply = wrong_packet; reply_size = sizeof(wrong_packet); }
    if (successful || partial || truncated) { reply = success; reply_size = sizeof(success); }
    if (!strcmp(peer->mode, "rejected-trailing")) { reply = rejected_trailing; reply_size = sizeof(rejected_trailing); }
    if (!strcmp(peer->mode, "ack-quit")) { reply = ack_quit; reply_size = sizeof(ack_quit); }
    if (!strcmp(peer->mode, "malformed-quit")) { reply = malformed_quit; reply_size = sizeof(malformed_quit); }
    if (partial || truncated || !strcmp(peer->mode, "ack-1") || !strcmp(peer->mode, "ack-2")) {
        size_t split = (size_t)(peer->mode[strlen(peer->mode) - 1] - '0');
        assert(split == 1 || split == 2);
        assert(send(socket, reply, split, 0) == (ssize_t)split);
        push_presentation_events();
        SDL_Delay(100);
        no_final_request(socket);
        if (successful)
            assert(send(socket, reply + split, reply_size - split, 0) == (ssize_t)(reply_size - split));
    } else if (!strcmp(peer->mode, "ack-trailing")) {
        /* One write: keepalive + SFLAGS + ping + ack + SFLAGS + ping.
         * Distinct pong sequences prove decoder order and retained remainder. */
        unsigned char packets[80], *out = packets;
        *out++ = 13; /* PKT_KEEPALIVE */
        *out++ = 213;
        for (unsigned i = 0; i < 4; ++i) put32(&out, 10 + i);
        unsigned char ping[15] = {PKT_PING, 0}; ping[5] = 41;
        memcpy(out, ping, sizeof(ping)); out += sizeof(ping);
        memcpy(out, success, sizeof(success)); out += sizeof(success);
        *out++ = 213;
        for (unsigned i = 0; i < 4; ++i) put32(&out, 20 + i);
        ping[5] = 42;
        memcpy(out, ping, sizeof(ping)); out += sizeof(ping);
        assert(send(socket, packets, (size_t)(out - packets), 0) == out - packets);
        unsigned char pongs[30];
        assert(exact(socket, pongs, sizeof(pongs)));
        assert(pongs[0] == PKT_PING && pongs[1] == 1 && pongs[5] == 41);
        assert(pongs[15] == PKT_PING && pongs[16] == 1 && pongs[20] == 42);
        peer->interleaved_pongs += 2;
        no_final_request(socket);
    } else if (strcmp(peer->mode, "disconnect"))
        assert(send(socket, reply, reply_size, 0) == (ssize_t)reply_size);
    if (!strcmp(peer->mode, "disconnect") || truncated) assert(!shutdown(socket, SHUT_WR));
    SDL_Delay(150);
    if (!successful && !partial) {
        push_key(SDLK_R, 0); /* Play failure exits this session; it cannot retry name/login. */
        push_key(SDLK_RETURN, 0);
        SDL_Delay(80);
        struct pollfd retry = {.fd = peer->listener, .events = POLLIN};
        assert(poll(&retry, 1, 100) == 0);
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
        .skip_motd = true, .real_name = "PLAYER", .outcome = &outcome});
    assert(!pthread_join(worker, NULL));
    close(peer.listener);
    assert(outcome.birth_mode_ready && outcome.birth_stats_confirmed);
    bool successful = !strncmp(argv[3], "ack", 3) && strcmp(argv[3], "ack-quit");
    bool partial = !strncmp(argv[3], "partial", 7);
    bool truncated = !strncmp(argv[3], "truncated", 9);
    bool disconnected = !strcmp(argv[3], "ack-quit") || !strcmp(argv[3], "peer-quit") || !strcmp(argv[3], "disconnect") || truncated;
    assert(outcome.phase == (successful ? SV_PREGAME_LIVE_HANDOFF :
        partial ? SV_PREGAME_PLAY_PENDING : disconnected ? SV_PREGAME_DISCONNECTED : SV_PREGAME_FAILED));
    assert((result == 0) == successful);
    if (!strcmp(argv[3], "ack-retry")) {
        assert(outcome.name_rejected && outcome.name_recovery_started);
        assert(outcome.prior_generation && outcome.generation != outcome.prior_generation);
        assert(outcome.name_rejected_generation == outcome.prior_generation);
        assert(outcome.name_recovery_generation == outcome.generation);
        assert(outcome.birth_generation == outcome.generation);
    } else assert(!outcome.name_rejected && !outcome.name_recovery_started);
    const char *reason = (!strcmp(argv[3], "ack-quit") || !strcmp(argv[3], "peer-quit")) ? "Bye" :
        !strcmp(argv[3], "malformed-quit") ? "Session closed: packet decode failed" :
        (!strcmp(argv[3], "disconnect") || truncated) ? "Error reading play reply" :
        !strcmp(argv[3], "wrong-type") ? "Can't receive reply packet after play" :
        !strcmp(argv[3], "wrong-packet") ? "Not a reply packet after play (255)" :
        (successful || partial) ? "" : "Start play not allowed (0)";
    assert(!strcmp(outcome.reason, reason));
    assert(outcome.birth_closed && !outcome.birth_active);
    if (!strcmp(argv[3], "ack-trailing")) {
        const uint32_t flags[] = {23, 22, 21, 20};
        assert(!memcmp(outcome.server_flags, flags, sizeof(flags)));
    }
    assert(outcome.selected_character[0] && !strcmp(outcome.selected_character, "Carol"));
    assert(peer.interleaved_pongs == (!strcmp(argv[3], "ack-trailing") ? 3u : 1u));
    assert(outcome.birth_sex == SV_BIRTH_SEX_MALE && outcome.birth_race == 2 &&
           outcome.birth_class == 2 && outcome.birth_trait ==
           2 &&
           outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    static const int16_t expected_stats[] = {0, 1, 2, 3, 4, 5};
    assert(!memcmp(outcome.birth_committed_stats, expected_stats, sizeof(expected_stats)));

    printf("SV-B-024 %s captured %zu handshake bytes; phase=%s character=%s generation=%llu\n",
           argv[3], expected_size, sv_pregame_phase_name(outcome.phase),
           outcome.selected_character, (unsigned long long)outcome.generation);
    return 0;
}
