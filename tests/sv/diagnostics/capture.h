#ifndef SV_DIAGNOSTIC_CAPTURE_H
#define SV_DIAGNOSTIC_CAPTURE_H
#include "login/ui/login-shell.h"
/* Configure the optional capture observer from the acceptance environment. */
void sv_diagnostic_capture_configure(void);
SvLoginShellOptions sv_diagnostic_shell_options(void);
#endif
