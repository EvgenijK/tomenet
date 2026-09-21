/* Shared baseline key-request decoder and response serializer.
 * Include after angband.h, sockbuf.h and pack.h. */
#ifndef CLIENT_KEY_REQUEST_H
#define CLIENT_KEY_REQUEST_H
static inline int client_decode_key_request(sockbuf_t *input, int *id, char prompt[MAX_CHARS])
{
    size_t available = (size_t)(input->len - (input->ptr - input->buf));
    if (available < 6) return 0;
    size_t field = available - 5;
    if (field > MAX_CHARS) field = MAX_CHARS;
    if (!memchr(input->ptr + 5, 0, field)) return field == MAX_CHARS ? -1 : 0;
    char type, next[MAX_CHARS] = {0};
    int next_id;
    int result = Packet_scanf(input, "%c%d%s", &type, &next_id, next);
    if (result > 0) { *id = next_id; memcpy(prompt, next, MAX_CHARS); }
    return result;
}
static inline int client_send_key_reply(sockbuf_t *output, int id, unsigned char key)
{
    return Packet_printf(output, "%c%d%c", PKT_REQUEST_KEY, id, key);
}
#endif
