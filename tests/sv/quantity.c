#include "input/quantity.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

static int32_t answer(int32_t predef, int32_t max, const char *text)
{
    SvQuantity q;
    int32_t value;
    sv_quantity_begin(&q, predef, max, false);
    if (text) {
        sv_quantity_edit(&q, SV_EDIT_BACKSPACE);
        assert(sv_quantity_text(&q, text) == SV_TEXT_OK);
    }
    sv_quantity_key(&q, '\r');
    assert(!q.active && q.completed);
    assert(sv_quantity_take(&q, &value));
    assert(!sv_quantity_take(&q, &value));
    return value;
}
int main(void)
{
    assert(answer(7, 20, NULL) == 7);
    assert(answer(7, 20, "") == 7);
    assert(answer(-1, -1, "") == SV_QUANTITY_ALL);
    /* An untouched negative default is text, not accepted-empty. */
    assert(answer(-1, -1, NULL) == 0);
    assert(answer(7, 20, "0") == 0);
    assert(answer(7, 20, "-9") == 0);
    assert(answer(7, 20, "99") == 20);
    assert(answer(7, 20, "A") == 20);
    assert(answer(7, -1, "q") == SV_QUANTITY_ALL);
    assert(answer(7, 20, "mX") == 20);
    assert(answer(7, -1, "1m35") == 1350000);
    assert(answer(7, -1, "k5") == 500);
    assert(answer(7, -1, "M5") == 500000);
    assert(answer(7, -1, "1g123456") == 1123456000);
    assert(answer(7, -1, "1k123456") == 1123);
    assert(answer(7, -1, "9G") == INT32_MAX);
    assert(answer(7, -1, "12x") == 12);
    assert(answer(7, 0, "a") == 0);
    assert(answer(7, 20, "1`") == 1); /* Bulk paste is not key replay. */

    SvQuantity q;
    int32_t value;
    sv_quantity_begin(&q, 7, 20, true);
    sv_quantity_key(&q, ' ');
    assert(sv_quantity_take(&q, &value) && value == 20);
    sv_quantity_begin(&q, 7, 20, false);
    sv_quantity_key(&q, ' ');
    assert(q.active && !q.completed);
    sv_quantity_key(&q, '\r');
    assert(sv_quantity_take(&q, &value) && value == 0);
    sv_quantity_begin(&q, 7, 20, true);
    sv_quantity_key(&q, 'q');
    assert(q.active); /* Other letters require confirmation. */
    sv_quantity_key(&q, '\r');
    assert(sv_quantity_take(&q, &value) && value == 20);
    sv_quantity_begin(&q, 7, 20, true);
    sv_quantity_key(&q, 27);
    assert(q.cancelled && sv_quantity_take(&q, &value) && value == 0);

    sv_quantity_begin(&q, 7, 20, false);
    assert(sv_quantity_text(&q, "`") == SV_TEXT_OK);
    assert(q.cancelled && sv_quantity_take(&q, &value) && value == 0);
    sv_quantity_begin(&q, 7, 20, false);
    sv_quantity_key(&q, '`');
    assert(q.cancelled && sv_quantity_take(&q, &value) && value == 0);

    sv_quantity_begin(&q, 7, 20, false);
    assert(sv_quantity_text(&q, "12345678901") == SV_TEXT_TRUNCATED);
    assert(q.field.length == SV_QUANTITY_WIDTH);
    sv_quantity_reset(&q);
    sv_quantity_key(&q, '\r');
    assert(!q.active && !q.completed && !sv_quantity_take(&q, &value));
    assert(sv_quantity_text(&q, "1") == SV_TEXT_INVALID);
    sv_quantity_begin(&q, 1, 99, false);
    sv_quantity_key(&q, '8');
    sv_quantity_key(&q, '\r');
    assert(sv_quantity_take(&q, &value) && value == 8);
    puts("quantity: OK");
    return 0;
}
