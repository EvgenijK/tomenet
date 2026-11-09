#include "input/quantity.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static bool digit(unsigned char c) { return c >= '0' && c <= '9'; }
static bool letter(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static int64_t factor(unsigned char c)
{
    if (c == 'k' || c == 'K') return 1000;
    if (c == 'm' || c == 'M') return 1000000;
    if (c == 'g' || c == 'G') return 1000000000;
    return 1;
}
static int32_t parse(const SvQuantity *q)
{
    const unsigned char *p = (const unsigned char *)q->field.bytes;
    int64_t amount = 0;
    if (!*p) amount = q->predef < 0 ? SV_QUANTITY_ALL : q->predef;
    else if (letter(*p) && (factor(*p) == 1 || !digit(p[1])))
        amount = q->maximum < 0 ? SV_QUANTITY_ALL : q->maximum;
    else {
        int64_t scale = 1, tail = 0;
        if (factor(*p) > 1 && digit(p[1])) scale = factor(*p++);
        else {
            /* Ten bytes allow ten integer digits without a factor, or at
             * most nine before a factor. Even nine digits times 1e9 fit
             * int64_t, so parsing does not rely on signed overflow. */
            while (digit(*p)) amount = amount * 10 + (*p++ - '0');
            scale = factor(*p);
            if (scale > 1) p++;
        }
        if (scale > 1) {
            int places = scale == 1000 ? 3 : scale == 1000000 ? 6 : 9;
            int count = 0;
            /* Baseline reads at most six fractional digits, then pads to the
             * factor width (even giga's nine places). */
            while (digit(*p) && count < 6) {
                if (count < places) tail = tail * 10 + (*p - '0');
                p++;
                count++;
            }
            while (count++ < places) tail *= 10;
        }
        amount = amount * scale + tail;
    }
    if (q->maximum >= 0 && amount > q->maximum) amount = q->maximum;
    if (amount < 0) amount = 0;
    /* Baseline signed overflow has no defined result. Keep defined numeric
     * results exact and bound oversized input to the representable range. */
    if (amount > INT32_MAX) amount = INT32_MAX;
    return (int32_t)amount;
}
static void finish(SvQuantity *q, bool cancelled)
{
    q->value = cancelled ? 0 : parse(q);
    q->cancelled = cancelled;
    q->active = false;
    q->completed = true;
    q->all_shortcut = false;
}
void sv_quantity_reset(SvQuantity *q) { *q = (SvQuantity){0}; }
void sv_quantity_begin(SvQuantity *q, int32_t predef, int32_t maximum, bool all_shortcut)
{
    char initial[16];
    *q = (SvQuantity){.predef = predef, .maximum = maximum,
                      .all_shortcut = all_shortcut, .active = true};
    snprintf(initial, sizeof(initial), "%d", (int)predef);
    sv_text_begin(&q->field, initial, SV_QUANTITY_WIDTH, false);
}
SvTextResult sv_quantity_text(SvQuantity *q, const char *utf8)
{
    if (!q->active || !utf8) return SV_TEXT_INVALID;
    /* A single native text event represents the post-macro key that inkey
     * normalizes to Escape. Bulk inserted text is clipboard/editor content,
     * which askfor_aux copies without applying inkey to its interior bytes.
     * C053 consumes MACRO_WAIT tokens before invoking this primitive. */
    if (utf8[0] == '`' && !utf8[1]) {
        finish(q, true);
        return SV_TEXT_OK;
    }
    bool empty = !q->field.length ||
        ((q->field.anchor == 0 && q->field.cursor == q->field.length) ||
         (q->field.cursor == 0 && q->field.anchor == q->field.length));
    bool shortcut = q->all_shortcut && empty && utf8[0] && !utf8[1] &&
        (utf8[0] == 'a' || utf8[0] == 'A' || utf8[0] == ' ');
    SvTextResult result = sv_text_insert_utf8(&q->field, shortcut ? "a" : utf8);
    if (result == SV_TEXT_OK && shortcut) finish(q, false);
    return result;
}
void sv_quantity_edit(SvQuantity *q, SvEditKey key)
{
    if (!q->active) return;
    if (key == SV_EDIT_CANCEL) { finish(q, true); return; }
    if (key == SV_EDIT_ACCEPT) { finish(q, false); return; }
    sv_text_key(&q->field, key);
}
void sv_quantity_key(SvQuantity *q, unsigned char key)
{
    if (key == 27) sv_quantity_edit(q, SV_EDIT_CANCEL);
    else if (key == '\r' || key == '\n') sv_quantity_edit(q, SV_EDIT_ACCEPT);
    else if (key == 8 || key == 127) sv_quantity_edit(q, SV_EDIT_BACKSPACE);
    else if (key >= 32 && key < 127) {
        char text[2] = {(char)key, 0};
        (void)sv_quantity_text(q, text);
    }
}
bool sv_quantity_take(SvQuantity *q, int32_t *value)
{
    if (!q->completed || !value) return false;
    *value = q->value;
    q->completed = false;
    return true;
}
