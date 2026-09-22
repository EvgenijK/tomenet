/* SV-local HP decode/apply boundary. Include after angband.h and sockbuf.h.
 * Matches the legacy baseline without adding dependencies to legacy builds. */
#ifndef SV_HP_UPDATE_H
#define SV_HP_UPDATE_H

typedef struct {
    s16b maximum, current;
    bool bar, boosted;
    unsigned char drain;
} SvDecodedHp;

static inline int sv_decode_hp(sockbuf_t *input, version_type *version, SvDecodedHp *out)
{
    char type, drain = 0;
    s16b maximum, current;
    SvDecodedHp next;
    int result;
    if (is_newer_than(version, 4, 7, 0, 2, 0, 1))
        result = Packet_scanf(input, "%c%hd%hd%c", &type, &maximum, &current, &drain);
    else
        result = Packet_scanf(input, "%c%hd%hd", &type, &maximum, &current);
    if (result <= 0) return result;
    next.bar = maximum > 10000;
    /* Keep the midpoint threshold: boosted HP may be negative after death. */
    next.boosted = current > 5000;
    next.maximum = maximum - (next.bar ? 10000 : 0);
    next.current = current - (next.boosted ? 10000 : 0);
    next.drain = (unsigned char)drain;
    *out = next;
    return 1;
}

static inline void sv_apply_hp(player_type *player, const SvDecodedHp *hp)
{
    player->mhp = hp->maximum;
    player->chp = hp->current;
}
#endif
