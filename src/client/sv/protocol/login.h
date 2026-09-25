#ifndef SV_PROTOCOL_LOGIN_H
#define SV_PROTOCOL_LOGIN_H
#include "result.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SV_LOGIN_MAX_CHARACTERS 16
#define SV_LOGIN_NAME_CAPACITY 80
typedef enum { SV_LOGIN_WAIT_FLAGS, SV_LOGIN_WAIT_LIST, SV_LOGIN_OVERVIEW,
               SV_LOGIN_WAIT_STATUS, SV_LOGIN_SELECTED, SV_LOGIN_REJECTED } SvLoginState;
typedef struct {
    char name[SV_LOGIN_NAME_CAPACITY];
    char colour[8];
    char location[SV_LOGIN_NAME_CAPACITY];
    int16_t mode, level, race, class_id;
} SvLoginCharacter;
typedef struct SvLogin SvLogin;

SvLogin *sv_login_create(const int version[6], const unsigned char iaddr[6]);
void sv_login_destroy(SvLogin *login);
SvOutput sv_login_take_output(SvLogin *login, void *bytes, size_t capacity);
SvResult sv_login_receive(SvLogin *login, const void *bytes, size_t size);
SvResult sv_login_choose(SvLogin *login, size_t slot);
SvLoginState sv_login_state(const SvLogin *login);
size_t sv_login_count(const SvLogin *login);
const SvLoginCharacter *sv_login_character(const SvLogin *login, size_t slot);
const uint32_t *sv_login_flags(const SvLogin *login);
const char *sv_login_reason(const SvLogin *login);
#endif
