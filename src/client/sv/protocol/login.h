#ifndef SV_PROTOCOL_LOGIN_H
#define SV_PROTOCOL_LOGIN_H
#include "result.h"
#include "settings/options.h"
#include "settings/audio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SV_LOGIN_MAX_CHARACTERS 16
#define SV_LOGIN_NAME_CAPACITY 80
typedef enum { SV_LOGIN_WAIT_FLAGS, SV_LOGIN_WAIT_LIST, SV_LOGIN_OVERVIEW,
               SV_LOGIN_WAIT_STATUS, SV_LOGIN_SELECTED, SV_LOGIN_WAIT_PLAY, SV_LOGIN_PLAY_ACKNOWLEDGED,
               SV_LOGIN_REJECTED, SV_LOGIN_PEER_QUIT } SvLoginState;
typedef struct {
    char name[SV_LOGIN_NAME_CAPACITY];
    char colour[8];
    char location[SV_LOGIN_NAME_CAPACITY];
    int16_t mode, level, race, class_id;
} SvLoginCharacter;
typedef struct SvLogin SvLogin;
typedef struct {
    int samples[60], latest_ms, average_ms, average_count;
} SvLoginPingTelemetry;

SvLogin *sv_login_create(const int version[6], const unsigned char iaddr[6]);
void sv_login_destroy(SvLogin *login);
/* Copies option values before the first Net_start packet. */
SvResult sv_login_set_options(SvLogin *login, const SvSettingsOptions *options);
/* Early baseline Send_options, after character selection and before birth/play. */
SvResult sv_login_send_options(SvLogin *login);
/* Initial prepared audio snapshot; full local resource names stay with owner. */
SvResult sv_login_set_audio(SvLogin *login, const SvSettingsAudioSnapshot *audio);
/* Baseline Send_audio for the playing protocol; old unsupported versions no-op. */
SvResult sv_login_send_audio(SvLogin *login);
/* Current effective map/graphics report; missing until actual consumer exists. */
SvResult sv_login_send_font(SvLogin *login);
SvOutput sv_login_take_output(SvLogin *login, void *bytes, size_t capacity);
SvResult sv_login_receive(SvLogin *login, const void *bytes, size_t size);
SvResult sv_login_choose(SvLogin *login, size_t slot);
SvResult sv_login_create_character(SvLogin *login, const char *name,
                                   bool reincarnation_requested);
/* Final creation request, accepted only once after a committed birth draft.
 * font is the effective map-font name, never the UI text font; current native
 * caller supplies empty while no actual map consumer exists. */
SvResult sv_login_start_play(SvLogin *login, int16_t mode, int16_t race,
                              int16_t class_id, int16_t trait,
                              const int16_t stats[6], const char *font);
SvResult sv_login_keepalive(SvLogin *login);
SvLoginState sv_login_state(const SvLogin *login);
/* END closes startup decoding. Coalesced live bytes remain available for
 * transfer; callers must stop reading the login transport at this boundary. */
bool sv_login_play_stream_complete(const SvLogin *login);
SvOutput sv_login_take_remaining(SvLogin *login, void *bytes, size_t capacity);
size_t sv_login_count(const SvLogin *login);
const SvLoginCharacter *sv_login_character(const SvLogin *login, size_t slot);
const SvLoginCharacter *sv_login_selected_character(const SvLogin *login);
bool sv_login_creating_character(const SvLogin *login);
bool sv_login_reincarnation_requested(const SvLogin *login);
const uint32_t *sv_login_flags(const SvLogin *login);
const int *sv_login_version(const SvLogin *login);
const char *sv_login_reason(const SvLogin *login);
SvLoginPingTelemetry sv_login_ping(const SvLogin *login);
#endif
