#ifndef SV_SESSION_H
#define SV_SESSION_H
#include <stdint.h>
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
/* Model only. No wire version, transport or SDL dependency in this interface. */
SvSession *sv_session_create(void);
void sv_session_destroy(SvSession *session);
/* Each complete update advances revision, including repeated values. */
SvStatusChange sv_session_apply_hp(SvSession *session, SvHpUpdate update);
SvStatus sv_session_status(const SvSession *session);
#endif
