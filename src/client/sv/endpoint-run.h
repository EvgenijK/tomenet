#ifndef SV_ENDPOINT_RUN_H
#define SV_ENDPOINT_RUN_H
#include "input/endpoint.h"
#include "session/pregame.h"
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
    bool skip_motd;
    SvEndpointOutcome *outcome;
} SvEndpointOptions;
int sv_endpoint_run(SvEndpointOptions options);
#endif
