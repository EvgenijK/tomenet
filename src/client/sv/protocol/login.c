#include "protocol/login.h"
#include "../../common/pack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SV_LOGIN_INPUT_CAPACITY (64 * 1024)
#define SV_LOGIN_OUTPUT_CAPACITY 1024
struct SvLogin {
    int version[6];
    SvLoginState state;
    unsigned char input[SV_LOGIN_INPUT_CAPACITY];
    size_t input_size;
    unsigned char output[SV_LOGIN_OUTPUT_CAPACITY];
    size_t output_size;
    uint32_t flags[4]; /* sflags3, sflags2, sflags1, sflags0 */
    SvLoginCharacter characters[SV_LOGIN_MAX_CHARACTERS];
    size_t count;
    size_t selected_slot;
    bool has_selected_slot;
    char reason[256];
};

static uint32_t get32(const unsigned char *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}
static int16_t get16(const unsigned char *p)
{
    return (int16_t)(((unsigned)p[0] << 8) | p[1]);
}
static bool newer(const int v[6], const int threshold[6])
{
    for (int i = 0; i < 6; ++i)
        if (v[i] != threshold[i]) return v[i] > threshold[i];
    return false;
}
static bool atleast(const int v[6], const int threshold[6])
{
    return !newer(threshold, v);
}
static void consume(SvLogin *login, size_t size)
{
    login->input_size -= size;
    memmove(login->input, login->input + size, login->input_size);
}
static SvResult append_output(SvLogin *login, const void *bytes, size_t size)
{
    if (size > sizeof(login->output) - login->output_size) return SV_BACKPRESSURE;
    memcpy(login->output + login->output_size, bytes, size);
    login->output_size += size;
    return SV_OK;
}
/* A split NUL-terminated wire field must not publish a partial row. */
static int string(const unsigned char *bytes, size_t size, size_t *at,
                  char *out, size_t capacity)
{
    if (*at >= size) return 0;
    size_t available = size - *at;
    const unsigned char *end = memchr(bytes + *at, 0,
                                     available < capacity ? available : capacity);
    if (!end) return available >= capacity ? -1 : 0;
    size_t length = (size_t)(end - bytes) - *at;
    memcpy(out, bytes + *at, length);
    out[length] = 0;
    *at += length + 1;
    return 1;
}
SvLogin *sv_login_create(const int version[6], const unsigned char iaddr[6])
{
    if (!version || version[0] <= 0 || !iaddr) return NULL;
    SvLogin *login = calloc(1, sizeof(*login));
    if (!login) return NULL;
    memcpy(login->version, version, sizeof(login->version));
    login->state = SV_LOGIN_WAIT_FLAGS;
    unsigned char start[8] = {PKT_LOGIN, 0};
    size_t start_size = 2;
    const int iaddr_since[6] = {4, 9, 2, 1, 0, 2};
    if (atleast(version, iaddr_since)) {
        memcpy(start + 2, iaddr, 6);
        start_size += 6;
    }
    (void)append_output(login, start, start_size);
    return login;
}
void sv_login_destroy(SvLogin *login)
{
    if (!login) return;
    volatile unsigned char *p = (void *)login;
    for (size_t i = 0; i < sizeof(*login); ++i) p[i] = 0;
    free(login);
}
SvOutput sv_login_take_output(SvLogin *login, void *bytes, size_t capacity)
{
    if (!login || !login->output_size) return (SvOutput){SV_WAITING, 0};
    if (capacity < login->output_size) return (SvOutput){SV_OUTPUT_TOO_SMALL, login->output_size};
    if (!bytes) return (SvOutput){SV_INVALID, login->output_size};
    size_t size = login->output_size;
    memcpy(bytes, login->output, size);
    login->output_size = 0;
    return (SvOutput){SV_OK, size};
}
SvLoginState sv_login_state(const SvLogin *login) { return login->state; }
size_t sv_login_count(const SvLogin *login) { return login->count; }
const SvLoginCharacter *sv_login_character(const SvLogin *login, size_t slot)
{
    return login && slot < login->count ? &login->characters[slot] : NULL;
}
const SvLoginCharacter *sv_login_selected_character(const SvLogin *login)
{
    return login && login->has_selected_slot && login->selected_slot < login->count ?
        &login->characters[login->selected_slot] : NULL;
}
const uint32_t *sv_login_flags(const SvLogin *login) { return login->flags; }
const char *sv_login_reason(const SvLogin *login) { return login->reason; }
SvResult sv_login_choose(SvLogin *login, size_t slot)
{
    if (!login || login->state != SV_LOGIN_OVERVIEW || slot >= login->count)
        return SV_INVALID;
    size_t length = strlen(login->characters[slot].name);
    unsigned char packet[SV_LOGIN_NAME_CAPACITY + 2] = {PKT_LOGIN};
    memcpy(packet + 1, login->characters[slot].name, length + 1);
    SvResult queued = append_output(login, packet, length + 2);
    if (queued != SV_OK) return queued;
    login->selected_slot = slot;
    login->has_selected_slot = true;
    login->state = SV_LOGIN_WAIT_STATUS;
    return SV_OK;
}
SvResult sv_login_keepalive(SvLogin *login)
{
    if (!login || login->state == SV_LOGIN_REJECTED) return SV_CLOSED;
    const unsigned char packet = PKT_KEEPALIVE;
    return append_output(login, &packet, 1);
}
SvResult sv_login_receive(SvLogin *login, const void *bytes, size_t size)
{
    if (!login || (size && !bytes) || size > sizeof(login->input) - login->input_size)
        return SV_INPUT_OVERFLOW;
    if (login->state == SV_LOGIN_REJECTED)
        return SV_INVALID;
    if (size) memcpy(login->input + login->input_size, bytes, size);
    login->input_size += size;
    for (;;) {
        if (!login->input_size) return SV_OK;
        if (login->input[0] == PKT_KEEPALIVE) {
            consume(login, 1);
            continue;
        }
        if (login->input[0] == PKT_PING) {
            if (login->input_size < 15) return SV_OK;
            size_t available = login->input_size - 14;
            if (available > 1024) available = 1024;
            const unsigned char *end = memchr(login->input + 14, 0, available);
            if (!end) return available == 1024 ? SV_DECODE_ERROR : SV_OK;
            size_t packet_size = (size_t)(end - login->input) + 1;
            if (!login->input[1]) {
                unsigned char reply[1024];
                if (packet_size > sizeof(reply)) return SV_DECODE_ERROR;
                memcpy(reply, login->input, packet_size);
                reply[1] = 1;
                SvResult queued = append_output(login, reply, packet_size);
                if (queued != SV_OK) return queued;
            }
            consume(login, packet_size);
            continue;
        }
        if (login->input[0] == PKT_QUIT) {
            size_t at = 1;
            int parsed = string(login->input, login->input_size, &at,
                                login->reason, sizeof(login->reason));
            if (parsed < 0) return SV_DECODE_ERROR;
            if (!parsed) return SV_OK;
            consume(login, at);
            login->state = SV_LOGIN_REJECTED;
            return SV_OK;
        }
        if ((login->state == SV_LOGIN_OVERVIEW || login->state == SV_LOGIN_SELECTED) &&
            login->input[0] == PKT_SFLAGS) {
            if (login->input_size < 17) return SV_OK;
            for (int i = 0; i < 4; ++i)
                login->flags[3 - i] = get32(login->input + 1 + 4 * i);
            consume(login, 17);
        } else if (login->state == SV_LOGIN_WAIT_FLAGS) {
            if (login->input[0] != PKT_SERVERDETAILS) return SV_DECODE_ERROR;
            if (login->input_size < 17) return SV_OK;
            for (int i = 0; i < 4; ++i) login->flags[i] = get32(login->input + 1 + 4 * i);
            consume(login, 17);
            login->state = SV_LOGIN_WAIT_LIST;
        } else if (login->state == SV_LOGIN_WAIT_LIST) {
            if (login->input[0] != PKT_LOGIN) return SV_DECODE_ERROR;
            size_t at = 1;
            const int mode_since[6] = {4, 4, 9, 2, 0, 0};
            const int location_since[6] = {4, 5, 7, 0, 0, 0};
            SvLoginCharacter row = {0};
            if (newer(login->version, mode_since)) {
                if (login->input_size - at < 2) return SV_OK;
                row.mode = get16(login->input + at); at += 2;
            }
            int parsed = string(login->input, login->input_size, &at,
                                row.colour, sizeof(row.colour));
            if (parsed < 0) return SV_DECODE_ERROR;
            if (!parsed) return SV_OK;
            parsed = string(login->input, login->input_size, &at,
                            row.name, sizeof(row.name));
            if (parsed < 0) return SV_DECODE_ERROR;
            if (!parsed) return SV_OK;
            if (login->input_size - at < 6) return SV_OK;
            row.level = get16(login->input + at); at += 2;
            row.race = get16(login->input + at); at += 2;
            row.class_id = get16(login->input + at); at += 2;
            if (newer(login->version, location_since)) {
                parsed = string(login->input, login->input_size, &at,
                                row.location, sizeof(row.location));
                if (parsed < 0) return SV_DECODE_ERROR;
                if (!parsed) return SV_OK;
            }
            consume(login, at);
            if (!row.name[0]) {
                login->state = SV_LOGIN_OVERVIEW;
                continue;
            }
            if (login->count >= SV_LOGIN_MAX_CHARACTERS) return SV_INPUT_OVERFLOW;
            login->characters[login->count++] = row;
        } else if (login->state == SV_LOGIN_WAIT_STATUS) {
            unsigned char status = login->input[0];
            consume(login, 1);
            if (status != 0) {
                snprintf(login->reason, sizeof(login->reason),
                         "Server rejected character selection (status %u).", status);
                login->state = SV_LOGIN_REJECTED;
                return SV_OK;
            }
            login->state = SV_LOGIN_SELECTED;
            continue;
        } else return SV_DECODE_ERROR;
    }
}
