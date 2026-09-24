#ifndef SV_SESSION_H
#define SV_SESSION_H
#include <stdint.h>
#include "result.h"
#define SV_MESSAGE_BYTES 256
#define SV_MESSAGE_PENDING 64
#define SV_MESSAGE_LINES 6
typedef struct {
    uint64_t sequence; /* Session-local occurrence identity, never deduplicated. */
    size_t length;
    unsigned char bytes[SV_MESSAGE_BYTES]; /* Original field, including NUL. */
} SvMessage;
typedef struct {
    uint64_t revision;
    size_t count;
    SvMessage lines[SV_MESSAGE_LINES];
} SvMessages;
#define SV_REQUEST_BYTES 80
typedef struct {
    uint64_t sequence;
    int id, pending, aborted;
    unsigned char prompt[SV_REQUEST_BYTES];
} SvKeyRequest;
typedef struct SvSession SvSession;
typedef struct {
    int maximum, current;
    unsigned char drain;
    int bar, boosted;
} SvHpUpdate;
typedef struct {
    int maximum, current;
    unsigned char drain;
    int bar, boosted, available;
    uint64_t revision;
} SvStatus;
typedef struct { SvStatus before, after; } SvStatusChange;
typedef enum { SV_CHANGE_HP, SV_CHANGE_MESSAGE, SV_CHANGE_KEY_REQUEST,
               SV_CHANGE_REQUEST_ABORT, SV_CHANGE_NOOP, SV_CHANGE_SERVER_FLAGS,
               SV_CHANGE_CONFIRM, SV_CHANGE_PAUSE, SV_CHANGE_FLUSH,
               SV_CHANGE_PING } SvChangeKind;
typedef struct { int32_t rtt_ms; unsigned char index; } SvPingUpdate;
typedef struct {
    int samples[60], latest_ms, average_ms, average_count;
} SvPingTelemetry;
typedef struct {
    SvChangeKind kind;
    union { SvHpUpdate hp; SvMessage message; SvKeyRequest request;
            uint32_t server_flags[4]; unsigned char confirmed_command;
            SvPingUpdate ping; };
} SvChange;
typedef struct { uint32_t server_flags[4]; } SvControlState;
typedef struct {
    SvResult result;
    SvStatusChange status;
    uint64_t message_occurrence; /* Zero denotes a cleared feed. */
    int message_chat;
} SvSessionChange;
/* Model only. No wire version, transport or SDL dependency in this interface. */
SvSession *sv_session_create(void);
void sv_session_destroy(SvSession *session);
/* Each complete update advances its region revision, including repeated values. */
SvSessionChange sv_session_apply(SvSession *session, const SvChange *change);
SvStatus sv_session_status(const SvSession *session);
/* Bounded mandatory delivery and a separate short live feed (not full recall).
 * Taking an event acknowledges its delivery; drawing the feed never takes events. */
SvResult sv_session_take_message(SvSession *session, SvMessage *message);
SvMessages sv_session_messages(const SvSession *session);
SvKeyRequest sv_session_request(const SvSession *session);
SvControlState sv_session_controls(const SvSession *session);
SvResult sv_session_take_confirmation(SvSession *session, unsigned char *command);
void sv_session_ping_sent(SvSession *session);
SvPingTelemetry sv_session_ping(const SvSession *session);
SvResult sv_session_complete_request(SvSession *session, uint64_t sequence);
#endif
