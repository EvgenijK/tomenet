#ifndef SV_LOGIN_FLOW_INTERNAL_H
#define SV_LOGIN_FLOW_INTERNAL_H
#include "login/flow.h"
#include "login/characters/login-interaction.h"
#include "protocol/contact-socket.h"
#include "credential/vault.h"
#include "files/history.h"
#include "files/dna.h"
#include "login/preparation-internal.h"
typedef struct {
    unsigned char sending[32 * 1024];
    size_t size, sent;
    unsigned char *remaining;
    size_t remaining_size, remaining_at;
} SvSessionWire;

typedef struct {
    bool started, cancelled, submitted, random_suggested, reincarnation_requested;
    bool dedicated;
    bool input_truncated, input_method_used, keyboard_edited;
    bool mouse_selected, paste_used;
    size_t maximum_draft_bytes;
    SvPregamePhase submit_phase;
    uint64_t entry_generation, cancel_generation;
    uint64_t entry_revision, cancel_revision;
} SvOrdinaryNameEvidence;

typedef struct {
    bool pending, rejected, started, draft_preserved;
    bool dedicated;
    uint64_t rejected_generation, recovery_generation;
    char draft[SV_LOGIN_NAME_CAPACITY];
    char reason[256];
} SvNameRecovery;

typedef struct {
    unsigned open_count, close_count;
    uint32_t owner_mask;
    bool draft_preserved;
    char text[64];
} SvBirthPlaceholderEvidence;

typedef struct {
    SvContactSocket *connection;
    SvInputHistory *history;
    /* Character records/effects belong to this attempt's generation. */
    SvPreferences *preferences;
    bool dna_saved;
    SvSocketState state;
    SvResult connection_failure, session_error;
    unsigned char iaddr[6];
    bool finished;
    const char *temporary_surface_text;
    SvLogin *login;
    SvLoginTransfer transfer;
    SvTransferSummary transfer_result;
    SvLuaResult lua_begin_result;
    SvLoginInteraction login_input;
    SvPregame pregame;
    SvBirth *birth;
    SvVaultRequest *save;
    SvSessionWire wire;
    unsigned rejection;
    int result;
    bool reached_ready, reported_failure, birth_started, play_started, settings_applied, audio_reported, font_reported;
    uint64_t birth_input_since_ns;
    bool birth_placeholder_open, birth_placeholder_drawn;
    SvBirthOwner birth_placeholder_owner;
    SvBirthPlaceholderEvidence birth_placeholder;
    bool save_started, save_failed;
    SvCredentialSaveState credential_save;
    SvOrdinaryNameEvidence name_evidence;
    bool selected_reported, handoff_reported, character_name_rejected;
    const char *save_message, *status;
    char failure_status[384];
} SvLoginAttempt;
struct SvLoginFlow {
    SvEndpointOptions options;
    SvEndpoint endpoint;
    char account[80], password[80], initial_account[80];
    char previous_character[SV_LOGIN_NAME_CAPACITY];
    const char *profile_root, *font_resource;
    bool first_run, interactive_credentials;
    uint64_t generation, prior_generation, input_epoch;
    SvNameRecovery recovery;
    SvLoginAttempt attempt;
    SvSessionPreparation *preparation;
};
enum { SV_CONTACT_SCENE_BACK = 3, SV_CONTACT_SCENE_EDIT_NAME = 4 };
/* Private native adapters operate on the owned interactions synchronously. */
void sv_login_flow_login_result(SvLoginFlow *, SvLoginInputResult);
void sv_login_flow_birth_result(SvLoginFlow *, SvBirthInputResult);
void sv_login_flow_finish_tick(SvLoginFlow *);
void sv_login_flow_end_attempt(SvLoginFlow *);
#endif
