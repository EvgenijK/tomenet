#include "input/confirm.h"

void sv_confirm_begin(SvConfirm *confirm, SvConfirmMode mode, bool default_no)
{
    *confirm = (SvConfirm){.mode = mode, .default_no = default_no};
}

SvConfirmResult sv_confirm_key(SvConfirm *confirm, unsigned char key)
{
    if (confirm->completed) return SV_CONFIRM_WAIT;
    SvConfirmResult answer = SV_CONFIRM_WAIT;
    if (key == 'y' || key == 'Y') answer = SV_CONFIRM_YES;
    else if (key == 'n' || key == 'N') answer = SV_CONFIRM_NO;
    else if (confirm->mode == SV_CONFIRM_TERNARY && (key == 'a' || key == 'A')) answer = SV_CONFIRM_ALL;
    else if (confirm->mode == SV_CONFIRM_RETRY && (key == 'r' || key == 'R')) answer = SV_CONFIRM_RETRY_CHOICE;
    else if (confirm->mode == SV_CONFIRM_DEFAULT_YES) answer = SV_CONFIRM_YES;
    else if (confirm->mode == SV_CONFIRM_DEFAULT_NO) answer = SV_CONFIRM_NO;
    else if (confirm->mode == SV_CONFIRM_TERNARY && confirm->default_no &&
             (key == 27 || key == 13 || key == 10 || key == 17)) answer = SV_CONFIRM_NO;
    if (answer != SV_CONFIRM_WAIT) {
        confirm->completed = true;
        confirm->flush_input = true;
    }
    return answer;
}
