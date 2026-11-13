#ifndef SV_QUANTITY_H
#define SV_QUANTITY_H
#include <stdint.h>
#include "input/text-field.h"
#define SV_QUANTITY_WIDTH 10
#define SV_QUANTITY_ALL 2000000000
/* Owner-thread interaction. The caller owns generation and parent context.
 * active accepts input; completed retains one result until take/reset. */
typedef struct {
    SvTextField field;
    int32_t predef, maximum, value;
    bool all_shortcut, active, completed, cancelled;
} SvQuantity;
void sv_quantity_begin(SvQuantity *quantity, int32_t predef, int32_t maximum,
                       bool all_shortcut);
void sv_quantity_reset(SvQuantity *quantity);
SvTextResult sv_quantity_text(SvQuantity *quantity, const char *utf8);
void sv_quantity_edit(SvQuantity *quantity, SvEditKey key);
void sv_quantity_key(SvQuantity *quantity, unsigned char key);
bool sv_quantity_take(SvQuantity *quantity, int32_t *value);
#endif
