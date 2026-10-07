#include "endpoint-run.h"
#include "../../src/common/pack.h"
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int listener;
    bool request_exact;
    unsigned interleaved_pongs;
    unsigned attempt;
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

static void push_stale_key(SDL_Keycode key)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.common.timestamp = 1;
    event.key.key = key;
    event.key.scancode = key == SDLK_BACKSPACE ? SDL_SCANCODE_BACKSPACE :
        key == SDLK_Q ? SDL_SCANCODE_Q : SDL_SCANCODE_RETURN;
    assert(SDL_PushEvent(&event));
}

static void push_presentation_events(void)
{
    const Uint32 types[] = {SDL_EVENT_WINDOW_FOCUS_LOST,
                            SDL_EVENT_WINDOW_FOCUS_GAINED,
                            SDL_EVENT_WINDOW_RESIZED};
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
        SDL_Event event = {0};
        event.type = types[i];
        event.window.data1 = 1024;
        event.window.data2 = 768;
        assert(SDL_PushEvent(&event));
    }
}

static void interleave_ping(Peer *peer, int socket, unsigned sequence)
{
    unsigned char ping[15] = {PKT_PING, 0};
    ping[5] = (unsigned char)sequence;
    assert(send(socket, ping, sizeof(ping), 0) == (ssize_t)sizeof(ping));
    unsigned char reply[15];
    assert(exact(socket, reply, sizeof(reply)));
    assert(reply[0] == PKT_PING && reply[1] == 1 &&
           !memcmp(reply + 2, ping + 2, sizeof(ping) - 2));
    ++peer->interleaved_pongs;
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

static void push_mouse_trait(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 90.0f;
    event.button.y = y;
    event.button.clicks = clicks;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_body(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.clicks = clicks;
    event.button.x = 90.0f;
    event.button.y = y;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_stats(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.clicks = clicks;
    event.button.x = 90.0f;
    event.button.y = y;
    assert(SDL_PushEvent(&event));
}

static void push_mouse_mode(float y, unsigned clicks)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.clicks = clicks;
    event.button.x = 90.0f;
    event.button.y = y;
    assert(SDL_PushEvent(&event));
}

static void push_guide_round_trip(void)
{
    push_key(SDLK_QUESTION, 0);
    SDL_Delay(80);
    push_key(SDLK_ESCAPE, 0);
    SDL_Delay(40);
}

static void push_guide_all_owners(void)
{
    push_key(SDLK_F, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
#if SV_CLASS_BEFORE_RACE
    push_key(SDLK_B, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_C, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
#else
    push_key(SDLK_C, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
#endif
    push_key(SDLK_B, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_F, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
#if !SV_CLASS_BEFORE_RACE
    push_key(SDLK_B, 0);
    push_guide_round_trip();
    push_key(SDLK_RETURN, 0);
#endif
    push_guide_round_trip();
    for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
        push_key(key, 0);
        push_key(SDLK_RETURN, 0);
    }
    push_key(SDLK_G, 0);
    push_guide_round_trip();
    push_key(SDLK_Q, 0);
}

static void push_to_stats(void)
{
    push_key(SDLK_RETURN, 0);
#if SV_CLASS_BEFORE_RACE
    push_key(SDLK_B, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_C, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_B, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_F, 0);
    push_key(SDLK_RETURN, 0);
#else
    push_key(SDLK_C, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_B, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_F, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_RETURN, 0);
#endif
}

static void push_to_mode(void)
{
    push_to_stats();
    for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
        push_key(key, 0);
        push_key(SDLK_RETURN, 0);
    }
}

static void push_to_quit_owner(const char *owner)
{
    if (!strcmp(owner, "sex")) return;
    push_key(SDLK_RETURN, 0);
#if SV_CLASS_BEFORE_RACE
    if (!strcmp(owner, "class")) return;
    push_key(SDLK_RETURN, 0);
    if (!strcmp(owner, "race")) return;
#else
    if (!strcmp(owner, "race")) return;
#endif
    push_key(SDLK_RETURN, 0);
    if (!strcmp(owner, "trait")) return;
    push_key(SDLK_RETURN, 0);
    if (!strcmp(owner, "body")) return;
    push_key(SDLK_RETURN, 0);
#if !SV_CLASS_BEFORE_RACE
    if (!strcmp(owner, "class")) return;
    push_key(SDLK_RETURN, 0);
#endif
    if (!strcmp(owner, "stats")) return;
    assert(!strcmp(owner, "mode"));
    for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
        push_key(key, 0);
        push_key(SDLK_RETURN, 0);
    }
}

static bool quit_matrix_case(const char *mode, const char **owner, bool *control)
{
    if (!strncmp(mode, "quit-", 5)) {
        *owner = mode + 5;
        *control = false;
        return true;
    }
    if (!strncmp(mode, "ctrl-quit-", 10)) {
        *owner = mode + 10;
        *control = true;
        return true;
    }
    return false;
}

static SvBirthOwner named_owner(const char *owner)
{
    if (!strcmp(owner, "sex")) return SV_BIRTH_SEX;
    if (!strcmp(owner, "race")) return SV_BIRTH_RACE;
    if (!strcmp(owner, "trait")) return SV_BIRTH_TRAIT;
    if (!strcmp(owner, "class")) return SV_BIRTH_CLASS;
    if (!strcmp(owner, "body")) return SV_BIRTH_BODY;
    if (!strcmp(owner, "stats")) return SV_BIRTH_STATS;
    assert(!strcmp(owner, "mode"));
    return SV_BIRTH_MODE;
}

static bool owner_confirmed(const SvEndpointOutcome *outcome, SvBirthOwner owner)
{
    switch (owner) {
    case SV_BIRTH_SEX: return outcome->birth_sex_confirmed;
    case SV_BIRTH_RACE: return outcome->birth_race_confirmed;
    case SV_BIRTH_TRAIT: return outcome->birth_trait_confirmed;
    case SV_BIRTH_CLASS: return outcome->birth_class_confirmed;
    case SV_BIRTH_BODY: return outcome->birth_body_confirmed;
    case SV_BIRTH_STATS: return outcome->birth_stats_confirmed;
    case SV_BIRTH_MODE: return outcome->birth_mode_confirmed;
    default: assert(false); return true;
    }
}

static unsigned maia_race(void)
{
#ifdef ENABLE_KOBOLD
    return 17;
#else
    return 16;
#endif
}

static void push_to_pvp_maia_mode(void)
{
    push_key(SDLK_RETURN, 0);
#if SV_CLASS_BEFORE_RACE
    push_key(SDLK_RETURN, 0);
    push_mouse_race(340.0f + 40.0f * maia_race(), 1);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_F, 0);
    push_key(SDLK_RETURN, 0);
#else
    push_mouse_race(340.0f + 40.0f * maia_race(), 1);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_F, 0);
    push_key(SDLK_RETURN, 0);
    push_key(SDLK_RETURN, 0);
#endif
    for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
        push_key(key, 0);
        push_key(SDLK_RETURN, 0);
    }
}

static void send_setup(int socket, const char *mode)
{
    unsigned char setup[4096], *at = setup;
    put32(&at, 0);
    put16(&at, 20);
    if (!strcmp(mode, "mode-pvp-maia") ||
        !strcmp(mode, "mode-pvp-maia-back")) {
        unsigned races = maia_race() + 1;
        *at++ = (unsigned char)races;
        *at++ = 1;
        *at++ = 16;
        unsigned char *size_at = at;
        at += 4;
        for (unsigned race = 0; race < races; ++race) {
            memset(at, 50, 6);
            at += 6;
            char title[24];
            snprintf(title, sizeof(title), race == maia_race() ? "Maia" :
                     "Race %u", race);
            memcpy(at, title, strlen(title) + 1);
            at += strlen(title) + 1;
            put32(&at, 1u);
        }
        memset(at, 50, 6);
        at += 6;
        memcpy(at, "Warrior", 8);
        at += 8;
        memset(at, 0, 6);
        at += 6;
        for (unsigned trait = 0; trait < 16; ++trait) {
            char title[24];
            snprintf(title, sizeof(title), trait == 0 ? "N/A" :
                     trait == 14 ? "Enlightened" :
                     trait == 15 ? "Corrupted" : "Trait %u", trait);
            memcpy(at, title, strlen(title) + 1);
            at += strlen(title) + 1;
            put32(&at, trait == 0 ? 1u << maia_race() : 0u);
        }
        unsigned char *size_writer = size_at;
        put32(&size_writer, (uint32_t)(at - setup));
        assert(send(socket, setup, (size_t)(at - setup), 0) ==
               (ssize_t)(at - setup));
        return;
    }
    *at++ = 3;
    bool dummy_trait = !strncmp(mode, "trait-dummy-skip", 16) ||
        !strcmp(mode, "body-back-no-trait") ||
        !strcmp(mode, "alternate-class-back-no-trait-body") ||
        !strcmp(mode, "stats-back-no-trait-body");
    bool unavailable_trait = !strcmp(mode, "trait-unavailable-skip");
    *at++ = 4;
    *at++ = dummy_trait ? 1 : 4;
    unsigned char *size_at = at;
    at += 4;

    static const char *races[] = {"Human", "Elf", "Dwarf"};
    static const uint32_t race_choices[] = {
        (1u << 0) | (1u << 2), 1u << 0, 1u << 2
    };
    for (size_t race = 0; race < sizeof(races) / sizeof(races[0]); ++race) {
        memset(at, 50, 6);
        if (!strcmp(mode, "stats-point-lower-bound") && race == 2) at[0] = 44;
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
    static const char *traits[] = {"N/A", "Dragon", "Phoenix", "Elf-only"};
    static const uint32_t trait_choices[] = {
        0, (1u << 0) | (1u << 2), 1u << 2, 1u << 1
    };
    size_t trait_count = dummy_trait ? 1 : sizeof(traits) / sizeof(traits[0]);
    for (size_t i = 0; i < trait_count; ++i) {
        memcpy(at, traits[i], strlen(traits[i]) + 1);
        at += strlen(traits[i]) + 1;
        put32(&at, dummy_trait ? 0x7u : unavailable_trait ? 1u << 1 :
              trait_choices[i]);
    }
    unsigned char *size_writer = size_at;
    put32(&size_writer, (uint32_t)(at - setup));
    assert(send(socket, setup, (size_t)(at - setup), 0) == (ssize_t)(at - setup));
}

static void send_overview(int socket, const char *mode)
{
    unsigned char response[128], *at = response;
    *at++ = PKT_SERVERDETAILS;
    put32(&at, 2u | (1u << 8) | (1u << 16));
    put32(&at, 0);
    put32(&at, 0);
    put32(&at, !strcmp(mode, "mode-dedicated-unavailable") ? 0x40u :
          !strcmp(mode, "mode-rpg") ? 0xc1u :
          (!strcmp(mode, "mode-pvp-maia") ||
           !strcmp(mode, "mode-pvp-maia-back")) ? 0x2c0u : 0xc0u);
    if (!strcmp(mode, "mode-pvp-maia") ||
        !strcmp(mode, "mode-pvp-maia-back")) {
        *at++ = PKT_LOGIN;
        put16(&at, 0);
        *at++ = 0;
        memcpy(at, "Hero", 5);
        at += 5;
        put16(&at, 10);
        put16(&at, 0);
        put16(&at, 0);
        memcpy(at, "Home", 5);
        at += 5;
    }
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
    unsigned attempt = peer->attempt++;
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
    bool body_unavailable = !strcmp(peer->mode, "body-unavailable-skip") ||
        !strcmp(peer->mode, "alternate-class-back-no-body") ||
        !strcmp(peer->mode, "alternate-class-back-no-trait-body") ||
        !strcmp(peer->mode, "stats-back-no-trait-body");
    bool point_buy = !strncmp(peer->mode, "stats-point-", 12);
    put32(&at, 2u | (body_unavailable ? 0x10u : point_buy ? 1u : 0u));
    const uint32_t version[6] = {4, 9, 4, 0, 0, 0};
    for (size_t i = 0; i < 6; ++i) put32(&at, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify_type;
    assert(exact(socket, &verify_type, 1) && field(socket, text, sizeof(text)) &&
           field(socket, text, sizeof(text)) && field(socket, text, sizeof(text)));
    static const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));
    send_setup(socket, peer->mode);

    unsigned char login_start[8];
    assert(exact(socket, login_start, sizeof(login_start)));
    send_overview(socket, peer->mode);
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
    const char *quit_owner = NULL;
    bool control_quit = false;
    if (quit_matrix_case(peer->mode, &quit_owner, &control_quit)) {
        push_to_quit_owner(quit_owner);
        push_key(SDLK_Q, control_quit ? SDL_KMOD_CTRL : 0);
    } else if (!strcmp(peer->mode, "session-interleave")) {
        push_key(SDLK_F, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 1);
        push_presentation_events();
        push_mouse_sex(360.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 2);
        push_presentation_events();
        push_mouse_class(380.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 3);
        push_presentation_events();
        push_mouse_race(420.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 4);
        push_presentation_events();
        push_mouse_trait(380.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_F, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 5);
        push_presentation_events();
        push_mouse_body(380.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_A, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 6);
        push_presentation_events();
        push_mouse_stats(340.0f, 1);
        for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
            push_key(key, 0);
            push_key(SDLK_RETURN, 0);
        }
        push_key(SDLK_S, 0);
        SDL_Delay(50);
        interleave_ping(peer, socket, 7);
        push_presentation_events();
        push_mouse_mode(420.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "session-stale-input")) {
        push_stale_key(SDLK_RETURN);
        push_stale_key(SDLK_BACKSPACE);
        push_stale_key(SDLK_Q);
        push_stale_key((SDL_Keycode)'?');
        push_key(SDLK_F, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "session-disconnect")) {
        push_key(SDLK_F, 0);
        SDL_Delay(50);
        assert(!shutdown(socket, SHUT_WR));
        SDL_Delay(100);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
        push_key((SDL_Keycode)'?', 0);
        SDL_Event close = {.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED};
        assert(SDL_PushEvent(&close));
    } else if (!strcmp(peer->mode, "session-retry-generation") && !attempt) {
        assert(!shutdown(socket, SHUT_WR));
        SDL_Delay(100);
        push_key(SDLK_R, 0);
    } else if (!strcmp(peer->mode, "session-retry-generation")) {
        push_stale_key(SDLK_RETURN);
        push_stale_key(SDLK_BACKSPACE);
        push_stale_key(SDLK_Q);
        push_stale_key((SDL_Keycode)'?');
        push_key(SDLK_F, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "guide-all-owners")) {
        push_guide_all_owners();
    } else if (!strcmp(peer->mode, "mode-rpg")) {
        push_to_mode();
        push_key(SDLK_N, 0);
        push_key(SDLK_E, 0);
        push_key(SDLK_P, 0);
        push_key(SDLK_S, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-dedicated-iddc")) {
        push_to_mode();
        push_key(SDLK_I, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-dedicated-dna")) {
        push_to_mode();
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-pvp-maia")) {
        push_to_pvp_maia_mode();
        push_key(SDLK_P, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-pvp-maia-back")) {
        push_to_pvp_maia_mode();
        push_key(SDLK_P, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-back")) {
        push_to_mode();
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-random")) {
        push_to_mode();
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-ctrl-quit")) {
        push_to_mode();
        push_key(SDLK_Q, SDL_KMOD_CTRL);
    } else if (!strcmp(peer->mode, "mode-dedicated")) {
        push_to_mode();
        push_key(SDLK_N, 0);
        push_mouse_mode(460.0f, 1);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-dedicated-unavailable")) {
        push_to_mode();
        push_key(SDLK_P, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-dna") ||
        !strcmp(peer->mode, "mode-dna-missing")) {
        push_to_mode();
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "mode-ordinary-ready")) {
        push_to_mode();
        push_key(SDLK_H, 0); /* First-character Hellish is unavailable. */
        push_mouse_mode(380.0f, 1);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-order-confirm")) {
        push_to_stats();
        for (SDL_Keycode key = SDLK_A; key <= SDLK_F; ++key) {
            push_key(key, 0);
            push_key(SDLK_RETURN, 0);
        }
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-order-random")) {
        push_to_stats();
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-order-mouse")) {
        push_to_stats();
        for (unsigned i = 0; i < 6; ++i) push_mouse_stats(340.0f, 2);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-back") ||
               !strcmp(peer->mode, "stats-back-no-trait-body")) {
        push_to_stats();
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-quit") ||
               !strcmp(peer->mode, "stats-ctrl-quit")) {
        push_to_stats();
        push_key(SDLK_Q, !strcmp(peer->mode, "stats-ctrl-quit") ?
            SDL_KMOD_CTRL : 0);
    } else if (!strcmp(peer->mode, "stats-point-confirm-warning")) {
        push_to_stats();
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-point-confirm") ||
               !strcmp(peer->mode, "stats-point-mouse")) {
        push_to_stats();
        if (!strcmp(peer->mode, "stats-point-mouse")) {
            push_mouse_stats(340.0f, 2);
            push_mouse_stats(340.0f, 2);
        } else {
            push_key(SDLK_RETURN, 0);
            push_key(SDLK_RETURN, 0);
        }
        push_key(SDLK_Q, 0);
    } else if (!strncmp(peer->mode, "stats-point-dna", 15)) {
        push_to_stats();
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-point-lower-bound")) {
        push_to_stats();
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-point-navigate")) {
        push_to_stats();
        push_key(SDLK_Z, 0);
        push_mouse_stats(380.0f, 1);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "stats-order-navigate")) {
        push_to_stats();
        push_key(SDLK_Z, 0);
        push_mouse_stats(380.0f, 1);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else
#if !SV_CLASS_BEFORE_RACE
    if (!strcmp(peer->mode, "alternate-race-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-trait-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-body-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_F, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-class-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_F, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-class-back-no-trait-body")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-class-back-no-body")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-navigate")) {
        push_mouse_sex(360.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Z, 0);
        push_mouse_race(340.0f, 1);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RETURN, 0);
        push_mouse_trait(340.0f, 1);
        push_key(SDLK_RETURN, 0);
        push_mouse_body(380.0f, 1);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Z, 0);
        push_mouse_class(380.0f, 1);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-random")) {
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-dna")) {
        push_key(SDLK_HASH, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-class-dna-incompatible")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-mouse")) {
        push_mouse_sex(360.0f, 2);
        push_mouse_race(420.0f, 2);
        push_mouse_trait(380.0f, 2);
        push_mouse_body(380.0f, 2);
        push_mouse_class(340.0f, 2);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "alternate-ctrl-quit")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, SDL_KMOD_CTRL);
    } else if (!strcmp(peer->mode, "alternate-order")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_F, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else
#endif
    if (!strcmp(peer->mode, "body-mouse")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_mouse_body(380.0f, 2);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "body-quit") ||
               !strcmp(peer->mode, "body-ctrl-quit")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, !strcmp(peer->mode, "body-ctrl-quit") ?
            SDL_KMOD_CTRL : 0);
    } else if (!strcmp(peer->mode, "body-unavailable-skip")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "body-back-no-trait")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_F, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "body-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_F, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "body-dna") ||
        !strcmp(peer->mode, "body-dna-stale") ||
        !strcmp(peer->mode, "body-dna-missing")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "body-random")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "body-navigate")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Z, 0);
        push_key(SDLK_N, 0);
        push_mouse_body(380.0f, 1);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-quit") ||
               !strcmp(peer->mode, "trait-ctrl-quit")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, !strcmp(peer->mode, "trait-ctrl-quit") ?
            SDL_KMOD_CTRL : 0);
    } else if (!strcmp(peer->mode, "trait-mouse")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_mouse_trait(380.0f, 2);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-dummy-skip-back")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_BACKSPACE, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-dummy-skip") ||
               !strcmp(peer->mode, "trait-unavailable-skip")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-dna") ||
        !strcmp(peer->mode, "trait-dna-incompatible") ||
        !strcmp(peer->mode, "trait-dna-stale") ||
        !strcmp(peer->mode, "trait-dna-missing")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_HASH, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-random")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_ASTERISK, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "trait-navigate")) {
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_B, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_C, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Z, 0);
        push_mouse_trait(380.0f, 1);
        push_key(SDLK_RIGHT, 0);
        push_key(SDLK_LEFT, 0);
        push_key(SDLK_RETURN, 0);
        push_key(SDLK_Q, 0);
    } else if (!strcmp(peer->mode, "race-back")) {
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
    if (!strcmp(peer->mode, "session-retry-generation") && !attempt)
        return peer_run(opaque);
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
        .birth_dna_available = !strcmp(argv[3], "dna") ||
            !strcmp(argv[3], "alternate-dna"),
        .birth_dna_sex = SV_BIRTH_SEX_FEMALE,
        .birth_dna_class_available = !strncmp(argv[3], "class-dna", 9) ||
            !strcmp(argv[3], "alternate-dna") ||
            !strcmp(argv[3], "alternate-class-dna-incompatible"),
        .birth_dna_class = !strcmp(argv[3], "class-dna") ||
            !strcmp(argv[3], "alternate-dna") ? 2 :
            !strcmp(argv[3], "alternate-class-dna-incompatible") ? 0 :
            !strcmp(argv[3], "class-dna-hidden") ? 1 :
            !strcmp(argv[3], "class-dna-incompatible") ? 3 : 99,
        .birth_dna_race_available = (!strncmp(argv[3], "race-dna", 8) &&
            strcmp(argv[3], "race-dna-missing")) ||
            !strcmp(argv[3], "alternate-dna"),
        .birth_dna_race = !strcmp(argv[3], "race-dna") ||
            !strcmp(argv[3], "alternate-dna") ? 2 :
            !strcmp(argv[3], "race-dna-incompatible") ? 1 : 99,
        .birth_dna_trait_available = (!strncmp(argv[3], "trait-dna", 9) &&
            strcmp(argv[3], "trait-dna-missing")) ||
            !strcmp(argv[3], "alternate-dna"),
        .birth_dna_trait = !strcmp(argv[3], "trait-dna") ||
            !strcmp(argv[3], "alternate-dna") ? 2 :
            !strcmp(argv[3], "trait-dna-incompatible") ? 3 : 99,
        .birth_dna_body_available = (!strncmp(argv[3], "body-dna", 8) &&
            strcmp(argv[3], "body-dna-missing")) ||
            !strcmp(argv[3], "alternate-dna"),
        .birth_dna_body = !strcmp(argv[3], "body-dna") ||
            !strcmp(argv[3], "alternate-dna") ?
            SV_BIRTH_BODY_FRUIT_BAT : (SvBirthBody)99,
        .birth_dna_stats_available = !strcmp(argv[3], "stats-point-dna") ||
            !strcmp(argv[3], "stats-point-dna-over-budget") ||
            !strcmp(argv[3], "stats-point-dna-illegal") ||
            !strcmp(argv[3], "stats-point-dna-multiple-max"),
        .birth_dna_stats = {
            !strcmp(argv[3], "stats-point-dna-illegal") ? 18 : 17,
            !strcmp(argv[3], "stats-point-dna-over-budget") ? 16 :
                !strcmp(argv[3], "stats-point-dna-multiple-max") ? 17 : 14,
            !strcmp(argv[3], "stats-point-dna-over-budget") ? 16 :
                !strcmp(argv[3], "stats-point-dna-multiple-max") ? 8 : 12,
            !strcmp(argv[3], "stats-point-dna-multiple-max") ? 8 : 10,
            !strcmp(argv[3], "stats-point-dna-multiple-max") ? 8 : 9,
            8
        },
        .birth_dna_mode_available = !strcmp(argv[3], "mode-dna") ||
            !strcmp(argv[3], "mode-dedicated-dna"),
        .birth_dna_mode = !strcmp(argv[3], "mode-dedicated-dna") ?
            SV_BIRTH_MODE_DEDICATED_PVP : SV_BIRTH_MODE_EVERLASTING,
        .birth_dedicated = !strcmp(argv[3], "mode-dedicated") ||
            !strcmp(argv[3], "mode-dedicated-unavailable") ||
            !strcmp(argv[3], "mode-dedicated-iddc") ||
            !strcmp(argv[3], "mode-dedicated-dna")
    });
    SDL_WaitThread(worker, NULL);
    close(peer.listener);

    assert(result == (!strcmp(argv[3], "session-disconnect") ? 1 : 0) &&
           peer.request_exact);
    assert(outcome.name_submitted);
    assert(outcome.random_name_suggested);
    assert(outcome.name_submit_phase == SV_PREGAME_CHARACTER_PENDING);
    assert(outcome.phase == (!strcmp(argv[3], "session-disconnect") ?
        SV_PREGAME_DISCONNECTED : SV_PREGAME_BIRTH));
    assert(outcome.birth_entered);
    assert(outcome.birth_generation == outcome.generation);
#if SV_CLASS_BEFORE_RACE
    if (!strcmp(argv[3], "mode-pvp-maia") ||
        !strcmp(argv[3], "mode-pvp-maia-back")) {
        assert(outcome.birth_legal_class_count == 1);
        assert(outcome.birth_legal_classes[0] == 0);
    } else {
        assert(outcome.birth_legal_class_count == 2);
        assert(outcome.birth_legal_classes[0] == 0);
        assert(outcome.birth_legal_classes[1] == 2);
    }
#endif
    const char *quit_owner = NULL;
    bool control_quit = false;
    if (quit_matrix_case(argv[3], &quit_owner, &control_quit)) {
        (void)control_quit;
        SvBirthOwner owner = named_owner(quit_owner);
        assert(outcome.birth_owner == owner);
        assert(!owner_confirmed(&outcome, owner));
        assert(outcome.birth_quit);
    } else if (!strcmp(argv[3], "mode-pvp-maia-back")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(outcome.birth_mode_confirmed && !outcome.birth_mode_ready);
        assert(outcome.birth_mode == SV_BIRTH_MODE_PVP);
        assert(outcome.birth_race == (int)maia_race());
        assert(!outcome.birth_trait_confirmed);
    } else if (!strcmp(argv[3], "session-interleave")) {
        static const int16_t expected[] = {0, 1, 2, 3, 4, 5};
        assert(peer.interleaved_pongs == 7);
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
        assert(outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_class == 2 && outcome.birth_race == 2);
        assert(outcome.birth_trait == 2);
        assert(outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(!memcmp(outcome.birth_stats, expected, sizeof(expected)));
        assert(outcome.birth_mode == SV_BIRTH_MODE_SOLOIST);
        assert(outcome.birth_mode_ready);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
    } else if (!strcmp(argv[3], "session-stale-input")) {
        assert(outcome.birth_owner == SV_BIRTH_SEX);
        assert(outcome.birth_highlighted_sex == SV_BIRTH_SEX_FEMALE);
        assert(!outcome.birth_sex_confirmed && outcome.birth_quit);
        assert(!outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "session-disconnect")) {
        assert(!outcome.birth_active && outcome.birth_closed);
        assert(outcome.birth_owner == SV_BIRTH_NONE);
        assert(!outcome.birth_sex_confirmed && !outcome.birth_quit);
        assert(outcome.birth_highlighted_sex == SV_BIRTH_SEX_FEMALE);
    } else if (!strcmp(argv[3], "session-retry-generation")) {
        assert(peer.attempt == 2);
        assert(outcome.prior_generation &&
               outcome.prior_generation != outcome.generation);
        assert(outcome.birth_generation == outcome.generation);
        assert(outcome.birth_owner == SV_BIRTH_SEX);
        assert(outcome.birth_highlighted_sex == SV_BIRTH_SEX_FEMALE);
        assert(!outcome.birth_sex_confirmed && outcome.birth_quit);
        assert(!outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "guide-all-owners")) {
        static const int16_t expected_stats[] = {0, 1, 2, 3, 4, 5};
        assert(outcome.birth_placeholder_open_count == 7);
        assert(outcome.birth_placeholder_close_count == 7);
        assert(outcome.birth_placeholder_owner_mask == 0xfeu);
        assert(outcome.birth_placeholder_draft_preserved);
        assert(!strcmp(outcome.birth_placeholder_text,
                       "The guide is in development"));
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(outcome.birth_highlighted_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_highlighted_class == 2 && outcome.birth_class == 2);
        assert(outcome.birth_highlighted_race == 2 && outcome.birth_race == 2);
        assert(outcome.birth_highlighted_trait == 2 && outcome.birth_trait == 2);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_FRUIT_BAT &&
               outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(!memcmp(outcome.birth_stats, expected_stats,
                       sizeof(expected_stats)));
        assert(outcome.birth_highlighted_mode == SV_BIRTH_MODE_NO_GHOST);
        assert(!outcome.birth_mode_confirmed &&
               outcome.birth_mode == SV_BIRTH_MODE_NONE);
    } else if (!strcmp(argv[3], "mode-rpg")) {
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
        assert(outcome.birth_mode_ready && outcome.birth_mode_confirmed);
        assert(outcome.birth_mode == SV_BIRTH_MODE_SOLOIST);
        assert(outcome.birth_legal_mode_count == 2);
        assert(outcome.birth_legal_modes[0] == SV_BIRTH_MODE_NO_GHOST);
        assert(outcome.birth_legal_modes[1] == SV_BIRTH_MODE_SOLOIST);
        assert(outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "mode-dedicated-iddc")) {
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
        assert(outcome.birth_mode_ready && outcome.birth_mode_confirmed);
        assert(outcome.birth_mode == SV_BIRTH_MODE_IDDC);
    } else if (!strcmp(argv[3], "mode-dedicated-dna")) {
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
        assert(outcome.birth_mode_ready && outcome.birth_mode_confirmed);
        assert(outcome.birth_mode_dna_used);
        assert(outcome.birth_mode == SV_BIRTH_MODE_DEDICATED_PVP);
    } else if (!strcmp(argv[3], "mode-pvp-maia")) {
        static const int16_t expected[] = {0, 1, 2, 3, 4, 5};
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
        assert(outcome.birth_mode_confirmed && outcome.birth_mode_ready);
        assert(outcome.birth_mode == SV_BIRTH_MODE_PVP);
        assert(outcome.birth_legal_mode_count == 6);
        assert(outcome.birth_legal_modes[0] == SV_BIRTH_MODE_NORMAL);
        assert(outcome.birth_legal_modes[1] == SV_BIRTH_MODE_NO_GHOST);
        assert(outcome.birth_legal_modes[2] == SV_BIRTH_MODE_SOLOIST);
        assert(outcome.birth_legal_modes[3] == SV_BIRTH_MODE_EVERLASTING);
        assert(outcome.birth_legal_modes[4] == SV_BIRTH_MODE_HELLISH);
        assert(outcome.birth_legal_modes[5] == SV_BIRTH_MODE_PVP);
        assert(outcome.birth_race == (int)maia_race());
        assert(outcome.birth_trait_confirmed && outcome.birth_trait == 15);
        assert(outcome.birth_stats_confirmed &&
               !memcmp(outcome.birth_stats, expected, sizeof(expected)));
    } else if (!strcmp(argv[3], "mode-back")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(outcome.birth_stats_confirmed);
        assert(!outcome.birth_mode_confirmed && !outcome.birth_mode_ready);
    } else if (!strcmp(argv[3], "mode-random")) {
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
        assert(outcome.birth_mode_confirmed && outcome.birth_mode_ready);
        assert(outcome.birth_mode_random_used);
        assert(outcome.birth_mode == SV_BIRTH_MODE_NORMAL ||
               outcome.birth_mode == SV_BIRTH_MODE_NO_GHOST ||
               outcome.birth_mode == SV_BIRTH_MODE_SOLOIST ||
               outcome.birth_mode == SV_BIRTH_MODE_EVERLASTING);
    } else if (!strcmp(argv[3], "mode-ctrl-quit")) {
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(!outcome.birth_mode_confirmed && !outcome.birth_mode_ready);
    } else if (!strcmp(argv[3], "mode-dedicated")) {
        assert(outcome.birth_mode_ready && outcome.birth_mode_confirmed);
        assert(outcome.birth_mode == SV_BIRTH_MODE_DEDICATED_PVP);
        assert(outcome.birth_legal_mode_count == 4);
        assert(outcome.birth_legal_modes[0] == SV_BIRTH_MODE_IDDC);
        assert(outcome.birth_legal_modes[1] == SV_BIRTH_MODE_IDDC_SOLOIST);
        assert(outcome.birth_legal_modes[2] == SV_BIRTH_MODE_IDDC_HELLISH);
        assert(outcome.birth_legal_modes[3] == SV_BIRTH_MODE_DEDICATED_PVP);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "mode-dedicated-unavailable")) {
        assert(!outcome.birth_mode_ready && !outcome.birth_mode_confirmed);
        assert(outcome.birth_legal_mode_count == 3);
        assert(outcome.birth_highlighted_mode == SV_BIRTH_MODE_IDDC);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "mode-dna")) {
        assert(outcome.birth_mode_ready && outcome.birth_mode_confirmed);
        assert(outcome.birth_mode_dna_used);
        assert(outcome.birth_mode == SV_BIRTH_MODE_EVERLASTING);
        assert(outcome.birth_owner == SV_BIRTH_READY && outcome.birth_quit);
    } else if (!strcmp(argv[3], "mode-dna-missing")) {
        assert(!outcome.birth_mode_ready && !outcome.birth_mode_confirmed);
        assert(!outcome.birth_mode_dna_used && outcome.birth_illegal_ignored);
        assert(outcome.birth_highlighted_mode == SV_BIRTH_MODE_NORMAL);
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "mode-ordinary-ready")) {
        assert(outcome.birth_mode_ready && outcome.birth_mode_confirmed);
        assert(outcome.birth_owner == SV_BIRTH_READY);
        assert(outcome.birth_mode == SV_BIRTH_MODE_NO_GHOST);
        assert(outcome.birth_highlighted_mode == SV_BIRTH_MODE_NO_GHOST);
        assert(outcome.birth_legal_mode_count == 4);
        assert(outcome.birth_legal_modes[0] == SV_BIRTH_MODE_NORMAL);
        assert(outcome.birth_legal_modes[1] == SV_BIRTH_MODE_NO_GHOST);
        assert(outcome.birth_legal_modes[2] == SV_BIRTH_MODE_SOLOIST);
        assert(outcome.birth_legal_modes[3] == SV_BIRTH_MODE_EVERLASTING);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "stats-order-confirm")) {
        static const int16_t expected[] = {0, 1, 2, 3, 4, 5};
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(outcome.birth_stats_confirmed &&
               outcome.birth_stats_order_count == 6);
        assert(!memcmp(outcome.birth_stats, expected, sizeof(expected)));
    } else if (!strcmp(argv[3], "stats-order-random")) {
        bool seen[6] = {false};
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(outcome.birth_stats_confirmed && outcome.birth_stats_random_used);
        for (unsigned i = 0; i < 6; ++i) {
            assert(outcome.birth_stats[i] >= 0 && outcome.birth_stats[i] < 6);
            assert(!seen[outcome.birth_stats[i]]);
            seen[outcome.birth_stats[i]] = true;
        }
    } else if (!strcmp(argv[3], "stats-order-mouse")) {
        static const int16_t expected[] = {0, 1, 2, 3, 4, 5};
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(outcome.birth_stats_confirmed && outcome.birth_mouse_navigated);
        assert(!memcmp(outcome.birth_stats, expected, sizeof(expected)));
    } else if (!strcmp(argv[3], "stats-back")) {
#if SV_CLASS_BEFORE_RACE
        assert(outcome.birth_owner == SV_BIRTH_BODY);
#else
        assert(outcome.birth_owner == SV_BIRTH_CLASS);
#endif
        assert(!outcome.birth_stats_confirmed && outcome.birth_quit);
    } else if (!strcmp(argv[3], "stats-back-no-trait-body")) {
        assert(outcome.birth_trait_skipped && outcome.birth_body_skipped);
#if SV_CLASS_BEFORE_RACE
        assert(outcome.birth_owner == SV_BIRTH_RACE);
#else
        assert(outcome.birth_owner == SV_BIRTH_CLASS);
#endif
        assert(!outcome.birth_stats_confirmed && outcome.birth_quit);
    } else if (!strcmp(argv[3], "stats-quit") ||
               !strcmp(argv[3], "stats-ctrl-quit")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(!outcome.birth_stats_confirmed);
    } else if (!strcmp(argv[3], "stats-point-confirm-warning")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(!outcome.birth_stats_confirmed &&
               outcome.birth_stats_confirmation_pending);
        assert(outcome.birth_stats_remaining == 30);
    } else if (!strcmp(argv[3], "stats-point-confirm") ||
               !strcmp(argv[3], "stats-point-mouse")) {
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(outcome.birth_stats_confirmed && outcome.birth_stats_point_buy);
        assert(outcome.birth_stats_remaining == 30);
        if (!strcmp(argv[3], "stats-point-mouse"))
            assert(outcome.birth_mouse_navigated);
    } else if (!strcmp(argv[3], "stats-point-dna")) {
        static const int16_t expected[] = {17, 14, 12, 10, 9, 8};
        assert(outcome.birth_owner == SV_BIRTH_MODE && outcome.birth_quit);
        assert(outcome.birth_stats_confirmed && outcome.birth_stats_dna_used);
        assert(!memcmp(outcome.birth_stats, expected, sizeof(expected)));
        assert(outcome.birth_stats_remaining == 9);
    } else if (!strcmp(argv[3], "stats-point-dna-over-budget") ||
               !strcmp(argv[3], "stats-point-dna-illegal") ||
               !strcmp(argv[3], "stats-point-dna-multiple-max") ||
               !strcmp(argv[3], "stats-point-dna-missing")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(!outcome.birth_stats_confirmed && !outcome.birth_stats_dna_used);
        assert(outcome.birth_highlighted_stat == 0);
        for (unsigned i = 0; i < 6; ++i) assert(outcome.birth_stats[i] == 10);
        assert(outcome.birth_stats_remaining == 30 &&
               outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "stats-point-lower-bound")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(outcome.birth_highlighted_stat == 0 && outcome.birth_stats[0] == 9);
        assert(outcome.birth_stats_remaining == 31 &&
               outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "stats-point-navigate")) {
        assert(outcome.creation_flags == 1u);
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(outcome.birth_stats_point_buy && !outcome.birth_stats_confirmed);
        assert(outcome.birth_highlighted_stat == 1);
        assert(outcome.birth_stats[0] == 10 && outcome.birth_stats[1] == 12);
        assert(outcome.birth_stats_remaining == 28);
        assert(outcome.birth_legal_stat_count == 6);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "stats-order-navigate")) {
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(!outcome.birth_stats_confirmed);
        assert(outcome.birth_stats_order_count == 1);
        assert(outcome.birth_stats[0] == 1);
        assert(outcome.birth_legal_stat_count == 5);
        assert(outcome.birth_highlighted_stat == 0);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored);
    } else
#if !SV_CLASS_BEFORE_RACE
    if (!strcmp(argv[3], "alternate-race-back")) {
        assert(outcome.birth_owner == SV_BIRTH_SEX && outcome.birth_quit);
        assert(!outcome.birth_race_confirmed &&
               outcome.birth_race == SV_BIRTH_RACE_NONE);
        assert(outcome.birth_highlighted_race == 2);
    } else if (!strcmp(argv[3], "alternate-trait-back")) {
        assert(outcome.birth_owner == SV_BIRTH_RACE && outcome.birth_quit);
        assert(outcome.birth_race_confirmed && outcome.birth_race == 2);
        assert(!outcome.birth_trait_confirmed &&
               outcome.birth_trait == SV_BIRTH_TRAIT_NONE);
        assert(outcome.birth_highlighted_trait == 2);
    } else if (!strcmp(argv[3], "alternate-body-back")) {
        assert(outcome.birth_owner == SV_BIRTH_TRAIT && outcome.birth_quit);
        assert(!outcome.birth_trait_confirmed &&
               outcome.birth_trait == SV_BIRTH_TRAIT_NONE);
        assert(!outcome.birth_body_confirmed &&
               outcome.birth_body == SV_BIRTH_BODY_NONE);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_FRUIT_BAT);
    } else if (!strcmp(argv[3], "alternate-class-back")) {
        assert(outcome.birth_owner == SV_BIRTH_BODY && outcome.birth_quit);
        assert(!outcome.birth_class_confirmed &&
               outcome.birth_class == SV_BIRTH_CLASS_NONE);
        assert(outcome.birth_body_confirmed &&
               outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    } else if (!strcmp(argv[3], "alternate-class-back-no-trait-body")) {
        assert(outcome.birth_trait_skipped && outcome.birth_body_skipped);
        assert(outcome.birth_owner == SV_BIRTH_RACE && outcome.birth_quit);
        assert(!outcome.birth_class_confirmed && outcome.birth_race == 2);
    } else if (!strcmp(argv[3], "alternate-class-back-no-body")) {
        assert(outcome.creation_flags == 0x10u);
        assert(outcome.birth_body_skipped && !outcome.birth_body_confirmed);
        assert(!outcome.birth_class_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(outcome.birth_race == 2 && outcome.birth_trait == 2);
        assert(outcome.birth_quit);
    } else if (!strcmp(argv[3], "alternate-navigate")) {
        assert(outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_race == 0 && outcome.birth_trait == 1);
        assert(outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_class == 2 && outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_legal_class_count == 2);
        assert(outcome.birth_legal_classes[0] == 0 &&
               outcome.birth_legal_classes[1] == 2);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "alternate-random")) {
        assert(outcome.birth_random_used && outcome.birth_race_random_used);
        assert(outcome.birth_trait_random_used && outcome.birth_body_random_used);
        assert(outcome.birth_class_random_used && outcome.birth_quit);
        assert(outcome.birth_owner == SV_BIRTH_STATS);
        assert((outcome.birth_race == 0 &&
                (outcome.birth_class == 0 || outcome.birth_class == 2)) ||
               (outcome.birth_race == 1 && outcome.birth_class == 0) ||
               (outcome.birth_race == 2 && outcome.birth_class == 2));
    } else if (!strcmp(argv[3], "alternate-dna")) {
        assert(outcome.birth_dna_used && outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_race_dna_used && outcome.birth_race == 2);
        assert(outcome.birth_trait_dna_used && outcome.birth_trait == 2);
        assert(outcome.birth_body_dna_used &&
               outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_class_dna_used && outcome.birth_class == 2);
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
    } else if (!strcmp(argv[3], "alternate-class-dna-incompatible")) {
        assert(outcome.birth_owner == SV_BIRTH_CLASS && outcome.birth_quit);
        assert(!outcome.birth_class_confirmed && !outcome.birth_class_dna_used);
        assert(outcome.birth_class == SV_BIRTH_CLASS_NONE);
        assert(outcome.birth_highlighted_class == 2 &&
               outcome.birth_illegal_ignored);
    } else if (!strcmp(argv[3], "alternate-mouse")) {
        assert(outcome.birth_sex == SV_BIRTH_SEX_FEMALE);
        assert(outcome.birth_race == 2 && outcome.birth_trait == 2);
        assert(outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_class == 2 && outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_mouse_navigated && outcome.birth_quit);
    } else if (!strcmp(argv[3], "alternate-ctrl-quit")) {
        assert(outcome.birth_owner == SV_BIRTH_RACE && outcome.birth_quit);
        assert(!outcome.birth_race_confirmed &&
               outcome.birth_race == SV_BIRTH_RACE_NONE);
    } else if (!strcmp(argv[3], "alternate-order")) {
        assert(outcome.birth_sex_confirmed && outcome.birth_sex == SV_BIRTH_SEX_MALE);
        assert(outcome.birth_race_confirmed && outcome.birth_race == 2);
        assert(outcome.birth_trait_confirmed && outcome.birth_trait == 2);
        assert(outcome.birth_body_confirmed &&
               outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_class_confirmed && outcome.birth_class == 2);
        assert(outcome.birth_owner == SV_BIRTH_STATS && outcome.birth_quit);
        assert(outcome.birth_legal_race_count == 3);
        assert(outcome.birth_legal_class_count == 1 &&
               outcome.birth_legal_classes[0] == 2);
    } else
#endif
    if (!strcmp(argv[3], "body-mouse")) {
        assert(outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_mouse_navigated && outcome.birth_quit);
    } else if (!strcmp(argv[3], "body-quit") ||
               !strcmp(argv[3], "body-ctrl-quit")) {
        assert(!outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_BODY);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_NORMAL);
        assert(outcome.birth_body == SV_BIRTH_BODY_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "body-unavailable-skip")) {
        assert(outcome.creation_flags == 0x10u);
        assert(outcome.birth_body_skipped && !outcome.birth_body_confirmed);
        assert(outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_body_count == 0 && outcome.birth_legal_body_count == 0);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_NONE);
        assert(outcome.birth_body == SV_BIRTH_BODY_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "body-back-no-trait")) {
        assert(outcome.birth_trait_skipped && !outcome.birth_trait_confirmed);
        assert(outcome.birth_legal_body_count == 2);
        assert(!outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_body == SV_BIRTH_BODY_NONE);
        assert(outcome.birth_race == 2 && outcome.birth_quit);
    } else if (!strcmp(argv[3], "body-back")) {
        assert(!outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_body == SV_BIRTH_BODY_NONE);
        assert(!outcome.birth_trait_confirmed && outcome.birth_trait == SV_BIRTH_TRAIT_NONE);
        assert(outcome.birth_quit);
    } else if (!strcmp(argv[3], "body-dna")) {
        assert(outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_body_dna_used);
        assert(outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    } else if (!strncmp(argv[3], "body-dna-", 9)) {
        assert(!outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_BODY);
        assert(!outcome.birth_body_dna_used && outcome.birth_illegal_ignored);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_NORMAL);
        assert(outcome.birth_body == SV_BIRTH_BODY_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "body-random")) {
        assert(outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_body_random_used);
        assert(outcome.birth_body == SV_BIRTH_BODY_NORMAL ||
               outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
    } else if (!strcmp(argv[3], "body-navigate")) {
        assert(outcome.birth_body_count == 2);
        assert(outcome.birth_legal_body_count == 2);
        assert(outcome.birth_legal_bodies[0] == SV_BIRTH_BODY_NORMAL);
        assert(outcome.birth_legal_bodies[1] == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_body_confirmed && outcome.birth_owner == SV_BIRTH_STATS);
        assert(outcome.birth_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_highlighted_body == SV_BIRTH_BODY_FRUIT_BAT);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "trait-back")) {
        assert(!outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_RACE);
        assert(outcome.birth_race == 2 && outcome.birth_highlighted_trait == 2);
        assert(outcome.birth_trait == SV_BIRTH_TRAIT_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "trait-quit") ||
               !strcmp(argv[3], "trait-ctrl-quit")) {
        assert(!outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(outcome.birth_race == 2 && outcome.birth_highlighted_trait == 1);
        assert(outcome.birth_trait == SV_BIRTH_TRAIT_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "trait-mouse")) {
        assert(outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_BODY);
        assert(outcome.birth_trait == 2 && outcome.birth_mouse_navigated);
        assert(outcome.birth_quit);
    } else if (!strcmp(argv[3], "trait-dummy-skip-back")) {
        assert(outcome.birth_trait_skipped && !outcome.birth_trait_confirmed);
        assert(outcome.birth_owner == SV_BIRTH_RACE && outcome.birth_quit);
        assert(outcome.birth_race == 2 && outcome.birth_trait == SV_BIRTH_TRAIT_NONE);
    } else if (!strcmp(argv[3], "trait-dummy-skip") ||
               !strcmp(argv[3], "trait-unavailable-skip")) {
        assert(outcome.birth_trait_skipped && !outcome.birth_trait_confirmed);
        assert(outcome.birth_owner == SV_BIRTH_BODY && outcome.birth_quit);
        assert(outcome.birth_legal_trait_count == 0);
        assert(outcome.birth_trait == SV_BIRTH_TRAIT_NONE);
        if (!strcmp(argv[3], "trait-dummy-skip"))
            assert(outcome.birth_trait_count == 1);
        else
            assert(outcome.birth_trait_count == 4);
    } else if (!strcmp(argv[3], "trait-dna")) {
        assert(outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_BODY);
        assert(outcome.birth_trait_dna_used && outcome.birth_trait == 2);
    } else if (!strncmp(argv[3], "trait-dna-", 10)) {
        assert(!outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_TRAIT);
        assert(!outcome.birth_trait_dna_used && outcome.birth_illegal_ignored);
        assert(outcome.birth_class == 2 && outcome.birth_race == 2);
        assert(outcome.birth_highlighted_trait == 1);
        assert(outcome.birth_trait == SV_BIRTH_TRAIT_NONE && outcome.birth_quit);
    } else if (!strcmp(argv[3], "trait-random")) {
        assert(outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_BODY);
        assert(outcome.birth_trait_random_used);
        assert(outcome.birth_trait == 1 || outcome.birth_trait == 2);
    } else if (!strcmp(argv[3], "trait-navigate")) {
        assert(outcome.birth_class_confirmed && outcome.birth_class == 2);
        assert(outcome.birth_race_confirmed && outcome.birth_race == 2);
        assert(outcome.birth_trait_count == 4);
        assert(outcome.birth_legal_trait_count == 2);
        assert(outcome.birth_legal_traits[0] == 1);
        assert(outcome.birth_legal_traits[1] == 2);
        assert(outcome.birth_trait_confirmed && outcome.birth_owner == SV_BIRTH_BODY);
        assert(outcome.birth_trait == 2 && outcome.birth_highlighted_trait == 2);
        assert(outcome.birth_keyboard_navigated && outcome.birth_mouse_navigated);
        assert(outcome.birth_illegal_ignored && outcome.birth_quit);
    } else if (!strcmp(argv[3], "race-back")) {
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
