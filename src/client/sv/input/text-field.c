#include "input/text-field.h"
#include <string.h>

static bool word(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

static void wipe_private(void *memory, size_t size)
{
    volatile unsigned char *bytes = memory;
    while (size--) *bytes++ = 0;
}

void sv_text_begin(SvTextField *field, const char *initial, size_t limit, bool private_field)
{
    *field = (SvTextField){0};
    field->limit = limit > SV_TEXT_CAPACITY ? SV_TEXT_CAPACITY : limit;
    field->private_field = private_field;
    if (!initial) return;
    field->length = strlen(initial);
    if (field->length > field->limit) field->length = field->limit;
    memcpy(field->bytes, initial, field->length);
    field->cursor = field->length;
    field->anchor = 0; /* Baseline default is replaced by first ordinary input. */
}

void sv_text_begin_history(SvTextField *field, SvTextHistory *history,
                           const char *initial, size_t limit, bool private_field)
{
    sv_text_begin(field,initial,limit,private_field);
    if (!private_field && history) {
        field->shared_history = history;
        field->history_count = history->count > SV_TEXT_HISTORY ? SV_TEXT_HISTORY : history->count;
        memcpy(field->history,history->entries,field->history_count * sizeof(field->history[0]));
        field->history_cursor = field->search_cursor = field->history_count;
    }
}

bool sv_text_select(SvTextField *field, size_t start, size_t end)
{
    if (start > end || end > field->length) return false;
    field->anchor = start;
    field->cursor = end;
    return true;
}

static size_t selection_start(const SvTextField *field)
{
    return field->cursor < field->anchor ? field->cursor : field->anchor;
}
static size_t selection_end(const SvTextField *field)
{
    return field->cursor > field->anchor ? field->cursor : field->anchor;
}

static void erase(SvTextField *field, size_t start, size_t end)
{
    memmove(field->bytes + start, field->bytes + end, field->length - end + 1);
    field->length -= end - start;
    field->cursor = field->anchor = start;
    field->dirty = true;
    field->preview[0] = 0;
    field->search_cursor = field->history_count;
}

/* SDL text/clipboard is UTF-8. Latin-1 has an exact byte mapping; no guessed
 * transliteration or partial multibyte sequence may enter a protocol field. */
static SvTextResult decode(const char *utf8, size_t length, char *out, size_t *count)
{
    *count = 0;
    for (size_t i = 0; i < length;) {
        unsigned char c = (unsigned char)utf8[i++];
        if (c >= 0x20 && c <= 0x7e) out[(*count)++] = (char)c;
        else if (c == 0xc2 && i < length) {
            unsigned char next = (unsigned char)utf8[i++];
            if (next < 0xa0 || next > 0xbf) return SV_TEXT_ENCODING_ERROR;
            out[(*count)++] = (char)next;
        } else if (c == 0xc3 && i < length) {
            unsigned char next = (unsigned char)utf8[i++];
            if (next < 0x80 || next > 0xbf) return SV_TEXT_ENCODING_ERROR;
            out[(*count)++] = (char)(next + 0x40);
        } else return SV_TEXT_ENCODING_ERROR;
    }
    return SV_TEXT_OK;
}

SvTextResult sv_text_paste(SvTextField *field, const char *utf8, size_t length)
{
    if (!utf8) return SV_TEXT_INVALID;
    if (length > 4096) return SV_TEXT_INVALID;
    char decoded[4096];
    size_t count;
    SvTextResult result = decode(utf8, length, decoded, &count);
    if (result != SV_TEXT_OK) {
        if (field->private_field) wipe_private(decoded, sizeof(decoded));
        return result;
    }
    size_t start = selection_start(field), end = selection_end(field);
    size_t capacity = field->limit - (field->length - (end - start));
    if (count > capacity) { count = capacity; result = SV_TEXT_TRUNCATED; }
    if (!count && start == end) {
        if (field->private_field) wipe_private(decoded, sizeof(decoded));
        return result;
    }
    memmove(field->bytes + start + count, field->bytes + end, field->length - end + 1);
    memcpy(field->bytes + start, decoded, count);
    field->length = field->length - (end - start) + count;
    field->cursor = field->anchor = start + count;
    field->dirty = true;
    field->preview[0] = 0;
    field->search_cursor = field->history_count;
    if (field->private_field) wipe_private(decoded, sizeof(decoded));
    return result;
}

SvTextResult sv_text_insert_utf8(SvTextField *field, const char *utf8)
{
    if (!utf8) return SV_TEXT_INVALID;
    return sv_text_paste(field, utf8, strlen(utf8));
}

void sv_text_remember(SvTextField *field)
{
    if (field->private_field || !field->length) return;
    size_t slot;
    if (field->history_count < SV_TEXT_HISTORY) slot = field->history_count++;
    else {
        memmove(field->history, field->history + 1, (SV_TEXT_HISTORY - 1) * sizeof(field->history[0]));
        slot = SV_TEXT_HISTORY - 1;
    }
    memcpy(field->history[slot], field->bytes, field->length + 1);
    field->history_cursor = field->history_count;
    if (field->shared_history) {
        field->shared_history->count = field->history_count;
        memcpy(field->shared_history->entries,field->history,
               field->history_count * sizeof(field->history[0]));
    }
}

static unsigned char folded(unsigned char c)
{
    return c >= 'A' && c <= 'Z' ? (unsigned char)(c + ('a' - 'A')) : c;
}

static bool matches(const char *entry, const char *pattern)
{
    const char *from = entry;
    while (*pattern) {
        char segment[SV_TEXT_CAPACITY + 1];
        size_t length = 0;
        while (*pattern && *pattern != '*') segment[length++] = *pattern++;
        if (*pattern == '*') ++pattern;
        if (!length) continue;
        bool found = false;
        for (const char *candidate = from; *candidate; ++candidate) {
            size_t j = 0;
            while (j < length && candidate[j] &&
                   folded((unsigned char)candidate[j]) == folded((unsigned char)segment[j])) ++j;
            if (j == length) { from = candidate + j; found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

static void search(SvTextField *field)
{
    field->preview[0] = 0;
    if (!field->history_count || !field->length) return;
    size_t cursor = field->search_cursor;
    for (size_t attempt = 0; attempt < field->history_count; ++attempt) {
        size_t index = (cursor + field->history_count - 1 - attempt) % field->history_count;
        if (matches(field->history[index],field->bytes)) {
            strcpy(field->preview,field->history[index]);
            field->search_cursor = index;
            return;
        }
    }
    field->search_cursor = field->history_count;
}

void sv_text_key(SvTextField *field, SvEditKey key)
{
    size_t start = selection_start(field), end = selection_end(field);
    switch (key) {
    case SV_EDIT_LEFT: field->cursor = field->anchor = start == end && start ? start - 1 : start; break;
    case SV_EDIT_RIGHT: field->cursor = field->anchor = start == end && end < field->length ? end + 1 : end; break;
    case SV_EDIT_HOME: field->cursor = field->anchor = 0; break;
    case SV_EDIT_END: field->cursor = field->anchor = field->length; break;
    case SV_EDIT_WORD_LEFT:
        while (start && !word((unsigned char)field->bytes[start - 1])) --start;
        while (start && word((unsigned char)field->bytes[start - 1])) --start;
        field->cursor = field->anchor = start; break;
    case SV_EDIT_WORD_RIGHT:
        while (end < field->length && word((unsigned char)field->bytes[end])) ++end;
        while (end < field->length && !word((unsigned char)field->bytes[end])) ++end;
        field->cursor = field->anchor = end; break;
    case SV_EDIT_BACKSPACE:
        if (start != end) erase(field, start, end);
        else if (start) erase(field, start - 1, start);
        break;
    case SV_EDIT_DELETE:
        if (start != end) erase(field, start, end);
        else if (start < field->length) erase(field, start, start + 1);
        break;
    case SV_EDIT_WORD_DELETE:
        if (start == end) {
            while (start && !word((unsigned char)field->bytes[start - 1])) --start;
            while (start && word((unsigned char)field->bytes[start - 1])) --start;
        }
        erase(field, start, end); break;
    case SV_EDIT_HISTORY_BACK:
    case SV_EDIT_HISTORY_FORWARD:
        if (!field->private_field && field->history_count) {
            if (key == SV_EDIT_HISTORY_BACK && field->history_cursor) --field->history_cursor;
            if (key == SV_EDIT_HISTORY_FORWARD && field->history_cursor + 1 < field->history_count)
                ++field->history_cursor;
            if (field->history_cursor >= field->history_count) break;
            const char *entry = field->history[field->history_cursor];
            field->length = strlen(entry);
            if (field->length > field->limit) field->length = field->limit;
            memcpy(field->bytes, entry, field->length);
            field->bytes[field->length] = 0;
            field->cursor = field->anchor = field->length;
        }
        break;
    case SV_EDIT_SEARCH:
        if (!field->private_field && field->history_count) {
            field->searching = true;
            search(field);
        }
        break;
    case SV_EDIT_ACCEPT:
        if (field->searching) {
            field->searching = false;
            if (field->preview[0]) {
                field->length = strlen(field->preview);
                if (field->length > field->limit) field->length = field->limit;
                memcpy(field->bytes,field->preview,field->length);
                field->bytes[field->length] = 0;
                field->cursor = field->anchor = field->length;
            }
            field->preview[0] = 0;
        } else { field->accepted = true; sv_text_remember(field); }
        break;
    case SV_EDIT_CANCEL:
        if (field->searching) { field->searching = false; field->preview[0] = 0; field->bytes[0] = 0;
            field->length = field->cursor = field->anchor = 0; }
        else field->cancelled = true;
        break;
    }
}
