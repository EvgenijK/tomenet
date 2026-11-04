#ifndef SV_ALERTS_H
#define SV_ALERTS_H
#include "session/session.h"
typedef enum { SV_DAMAGE_SOUND, SV_LOW_HP_SOUND, SV_LOW_HP_NOTICE } SvAlert;
typedef struct {
    void *context;
    /* Nonblocking delivery; 0 means optional executor failure, not session failure. */
    int (*deliver)(void *context, SvAlert alert);
} SvAlertSink;
typedef struct { int low_hp, off_panel_damage; } SvAlertOptions;
/* Explicit semantic context; not inferred from SDL focus or visibility. */
typedef struct { int off_panel; } SvAttention;
/* At most damage sound, low-HP sound and low-HP notice, in baseline order. */
typedef struct { SvAlert items[3]; unsigned count; } SvAlertEffects;
SvAlertEffects sv_alerts_evaluate(SvStatusChange change, SvAlertOptions options, SvAttention attention);
/* Returns false if any optional delivery failed; all effects are attempted. */
int sv_alerts_deliver(SvAlertSink sink, SvAlertEffects effects);
#endif
