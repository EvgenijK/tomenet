#ifndef SV_CONTACT_H
#define SV_CONTACT_H
#include "result.h"
#include "session/character-setup.h"
#include <stdint.h>

typedef enum { SV_CONTACT_WAIT_CONTACT, SV_CONTACT_WAIT_VERIFY,
               SV_CONTACT_WAIT_SETUP, SV_CONTACT_READY, SV_CONTACT_FAILED } SvContactState;
typedef struct SvContact SvContact;
#define SV_CONTACT_INPUT_CAPACITY (128 * 1024)
#define SV_CONTACT_OUTPUT_CAPACITY 512
typedef struct {
    const char *real_name, *account, *host_name, *password;
} SvContactIdentity;

/* The caller owns the TCP socket. This parser owns only negotiation bytes. */
SvContact *sv_contact_create(int server_protocol, const SvContactIdentity *identity);
void sv_contact_destroy(SvContact *contact);
SvResult sv_contact_receive(SvContact *contact, const void *bytes, size_t size);
SvOutput sv_contact_take_output(SvContact *contact, void *bytes, size_t capacity);
SvContactState sv_contact_state(const SvContact *contact);
unsigned sv_contact_rejection(const SvContact *contact);
SvContactState sv_contact_failure_phase(const SvContact *contact);
const int *sv_contact_version(const SvContact *contact);
uint32_t sv_contact_creation_flags(const SvContact *contact);
/* Borrowed until contact destruction; available only after complete setup. */
const SvContactSetup *sv_contact_setup(const SvContact *contact);
SvOutput sv_contact_take_remaining(SvContact *contact, void *bytes, size_t capacity);
#endif
