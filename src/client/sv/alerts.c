#include "alerts.h"
SvAlertEffects sv_alerts_evaluate(SvStatusChange change, SvAlertOptions options, SvAttention attention)
{
    SvAlertEffects effects = {{0}, 0};
    if (change.after.revision == change.before.revision) return effects;
    if (options.off_panel_damage && attention.off_panel &&
        change.after.current < change.before.current && !change.after.drain)
        effects.items[effects.count++] = SV_DAMAGE_SOUND;
    /* Preserve integer threshold and per-packet behavior of Receive_hp. */
    if (options.low_hp && change.after.current < change.after.maximum / 5) {
        effects.items[effects.count++] = SV_LOW_HP_SOUND;
        effects.items[effects.count++] = SV_LOW_HP_NOTICE;
    }
    return effects;
}
int sv_alerts_deliver(SvAlertSink sink, SvAlertEffects effects)
{
    int delivered = 1;
    for (unsigned i = 0; i < effects.count; ++i)
        if (!sink.deliver || !sink.deliver(sink.context, effects.items[i])) delivered = 0;
    return delivered;
}
