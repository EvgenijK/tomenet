/* Shared PKT_MESSAGE field decoder. Include after angband.h and sockbuf.h.
 * Raw unsigned byte identity is independent of the display font/profile. */
#ifndef CLIENT_MESSAGE_UPDATE_H
#define CLIENT_MESSAGE_UPDATE_H
static inline int client_decode_message(sockbuf_t *input, char out[MSG_LEN])
{
    /* The field must terminate inside its slot. Do not let Packet_scanf's
     * legacy truncation turn a remainder into another packet. No cursor or
     * output changes occur while waiting or on a malformed boundary. */
    size_t available = (size_t)(input->len - (input->ptr - input->buf));
    if (available < 2) return 0;
    size_t field = available - 1;
    if (field > MSG_LEN) field = MSG_LEN;
    if (!memchr(input->ptr + 1, 0, field)) return field == MSG_LEN ? -1 : 0;
    char type, next[MSG_LEN] = {0};
    int result = Packet_scanf(input, "%c%S", &type, next);
    if (result > 0) memcpy(out, next, MSG_LEN);
    return result;
}
#endif
