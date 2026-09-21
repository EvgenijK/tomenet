/* Shared HP decode/apply boundary. Include after angband.h and sockbuf.h.
 * Static definitions keep all existing legacy build variants on the same path. */
#ifndef CLIENT_HP_UPDATE_H
#define CLIENT_HP_UPDATE_H

typedef struct {
    s16b maximum, current;
    bool bar, boosted;
    unsigned char drain;
} ClientHpUpdate;

static inline int client_decode_hp(sockbuf_t *input, version_type *version, ClientHpUpdate *out)
{
    char type, drain = 0;
    s16b maximum, current;
    ClientHpUpdate next;
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

static inline void client_apply_hp(player_type *player, const ClientHpUpdate *hp)
{
    player->mhp = hp->maximum;
    player->chp = hp->current;
}
#endif
