#ifndef SV_SESSION_PREGAME_H
#define SV_SESSION_PREGAME_H
#include "protocol/login.h"
#include "session/character-setup.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SV_PREGAME_CONTACT,
    SV_PREGAME_AUTHENTICATING,
    SV_PREGAME_OVERVIEW,
    SV_PREGAME_CHARACTER_PENDING,
    SV_PREGAME_MOTD,
    SV_PREGAME_LIVE_HANDOFF,
    SV_PREGAME_FAILED,
    SV_PREGAME_DISCONNECTED
} SvPregamePhase;

typedef struct {
    char name[SV_LOGIN_NAME_CAPACITY];
    char race[SV_CONTACT_NAME_CAPACITY];
    char class_title[SV_CONTACT_NAME_CAPACITY];
    char location[SV_LOGIN_NAME_CAPACITY];
    int16_t level, mode;
} SvPregameCharacter;

typedef struct {
    uint64_t generation, revision;
    SvPregamePhase phase;
    SvPregameCharacter characters[SV_LOGIN_MAX_CHARACTERS];
    size_t character_count;
    bool authenticated;
    char selected_character[SV_LOGIN_NAME_CAPACITY];
    uint32_t server_flags[4], creation_flags;
    unsigned char motd[SV_CONTACT_MOTD_CAPACITY + 1];
    size_t motd_size;
    char reason[256];
} SvPregame;

void sv_pregame_begin(SvPregame *pregame, uint64_t generation);
SvResult sv_pregame_contact_ready(SvPregame *pregame, uint64_t generation);
SvResult sv_pregame_sync_login(SvPregame *pregame, uint64_t generation,
                               const SvLogin *login, const SvContactSetup *setup,
                               bool motd_complete);
SvResult sv_pregame_disconnect(SvPregame *pregame, uint64_t generation,
                               const char *reason);
const char *sv_pregame_phase_name(SvPregamePhase phase);

#endif
