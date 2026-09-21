#ifndef SV_NATIVE_FRAME_H
#define SV_NATIVE_FRAME_H
#include "ui.h"
/* Test-only adapter: context is the shell's SvUi, borrowed for the submission. */
int sv_scenario_frame(void *context, SvAppView view);
#endif
