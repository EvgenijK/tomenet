#ifndef SV_ENDPOINT_RUN_H
#define SV_ENDPOINT_RUN_H
#include "input/endpoint.h"
#include "session/pregame.h"
#include "session/birth.h"
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
    bool birth_illegal_ignored, birth_quit;
    bool birth_class_confirmed, birth_class_random_used, birth_class_dna_used;
    SvBirthOwner birth_owner;
    SvBirthSex birth_highlighted_sex, birth_sex;
    int birth_highlighted_class, birth_class;
    uint8_t birth_legal_classes[255], birth_legal_class_count;
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
    SvEndpointOutcome *outcome;
} SvEndpointOptions;
int sv_endpoint_run(SvEndpointOptions options);
#endif
