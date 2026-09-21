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
typedef enum { SV_CHANGE_HP, SV_CHANGE_MESSAGE } SvChangeKind;
typedef struct {
    SvChangeKind kind;
    union { SvHpUpdate hp; SvMessage message; };
} SvChange;
typedef struct { SvResult result; SvStatusChange status; } SvSessionChange;
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
#endif
