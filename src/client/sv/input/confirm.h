#ifndef SV_CONFIRM_H
#define SV_CONFIRM_H
#include <stdbool.h>
typedef enum {
    SV_CONFIRM_STRICT, SV_CONFIRM_DEFAULT_YES, SV_CONFIRM_DEFAULT_NO,
    SV_CONFIRM_RETRY, SV_CONFIRM_TERNARY
} SvConfirmMode;
typedef enum { SV_CONFIRM_WAIT, SV_CONFIRM_YES, SV_CONFIRM_NO,
               SV_CONFIRM_RETRY_CHOICE, SV_CONFIRM_ALL } SvConfirmResult;
typedef struct {
    SvConfirmMode mode;
    bool default_no, completed, flush_input;
} SvConfirm;
void sv_confirm_begin(SvConfirm *confirm, SvConfirmMode mode, bool default_no);
SvConfirmResult sv_confirm_key(SvConfirm *confirm, unsigned char key);
#endif
