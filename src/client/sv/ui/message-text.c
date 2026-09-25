#include "ui/message-text.h"
#include <string.h>
SvMessageText sv_message_text(const SvMessage *message)
{
    SvMessageText out = {0};
    unsigned char color = 1, first = 1, previous = 1;
    int initial = 1;
    const char *palette = "dwsorgbuDWvyRGBU";
    for (size_t i = 0; i < message->length && i < SV_MESSAGE_BYTES - 1; ++i) {
        unsigned char c = message->bytes[i];
        if (c >= 252 && c <= 254) continue; /* Routing/recall markers, not glyphs. */
        if (c == 255) {
            if (++i >= message->length) break;
            c = message->bytes[i];
            if (c == 255) c = '{';
            else if (c == '-') { previous = color; color = first; continue; }
            else if (c == '.') { unsigned char swap = color; color = previous; previous = swap; continue; }
            else if (c == '%') continue;
            else {
                const char *found = c ? strchr(palette, c) : NULL;
                if (found) {
                    previous = color; color = (unsigned char)(found - palette);
                    if (initial) first = color;
                    continue;
                }
                /* Unknown code retains its symbol in the current color. */
            }
        }
        initial = 0;
        out.colors[out.length] = color;
        out.text[out.length++] = c >= 32 && c <= 126 ? (char)c : '?';
    }
    return out;
}

int sv_messages_prepare(SvMessageCache *cache, uint64_t generation, SvMessages messages)
{
    if (cache->valid && cache->generation == generation && cache->revision == messages.revision) return 0;
    *cache = (SvMessageCache){.generation = generation, .revision = messages.revision,
                              .valid = 1, .count = messages.count};
    for (size_t i = 0; i < messages.count; ++i) cache->lines[i] = sv_message_text(&messages.lines[i]);
    return 1;
}
