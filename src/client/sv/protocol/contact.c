#include "protocol/contact.h"
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#define CLIENT_SIDE
#define USE_SDL3
#ifndef MINGW
#define LINUX
#endif
#include "../../../common/defines.h"


struct SvContact {
    SvContactState state;
    SvContactState failed_phase;
    int protocol, version[6];
    unsigned rejection;
    uint32_t creation_flags;
    SvContactSetup *setup;
    unsigned char input[SV_CONTACT_INPUT_CAPACITY], output[SV_CONTACT_OUTPUT_CAPACITY];
    size_t input_size, output_size;
    char real[REALNAME_LEN], account[ACCNAME_LEN], host[HOSTNAME_LEN];
    unsigned char password[PASSWORD_LEN];
    size_t password_size;
};

static void put16(unsigned char *out, unsigned value)
{
    out[0] = (unsigned char)(value >> 8); out[1] = (unsigned char)value;
}
static void wipe(void *memory, size_t size)
{
    volatile unsigned char *bytes = memory;
    while (size--) *bytes++ = 0;
}
static void put32(unsigned char *out, uint32_t value)
{
    out[0] = (unsigned char)(value >> 24); out[1] = (unsigned char)(value >> 16);
    out[2] = (unsigned char)(value >> 8); out[3] = (unsigned char)value;
}
static uint32_t get32(const unsigned char *in)
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
           ((uint32_t)in[2] << 8) | in[3];
}
static bool field(char *out, size_t capacity, const char *value)
{
    if (!value) return false;
    size_t length = strlen(value);
    if (!length || length >= capacity || memchr(value, 0, length) != NULL) return false;
    memcpy(out, value, length + 1);
    return true;
}
static void consume(SvContact *contact, size_t count)
{
    contact->input_size -= count;
    memmove(contact->input, contact->input + count, contact->input_size);
}
static void append_string(SvContact *contact, const char *value)
{
    size_t count = strlen(value) + 1;
    memcpy(contact->output + contact->output_size, value, count);
    contact->output_size += count;
}
SvContact *sv_contact_create(int server_protocol, const SvContactIdentity *identity)
{
    if (!identity || server_protocol < 0 || server_protocol > 255) return NULL;
    SvContact *contact = calloc(1, sizeof(*contact));
    if (!contact) return NULL;
    contact->setup = calloc(1, sizeof(*contact->setup));
    if (!contact->setup) goto invalid;
    if (!field(contact->real, sizeof(contact->real), identity->real_name) ||
        !field(contact->account, sizeof(contact->account), identity->account) ||
        !field(contact->host, sizeof(contact->host), identity->host_name) ||
        !identity->password) goto invalid;
    contact->password_size = strlen(identity->password);
    if (!contact->password_size || contact->password_size >= sizeof(contact->password)) goto invalid;
    if (server_protocol >= 2 && memchr(identity->password, '*', contact->password_size)) goto invalid;
    memcpy(contact->password, identity->password, contact->password_size);
    contact->account[0] = (char)toupper((unsigned char)contact->account[0]);
    contact->protocol = server_protocol;
    contact->state = SV_CONTACT_WAIT_CONTACT;
    unsigned char *out = contact->output;
    put32(out, 12345); contact->output_size = 4;
    append_string(contact, contact->real);
    put16(out + contact->output_size, 0); contact->output_size += 2;
    out[contact->output_size++] = 255;
    append_string(contact, contact->account);
    append_string(contact, contact->host);
    put16(out + contact->output_size, server_protocol >= 2 ? 65535 : MY_VERSION);
    contact->output_size += 2;
    if (server_protocol >= 2) {
        const int version[6] = {VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
            VERSION_EXTRA, VERSION_BRANCH,
            VERSION_BUILD + (VERSION_OS + VERSION_OS_SUB * 100) * 1000000};
        for (int i = 0; i < 6; ++i) {
            put32(out + contact->output_size, (uint32_t)version[i]);
            contact->output_size += 4;
        }
    }
    return contact;
invalid:
    sv_contact_destroy(contact);
    return NULL;
}
void sv_contact_destroy(SvContact *contact)
{
    if (!contact) return;
    free(contact->setup);
    wipe(contact, sizeof(*contact));
    free(contact);
}
SvContactState sv_contact_state(const SvContact *contact) { return contact->state; }
unsigned sv_contact_rejection(const SvContact *contact) { return contact->rejection; }
SvContactState sv_contact_failure_phase(const SvContact *contact) { return contact->failed_phase; }
const int *sv_contact_version(const SvContact *contact) { return contact->version; }
const SvContactSetup *sv_contact_setup(const SvContact *contact)
{
    return contact && contact->state == SV_CONTACT_READY ? contact->setup : NULL;
}
uint32_t sv_contact_creation_flags(const SvContact *contact)
{
    return contact ? contact->creation_flags : 0;
}
SvOutput sv_contact_take_output(SvContact *contact, void *bytes, size_t capacity)
{
    if (!contact->output_size) return (SvOutput){SV_WAITING, 0};
    if (capacity < contact->output_size) return (SvOutput){SV_OUTPUT_TOO_SMALL, contact->output_size};
    if (!bytes) return (SvOutput){SV_INVALID, contact->output_size};
    size_t count = contact->output_size;
    memcpy(bytes, contact->output, count);
    wipe(contact->output, count);
    contact->output_size = 0;
    return (SvOutput){SV_OK, count};
}
SvOutput sv_contact_take_remaining(SvContact *contact, void *bytes, size_t capacity)
{
    if (contact->state != SV_CONTACT_READY) return (SvOutput){SV_WAITING, 0};
    if (capacity < contact->input_size) return (SvOutput){SV_OUTPUT_TOO_SMALL, contact->input_size};
    if (contact->input_size && !bytes) return (SvOutput){SV_INVALID, contact->input_size};
    size_t count = contact->input_size;
    if (count) memcpy(bytes, contact->input, count);
    contact->input_size = 0;
    return (SvOutput){SV_OK, count};
}
static int version_newer(const int version[6], const int threshold[6])
{
    for (int i = 0; i < 6; ++i)
        if (version[i] != threshold[i]) return version[i] > threshold[i];
    return 0;
}
static int read_string(const unsigned char *bytes, size_t length, size_t *cursor,
                       char out[SV_CONTACT_NAME_CAPACITY])
{
    if (*cursor >= length) return 0;
    size_t available = length - *cursor;
    if (available > SV_CONTACT_NAME_CAPACITY) available = SV_CONTACT_NAME_CAPACITY;
    const unsigned char *end = memchr(bytes + *cursor, 0, available);
    if (!end) return available == SV_CONTACT_NAME_CAPACITY ? -1 : 0;
    size_t count = (size_t)(end - (bytes + *cursor));
    if (!count) return -1;
    memcpy(out, bytes + *cursor, count);
    out[count] = 0;
    *cursor = (size_t)(end - bytes) + 1;
    return 1;
}
static int parse_setup(SvContact *contact, size_t *used)
{
    const unsigned char *in = contact->input;
    size_t size = contact->input_size;
    SvContactSetup *setup = contact->setup;
    const int traits_since[6] = {4, 4, 5, 10, 0, 0};
    const int recommendations_since[6] = {4, 4, 3, 1, 0, 0};
    int traits = version_newer(contact->version, traits_since);
    size_t header = traits ? 13 : 12;
    if (size < header) return 0;
    uint32_t motd_size = get32(in);
    if (motd_size > SV_CONTACT_MOTD_CAPACITY || get32(in + header - 4) < header) return -1;
    unsigned races = in[6], classes = in[7], trait_count = traits ? in[8] : 0;
    setup->motd_size = motd_size;
    setup->frames_per_second = (uint16_t)((in[4] << 8) | in[5]);
    setup->race_count = (uint8_t)races;
    setup->class_count = (uint8_t)classes;
    setup->trait_count = (uint8_t)trait_count;
    setup->setup_size = get32(in + header - 4);
    size_t at = header;
    for (unsigned i = 0; i < races; ++i) {
        if (size - at < 6) return 0;
        for (int j = 0; j < 6; ++j)
            setup->races[i].adjustment[j] = (int16_t)in[at + j] - 50;
        at += 6;
        int next = read_string(in, size, &at, setup->races[i].title);
        if (next <= 0) return next;
        if (size - at < 4) return 0;
        setup->races[i].choice = get32(in + at);
        at += 4;
    }
    for (unsigned i = 0; i < classes; ++i) {
        if (size - at < 6) return 0;
        for (int j = 0; j < 6; ++j)
            setup->classes[i].adjustment[j] = (int16_t)in[at + j] - 50;
        at += 6;
        int next = read_string(in, size, &at, setup->classes[i].title);
        if (next <= 0) return next;
        if (isdigit((unsigned char)setup->classes[i].title[0]) &&
            setup->classes[i].title[1]) {
            setup->classes[i].hidden = 1;
            long base_class = strtol(setup->classes[i].title, NULL, 10);
            if (base_class < 0 || base_class > INT_MAX) return -1;
            setup->classes[i].base_class = (int32_t)base_class;
            memmove(setup->classes[i].title, setup->classes[i].title + 2,
                    strlen(setup->classes[i].title + 2) + 1);
        }
        if (version_newer(contact->version, recommendations_since)) {
            if (size - at < 6) return 0;
            memcpy(setup->classes[i].recommendation, in + at, 6);
            at += 6;
        }
    }
    for (unsigned i = 0; i < trait_count; ++i) {
        int next = read_string(in, size, &at, setup->traits[i].title);
        if (next <= 0) return next;
        if (size - at < 4) return 0;
        setup->traits[i].choice = get32(in + at);
        at += 4;
    }
    if (size - at < motd_size) return 0;
    memcpy(setup->motd, in + at, motd_size);
    setup->motd[motd_size] = 0;
    *used = at + motd_size;
    return 1;
}
SvResult sv_contact_receive(SvContact *contact, const void *bytes, size_t size)
{
    if (!contact || contact->state == SV_CONTACT_FAILED) return SV_CLOSED;
    if ((size && !bytes) || size > sizeof(contact->input) - contact->input_size) return SV_INPUT_OVERFLOW;
    if (size) memcpy(contact->input + contact->input_size, bytes, size);
    contact->input_size += size;
    for (;;) {
        if (contact->state == SV_CONTACT_WAIT_CONTACT) {
            if (contact->input_size && contact->input[0] != 255) {
                contact->failed_phase = contact->state;
                contact->state = SV_CONTACT_FAILED;
                return SV_INVALID;
            }
            if (contact->input_size < 10) return SV_OK;
            uint32_t flags = get32(contact->input + 6);
            size_t response = flags & 2 ? 34 : 10;
            if (contact->input_size < response) return SV_OK;
            contact->rejection = contact->input[1];
            if (flags & 2) for (int i = 0; i < 6; ++i) {
                uint32_t part = get32(contact->input + 10 + 4 * i);
                if (part > INT_MAX || (i == 0 && !part)) {
                    contact->failed_phase = contact->state;
                    contact->state = SV_CONTACT_FAILED;
                    return SV_INVALID;
                }
                contact->version[i] = (int)part;
            }
            else {
                const int fallback[6] = {VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, 0, 0, 0};
                memcpy(contact->version, fallback, sizeof(fallback));
            }
            contact->creation_flags = flags & ~2u;
            contact->setup->creation_flags = contact->creation_flags;
            consume(contact, response);
            if (contact->rejection && contact->rejection != 3) {
                contact->failed_phase = contact->state;
                contact->state = SV_CONTACT_FAILED;
                return SV_INVALID;
            }
            unsigned char *out = contact->output;
            contact->output_size = 0;
            out[contact->output_size++] = 1;
            append_string(contact, contact->real);
            append_string(contact, contact->account);
            for (size_t i = 0; i < contact->password_size; ++i)
                out[contact->output_size++] = contact->password[i] ^ (contact->protocol >= 2 ? 42 : 0);
            out[contact->output_size++] = 0;
            wipe(contact->password, sizeof(contact->password));
            contact->password_size = 0;
            contact->state = SV_CONTACT_WAIT_VERIFY;
        } else if (contact->state == SV_CONTACT_WAIT_VERIFY) {
            if (contact->input_size < 3) return SV_OK;
            if (contact->input[0] != 2 || contact->input[1] != 1 || contact->input[2] != 122) {
                contact->failed_phase = contact->state;
                contact->state = SV_CONTACT_FAILED;
                return SV_INVALID;
            }
            if (contact->input_size < 8) return SV_OK;
            if (contact->input[3] != 6) {
                contact->failed_phase = contact->state;
                contact->state = SV_CONTACT_FAILED; return SV_DECODE_ERROR;
            }
            consume(contact, 8);
            contact->state = SV_CONTACT_WAIT_SETUP;
        } else if (contact->state == SV_CONTACT_WAIT_SETUP) {
            size_t used = 0;
            int parsed = parse_setup(contact, &used);
            if (parsed == 0) return SV_OK;
            if (parsed < 0) {
                contact->failed_phase = contact->state;
                contact->state = SV_CONTACT_FAILED; return SV_DECODE_ERROR;
            }
            consume(contact, used);
            contact->state = SV_CONTACT_READY;
            return SV_OK;
        } else return SV_OK;
    }
}
