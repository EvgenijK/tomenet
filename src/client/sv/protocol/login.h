#ifndef SV_PROTOCOL_LOGIN_H
#define SV_PROTOCOL_LOGIN_H
#include "result.h"
#include "settings/options.h"
#include "settings/audio.h"
#include "protocol/visuals.h"
#include "protocol/startup-prefix.h"
#include "transfer/transfer.h"
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
/* Explicit optional production attachment. The login decoder owns input/output;
 * context is borrowed until detach/destroy. Existing standalone login users do
 * not need a file or Lua implementation linked into their process. */
typedef struct {
    void *context;
    SvResult (*flags)(void *, const uint32_t flags[4]);
    SvResult (*receive)(void *, const void *, size_t, size_t *);
    SvResult (*pump)(void *);
    SvOutput (*take_output)(void *, void *, size_t);
    SvTransferSummary (*summary)(const void *);
    SvLuaResult (*begin_result)(const void *);
    SvResult (*upload)(void *, const char *);
} SvLoginTransfer;
typedef struct {
    void *context;
    bool (*create)(void *, uint64_t generation, const int version[6], SvLoginTransfer *);
    void (*destroy)(SvLoginTransfer *);
} SvLoginTransferFactory;
SvResult sv_login_attach_transfer(SvLogin *, SvLoginTransfer);
void sv_login_detach_transfer(SvLogin *);
SvTransferSummary sv_login_transfer_summary(const SvLogin *);
SvLuaResult sv_login_lua_begin_result(const SvLogin *);
SvResult sv_login_upload(SvLogin *, const char *filename);
/* Pump sender and resume owned buffered input, even without a socket read. */
SvResult sv_login_tick(SvLogin *);
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
/* Effective font definitions are copied, never borrowed from a rendering cache.
 * Live update appends setup+font atomically; backpressure publishes neither. */
SvResult sv_login_set_visuals(SvLogin *,const SvVisualDefinitions *,const char *font);
SvResult sv_login_update_visuals(SvLogin *,const SvVisualDefinitions *,const char *font);
/* Prepared graphics/font stack. Requested references stay with the resource
 * owner; existing report serializers retain their byte limits and gates. */
SvResult sv_login_set_rendering(SvLogin *, const SvVisualDefinitions *, int graphics,
    const char *tiles, const char *font);
SvResult sv_login_update_rendering(SvLogin *, const SvVisualDefinitions *, int graphics,
    const char *tiles, const char *font);
SvOutput sv_login_take_output(SvLogin *login, void *bytes, size_t capacity);
/* Assembly forwards post-END app replies into the same ordered output queue.
 * Reserve before consuming the app output; failure consumes nothing. */
size_t sv_login_output_capacity(const SvLogin *);
SvResult sv_login_queue_prepared_output(SvLogin *, const void *, size_t);
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
/* Exact effective PLAY rendering context, copied for post-END framing. */
SvStartupPrefixConfig sv_login_startup_prefix_config(const SvLogin *);
const char *sv_login_reason(const SvLogin *login);
SvLoginPingTelemetry sv_login_ping(const SvLogin *login);
#endif
