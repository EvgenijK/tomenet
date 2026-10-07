#include "endpoint-run.h"
#include "../../src/common/pack.h"
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int listener;
    bool request_exact;
    const char *mode;
} Peer;

static bool exact(int socket, void *out, size_t size)
{
    unsigned char *bytes = out;
    while (size) {
        ssize_t count = recv(socket, bytes, size, 0);
        if (count <= 0) return false;
        bytes += count;
        size -= (size_t)count;
    }
    return true;
}

static bool field(int socket, char *out, size_t capacity)
{
    size_t at = 0;
    do {
        if (at + 1 >= capacity || !exact(socket, out + at, 1)) return false;
    } while (out[at++]);
    return true;
}

static void put16(unsigned char **at, uint16_t value)
{
    *(*at)++ = (unsigned char)(value >> 8);
    *(*at)++ = (unsigned char)value;
}

static void put32(unsigned char **at, uint32_t value)
{
    *(*at)++ = (unsigned char)(value >> 24);
    *(*at)++ = (unsigned char)(value >> 16);
    *(*at)++ = (unsigned char)(value >> 8);
    *(*at)++ = (unsigned char)value;
}

static void push_key(SDL_Keycode key, SDL_Keymod mod)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = key;
    event.key.mod = mod;
    switch (key) {
    case SDLK_N: event.key.scancode = SDL_SCANCODE_N; break;
    case SDLK_L: event.key.scancode = SDL_SCANCODE_L; break;
    case SDLK_BACKSPACE: event.key.scancode = SDL_SCANCODE_BACKSPACE; break;
    default: event.key.scancode = SDL_SCANCODE_RETURN; break;
    }
    assert(SDL_PushEvent(&event));
}

static void push_text(const char *text)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = text;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_select_all(void)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.clicks = 2;
    event.button.x = 220.0f;
    event.button.y = 690.0f;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_sex(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 90.0f;
    event.button.y = y;
    event.button.clicks = clicks;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_class(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 90.0f;
    event.button.y = y;
    event.button.clicks = clicks;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_race(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 90.0f;
    event.button.y = y;
    event.button.clicks = clicks;
    assert(SDL_PushEvent(&event));
}

static void send_setup(int socket)
{
    unsigned char setup[256], *at = setup;
    put32(&at, 0);
    put16(&at, 20);
    *at++ = 3;
    *at++ = 4;
    *at++ = 1;
    unsigned char *size_at = at;
    at += 4;

    static const char *races[] = {"Human", "Elf", "Dwarf"};
    static const uint32_t race_choices[] = {
        (1u << 0) | (1u << 2), 1u << 0, 1u << 2
    };
    for (size_t race = 0; race < sizeof(races) / sizeof(races[0]); ++race) {
        memset(at, 50, 6);
        at += 6;
        const char *title = races[race];
        memcpy(at, title, strlen(title) + 1);
        at += strlen(title) + 1;
        put32(&at, race_choices[race]);
    }
    static const char *classes[] = {"Warrior", "0 Hidden", "Mage", "Orphan"};
    for (size_t i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i) {
        memset(at, 50, 6);
        at += 6;
        memcpy(at, classes[i], strlen(classes[i]) + 1);
        at += strlen(classes[i]) + 1;
        memset(at, 0, 6);
        at += 6;
    }
    static const char trait[] = "Common";
    memcpy(at, trait, sizeof(trait));
    at += sizeof(trait);
    put32(&at, 0x7u);
    unsigned char *size_writer = size_at;
    put32(&size_writer, (uint32_t)(at - setup));
    assert(send(socket, setup, (size_t)(at - setup), 0) == (ssize_t)(at - setup));
}

static void send_overview(int socket)
{
    unsigned char response[64], *at = response;
    *at++ = PKT_SERVERDETAILS;
    put32(&at, 2u | (1u << 8) | (1u << 16));
    put32(&at, 0);
    put32(&at, 0);
    put32(&at, 0xc0);
    *at++ = PKT_LOGIN;
    put16(&at, 0);
    *at++ = 0;
    *at++ = 0;
    put16(&at, 0);
    put16(&at, 0);
    put16(&at, 0);
    *at++ = 0;
    assert(send(socket, response, (size_t)(at - response), 0) ==
           (ssize_t)(at - response));
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
    int socket = accept(peer->listener, NULL, NULL);
    assert(socket >= 0);
    struct timeval timeout = {.tv_sec = 8};
    assert(!setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));

    unsigned char number[40];
    char text[80];
    assert(exact(socket, number, 4) && field(socket, text, sizeof(text)) &&
           exact(socket, number + 4, 3) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && exact(socket, number + 7, 26));
    unsigned char contact[34], *at = contact;
    *at++ = 255;
    *at++ = 0;
    put32(&at, 0);
    put32(&at, 2);
    const uint32_t version[6] = {4, 9, 4, 0, 0, 0};
    for (size_t i = 0; i < 6; ++i) put32(&at, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify_type;
    assert(exact(socket, &verify_type, 1) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && field(socket, text, sizeof(text)));
    static const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));
    send_setup(socket);

    unsigned char login_start[8];
    assert(exact(socket, login_start, sizeof(login_start)));
    send_overview(socket);
    SDL_Delay(200);
    push_key(SDLK_N, SDL_KMOD_SHIFT);
    SDL_Delay(80);
    push_key(SDLK_RETURN, 0);
    SDL_Delay(80);
    push_text("abcdefghijklmnop");
    push_mouse_select_all();
    push_text("\xc3\xa9ve@");
    push_key(SDLK_BACKSPACE, 0);
    push_mouse_select_all();
    assert(SDL_SetClipboardText("+carol$"));
    push_key(SDLK_L, SDL_KMOD_CTRL);
    push_key(SDLK_RETURN, 0);

    unsigned char request[8];
    assert(exact(socket, request, sizeof(request)));
    static const unsigned char expected[] = {
        PKT_LOGIN, 'C', 'a', 'r', 'o', 'l', '$', 0
    };
    peer->request_exact = !memcmp(request, expected, sizeof(expected));
    SDL_Delay(150); /* A sent name alone must remain pending. */
    const unsigned char permitted = 0;
    assert(send(socket, &permitted, 1, 0) == 1);
    SDL_Delay(150);
    if (!strcmp(peer->mode, "race-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "race-ctrl-quit")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, SDL_KMOD_CTRL);
    } else if (!strcmp(peer->mode, "race-dna") ||
        !strcmp(peer->mode, "race-dna-incompatible") ||
        !strcmp(peer->mode, "race-dna-stale") ||
        !strcmp(peer->mode, "race-dna-missing")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "race-random")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "race-navigate")) {
        push_key(SDLK_RETURN, 0);
        push_mouse_class(380.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_mouse_race(380.0f, 1);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_LEFT, 0);
        push_mouse_race(420.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "class-navigate")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Z, 0);
        push_mouse_class(380.0f, 1);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "class-mouse")) {
        push_key(SDLK_RETURN, 0);
        push_mouse_class(380.0f, 2);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "class-random")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "class-dna") ||
               !strcmp(peer->mode, "class-dna-hidden") ||
               !strcmp(peer->mode, "class-dna-incompatible") ||
               !strcmp(peer->mode, "class-dna-stale")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "class-back")) {
        push_key(SDLK_RETURN, 0);
        push_mouse_class(380.0f, 1);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "class-ctrl-quit")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, SDL_KMOD_CTRL);
    } else if (!strcmp(peer->mode, "navigate")) {
        push_key(SDLK_X, 0);
        push_mouse_sex(360.0f, 1);
        push_mouse_sex(310.0f, 1);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
    } else if (!strcmp(peer->mode, "mouse")) {
        push_mouse_sex(360.0f, 2);
    } else if (!strcmp(peer->mode, "random")) {
        push_key(SDLK_ASTERISK, 0);
    } else if (!strcmp(peer->mode, "dna") || !strcmp(peer->mode, "dna-missing")) {
        push_key(SDLK_HASH, 0);
    } else if (!strcmp(peer->mode, "ctrl-quit")) {
        push_key(SDLK_Q, SDL_KMOD_CTRL);
    } else {
        assert(!strcmp(peer->mode, "quit"));
        push_key(SDLK_Q, 0);
    }
    SDL_Delay(100);
    if (!strcmp(peer->mode, "navigate") || !strcmp(peer->mode, "mouse") ||
        !strcmp(peer->mode, "random") ||
        !strcmp(peer->mode, "dna") || !strcmp(peer->mode, "dna-missing"))
        push_key(SDLK_Q, 0);
    unsigned char closed;
    assert(recv(socket, &closed, 1, 0) == 0);
    close(socket);
    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0), .mode = argv[3]};
    assert(peer.listener >= 0);
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 1));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    SDL_Thread *worker = SDL_CreateThread(peer_run, "sv-b022-name-peer", &peer);
    assert(worker);

    SvEndpointOutcome outcome = {0};
    int result = sv_endpoint_run((SvEndpointOptions){
        .root = argv[1], .library = argv[2], .server = "127.0.0.1",
        .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port), .account = "Test", .password = "pw",
        .real_name = "PLAYER", .outcome = &outcome,
        .birth_dna_available = !strcmp(argv[3], "dna"),
        .birth_dna_sex = SV_BIRTH_SEX_FEMALE,
        .birth_dna_class_available = !strncmp(argv[3], "class-dna", 9),
        .birth_dna_class = !strcmp(argv[3], "class-dna") ? 2 :
            !strcmp(argv[3], "class-dna-hidden") ? 1 :
            !strcmp(argv[3], "class-dna-incompatible") ? 3 : 99,
        .birth_dna_race_available = !strncmp(argv[3], "race-dna", 8) &&
            strcmp(argv[3], "race-dna-missing"),
        .birth_dna_race = !strcmp(argv[3], "race-dna") ? 2 :
            !strcmp(argv[3], "race-dna-incompatible") ? 1 : 99
    });
    SDL_WaitThread(worker, NULL);
    close(peer.listener);

    assert(result == 0 && peer.request_exact);
    assert(outcome.name_submitted);
    assert(outcome.random_name_suggested);
    assert(outcome.name_submit_phase == SV_PREGAME_CHARACTER_PENDING);
    assert(outcome.phase == SV_PREGAME_BIRTH);
    assert(outcome.birth_entered);
    assert(outcome.birth_generation == outcome.generation);
    assert(outcome.birth_legal_class_count == 2);
    assert(outcome.birth_legal_classes[0] == 0);
    assert(outcome.birth_legal_classes[1] == 2);
    if (!strcmp(argv[3], "race-back")) {
        assert(outcome.birth_class_confirmed && outcome.birth_class == 2);
        assert(!outcome.birth_race_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(outcome.birth_highlighted_class == 2);
        assert(outcome.birth_highlighted_race == 2);
        assert(outcome.birth_race == SV_BIRTH_RACE_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "race-ctrl-quit")) {
        assert(outcome.birth_owner == SV_BIRTH_RACE && !outcome.birth_race_confirmed);
        assert(outcome.birth_highlighted_race == 0 && outcome.birth_quit);
    } else if (!strcmp(argv[3], "race-dna")) {
        assert(outcome.birth_race_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(outcome.birth_race_dna_used && outcome.birth_race == 2);
    } else if (!strncmp(argv[3], "race-dna-", 9)) {
        assert(!outcome.birth_race_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(!outcome.birth_race_dna_used && outcome.birth_illegal_ignored);
        assert(outcome.birth_class == 2);
        assert(outcome.birth_highlighted_race == 0);
        assert(outcome.birth_race == SV_BIRTH_RACE_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "race-random")) {
        assert(outcome.birth_class_confirmed && outcome.birth_class == 2);
        assert(outcome.birth_race_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(outcome.birth_race_random_used);
        assert(outcome.birth_race == 0 || outcome.birth_race == 2);
    } else if (!strcmp(argv[3], "race-navigate")) {
        assert(outcome.birth_class_confirmed && outcome.birth_class == 2);
        assert(outcome.birth_race_count == 3);
        assert(outcome.birth_legal_race_count == 2);
        assert(outcome.birth_legal_races[0] == 0);
        assert(outcome.birth_legal_races[1] == 2);
        assert(outcome.birth_race_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(outcome.birth_race == 2 && outcome.birth_highlighted_race == 2);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "class-navigate")) {
        assert(outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(outcome.birth_class == 2 && outcome.birth_highlighted_class == 2);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "class-mouse")) {
        assert(outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(outcome.birth_class == 2 && outcome.birth_highlighted_class == 2);
        assert(outcome.birth_mouse_navigated && outcome.birth_quit);
    } else if (!strcmp(argv[3], "class-random")) {
        assert(outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(outcome.birth_class_random_used);
        assert(outcome.birth_class == 0 || outcome.birth_class == 2);
    } else if (!strcmp(argv[3], "class-dna")) {
        assert(outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(outcome.birth_class_dna_used && outcome.birth_class == 2);
    } else if (!strcmp(argv[3], "class-back")) {
        assert(!outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_SEX);
        assert(outcome.birth_highlighted_class == 2);
        assert(outcome.birth_class == SV_BIRTH_CLASS_NONE && outcome.birth_quit);
    } else if (!strncmp(argv[3], "class-dna-", 10)) {
        assert(!outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(!outcome.birth_class_dna_used && outcome.birth_illegal_ignored);
        assert(outcome.birth_highlighted_class == 0 && outcome.birth_quit);
    } else if (!strcmp(argv[3], "class-ctrl-quit")) {
        assert(!outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(outcome.birth_quit);
    } else if (!strcmp(argv[3], "navigate")) {
        assert(outcome.birth_sex_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "mouse")) {
        assert(outcome.birth_sex_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_mouse_navigated && outcome.birth_quit);
    } else if (!strcmp(argv[3], "random")) {
        assert(outcome.birth_sex_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(outcome.birth_random_used);
        assert(outcome.birth_sex == SV_BIRTH_SEX_MALE ||
               outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
    } else if (!strcmp(argv[3], "dna")) {
        assert(outcome.birth_sex_confirmed && outcome.birth_owner == SV_BIRTH_CLASS);
        assert(outcome.birth_dna_used && outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
    } else {
        assert(!outcome.birth_sex_confirmed);
        assert(outcome.birth_owner == SV_BIRTH_SEX && outcome.birth_quit);
        if (!strcmp(argv[3], "dna-missing")) {
            assert(outcome.birth_illegal_ignored && !outcome.birth_dna_used);
            assert(outcome.birth_highlighted_sex == SV_BIRTH_SEX_MALE);
        }
    }
    assert(!strcmp(outcome.selected_character, "Carol$"));
    assert(outcome.reincarnation_requested);
    assert(outcome.name_maximum_draft_bytes == 15);
    assert(outcome.name_input_truncated);
    assert(outcome.name_input_method_used);
    assert(outcome.name_keyboard_edited);
    assert(outcome.name_mouse_selected);
    assert(outcome.name_paste_used);
    return 0;
}
