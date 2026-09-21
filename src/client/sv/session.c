#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../common/angband.h"
#include "../../common/sockbuf.h"
#include "../hp-update.h"
#include "session.h"
struct SvSession {
    player_type player; /* Sole HP value storage. */
    unsigned char drain;
    bool bar, boosted;
    uint64_t revision;
};
SvSession *sv_session_create(void) { return calloc(1, sizeof(SvSession)); }
void sv_session_destroy(SvSession *s) { free(s); }
SvStatus sv_session_status(const SvSession *s)
{
    SvStatus view = {s->player.mhp, s->player.chp, s->drain,
                    s->bar, s->boosted, s->revision != 0, s->revision};
    return view;
}
SvStatusChange sv_session_apply_hp(SvSession *s, SvHpUpdate update)
{
    SvStatusChange change;
    ClientHpUpdate hp = {update.maximum, update.current, update.bar,
                         update.boosted, update.drain};
    change.before = sv_session_status(s);
    client_apply_hp(&s->player, &hp);
    s->bar = hp.bar; s->boosted = hp.boosted; s->drain = hp.drain;
    ++s->revision;
    change.after = sv_session_status(s);
    return change;
}
