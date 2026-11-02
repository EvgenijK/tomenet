#ifndef SV_ENDPOINT_RUN_H
#define SV_ENDPOINT_RUN_H
#include "input/endpoint.h"
#include "ui/login-shell.h"
#include "session/pregame.h"
#include "input/birth-interaction.h"
typedef struct {
    SvEndpointPhase phase;
    char host[SV_HOST_LIMIT + 1];
    uint16_t port;
    int protocol;
} SvEndpointChoice;
typedef bool (*SvEndpointPoll)(void *context, SvEndpoint *endpoint, bool *failed);
typedef enum { SV_CREDENTIAL_SAVE_NOT_STARTED, SV_CREDENTIAL_SAVE_PENDING,
               SV_CREDENTIAL_SAVE_SAVED, SV_CREDENTIAL_SAVE_SESSION_ONLY }
    SvCredentialSaveState;
typedef struct {
    uint64_t generation, prior_generation, revision;
    SvPregamePhase phase;
    bool authenticated;
    size_t character_count;
    uint32_t server_flags[4], creation_flags;
    bool credential_save_started, credential_saved;
    SvCredentialSaveState credential_save;
    bool ordinary_name_started, ordinary_name_cancelled;
    bool exclusive_name_started, exclusive_name_cancelled;
    bool dedicated_creation;
    uint32_t dedicated_mode_flags;
    bool name_submitted, random_name_suggested, reincarnation_requested;
    bool name_input_truncated, name_input_method_used, name_keyboard_edited;
    bool name_mouse_selected, name_paste_used;
    size_t name_maximum_draft_bytes;
    SvPregamePhase name_submit_phase;
    uint64_t name_entry_generation, name_cancel_generation;
    uint64_t name_entry_overview_revision, name_cancel_overview_revision;
    bool name_rejected, name_recovery_started, name_recovery_draft_preserved;
    uint64_t name_rejected_generation, name_recovery_generation;
    char name_rejection_reason[256];
    bool birth_entered, birth_sex_confirmed, birth_keyboard_navigated;
    bool birth_mouse_navigated, birth_random_used, birth_dna_used;
    bool birth_illegal_ignored, birth_auto_reincarnation, birth_dna_available;
    bool birth_quit, birth_active, birth_closed;
    bool birth_class_confirmed, birth_class_random_used, birth_class_dna_used;
    bool birth_race_confirmed, birth_race_random_used, birth_race_dna_used;
    bool birth_trait_confirmed, birth_trait_random_used, birth_trait_dna_used;
    bool birth_trait_skipped;
    bool birth_body_confirmed, birth_body_random_used, birth_body_dna_used;
    bool birth_body_skipped;
    bool birth_stats_confirmed, birth_stats_random_used, birth_stats_dna_used;
    bool birth_stats_point_buy;
    bool birth_stats_confirmation_pending;
    bool birth_mode_confirmed, birth_mode_random_used, birth_mode_dna_used;
    bool birth_mode_ready;
    SvBirthOwner birth_owner;
    SvBirthSex birth_highlighted_sex, birth_sex;
    int birth_highlighted_class, birth_class;
    uint8_t birth_visible_classes[255], birth_visible_class_count;
    uint8_t birth_legal_classes[255], birth_legal_class_count;
    int birth_highlighted_race, birth_race;
    uint8_t birth_legal_races[255], birth_legal_race_count, birth_race_count;
    int birth_highlighted_trait, birth_trait;
    uint8_t birth_legal_traits[255], birth_legal_trait_count, birth_trait_count;
    SvBirthBody birth_highlighted_body, birth_body;
    SvBirthBody birth_legal_bodies[2];
    uint8_t birth_legal_body_count, birth_body_count;
    int birth_highlighted_stat;
    int16_t birth_stats[6];
    int16_t birth_committed_stats[6];
    uint8_t birth_legal_stats[6], birth_legal_stat_count, birth_stats_order_count;
    int birth_stats_remaining;
    SvBirthMode birth_highlighted_mode, birth_mode;
    SvBirthMode birth_legal_modes[7];
    uint8_t birth_legal_mode_count;
    unsigned birth_placeholder_open_count, birth_placeholder_close_count;
    uint32_t birth_placeholder_owner_mask;
    bool birth_placeholder_draft_preserved;
    char birth_placeholder_text[64];
    uint64_t birth_generation;
    char selected_character[SV_LOGIN_NAME_CAPACITY];
    char reason[256];
} SvEndpointOutcome;
typedef struct {
    const char *root, *library, *server, *server_list;
    int width, height, frames, windowed;
    unsigned port;
    const char *metaserver;
    SvEndpointChoice *selected_endpoint;
    SvEndpointPoll source_poll;
    void *source_context;
    const char *account, *password, *real_name, *character;
    bool skip_motd, retry_login_disabled;
    bool birth_dna_available;
    SvBirthSex birth_dna_sex;
    bool birth_dna_class_available;
    int birth_dna_class;
    bool birth_dna_race_available;
    int birth_dna_race;
    bool birth_dna_trait_available;
    int birth_dna_trait;
    bool birth_dna_body_available;
    SvBirthBody birth_dna_body;
    bool birth_dna_stats_available;
    int16_t birth_dna_stats[6];
    bool birth_dna_mode_available;
    SvBirthMode birth_dna_mode;
    SvEndpointOutcome *outcome;
    SvLoginShellOptions shell;
} SvEndpointOptions;
int sv_endpoint_run(SvEndpointOptions options);
#endif
