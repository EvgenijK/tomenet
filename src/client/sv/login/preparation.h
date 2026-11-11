#ifndef SV_LOGIN_PREPARATION_H
#define SV_LOGIN_PREPARATION_H
#include "app.h"
#include "protocol/startup-prefix.h"
#include "protocol/contact-socket.h"
#include "files/history.h"
#include "lua/reload.h"
#include "transfer/transfer.h"
#include "settings/startup-ins.h"
#include "settings/graphics.h"
#ifndef SV_SESSION_PREPARATION_DECLARED
#define SV_SESSION_PREPARATION_DECLARED
typedef struct SvSessionPreparation SvSessionPreparation;
#endif
typedef struct {
    char requested_text_font[SV_SETTINGS_TEXT_MAX];
    char requested_map_font[SV_SETTINGS_TEXT_MAX];
    char requested_tiles[SV_SETTINGS_TEXT_MAX];
    char effective_text_font[SV_FILE_PATH], effective_tiles[SV_FILE_PATH];
    char requested_sound_pack[SV_SETTINGS_TEXT_MAX], requested_music_pack[SV_SETTINGS_TEXT_MAX];
    int requested_graphics, effective_graphics;
    int requested_sound_subset, requested_music_subset;
    SvMapFont map_font; /* Owned value, including source/result and definitions. */
} SvPreparationResources;
typedef struct {
    uint64_t generation, app_generation;
    /* Current preparation state, not a claim that the server will never send
     * another asynchronous CHECK/INIT. Known transfers must be inactive; all
     * pending output remains owned until its ordered drain. */
    bool early_profile, startup_complete, identity_ready, profile_ready;
    bool runtime_ready, guide_valid, guide_required, transfer_failed, complete;
    SvLuaStatus guide_status;
    char failure_reason[160]; /* baseline MAX_CHARS_WIDE, including terminator */
    int version[6];
    uint32_t server_flags[4];
    unsigned unsupported_packet;
    size_t retained_input, pending_output, deferred_frames, deferred_bytes;
    uint64_t ingress_sequence;
    SvStartupPrefixConfig prefix_config;
    SvLuaResult early_init, begin;
    SvTransferSummary transfers;
    SvSettingsAudioSnapshot audio;
    SvAppView presentation;
} SvPreparationView;
/* Inspection never reloads, consumes effects or transfers resources. */
SvPreparationView sv_session_preparation_view(const SvSessionPreparation *);
const SvPreferences *sv_session_preparation_preferences(const SvSessionPreparation *, bool application);
const SvInputHistory *sv_session_preparation_history(const SvSessionPreparation *);
const SvStartupIns *sv_session_preparation_inscriptions(const SvSessionPreparation *);
SvApp *sv_session_preparation_app(SvSessionPreparation *);
/* Stable owned snapshot; native resources stay alive until preparation destroy.
 * Graphics is the existing prepared owner, borrowed by future map consumers. */
const SvPreparationResources *sv_session_preparation_resources(const SvSessionPreparation *);
const SvMapFont *sv_session_preparation_map_font(const SvSessionPreparation *);
SvGraphics *sv_session_preparation_graphics(SvSessionPreparation *);
const SvLuaReload *sv_session_preparation_lua(const SvSessionPreparation *);
/* Each source/index identifies one original load occurrence. Ack affects only
 * that recipient; options/messages/actions cannot clear one another's work. */
SvResult sv_session_preparation_ack(SvSessionPreparation *, bool application, size_t index);
bool sv_session_preparation_pending(const SvSessionPreparation *, bool application, size_t index);
/* Transport remains owned by this handle; C014 may adopt the handle and use
 * these drains. Existing SvApp keeps any partially decoded packet intact. */
SvContactSocket *sv_session_preparation_connection(SvSessionPreparation *);
/* Subsequent CHECK/SUM/INIT traffic uses the very same C010 registry. There is
 * no CHECK-batch terminator: a future INIT after this boundary belongs to C014.
 * Call pump on ticks, even without incoming bytes; drain output in wire order. */
SvResult sv_session_preparation_receive_file(SvSessionPreparation *, uint64_t generation,
    const void *, size_t, size_t *consumed);
SvResult sv_session_preparation_pump(SvSessionPreparation *, uint64_t generation);
/* Complete unapplied startup frames, FIFO, before the raw tail. Sequence gaps
 * are packets already applied by SvApp (VERSION replies/messages/controls/
 * identity); their effects must never be replayed. Drains never run effects. */
#define SV_PREPARATION_DEFERRED_BYTES (256u * 1024u)
#define SV_PREPARATION_DEFERRED_FRAMES 2048u
typedef struct { uint64_t sequence; unsigned packet; size_t size; } SvPreparationFrame;
SvOutput sv_session_preparation_take_deferred(SvSessionPreparation *, SvPreparationFrame *, void *, size_t);
/* True for already executed app/C010 occurrences; deferred entries stay false
 * even after their byte drain (the caller then owns their application). */
bool sv_session_preparation_was_applied(const SvSessionPreparation *, uint64_t sequence);
/* BUSY until deferred frames are drained. Partly decoded SvApp state remains
 * in the retained app and is never duplicated into this raw tail. */
SvOutput sv_session_preparation_take_input(SvSessionPreparation *, void *, size_t);
SvOutput sv_session_preparation_take_output(SvSessionPreparation *, void *, size_t);
void sv_session_preparation_destroy(SvSessionPreparation *);
#endif
