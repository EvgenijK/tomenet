#ifndef SV_SYNTHETIC_H
#define SV_SYNTHETIC_H
#include <SDL3/SDL.h>
#include "app.h"
/* Temporary isolated profile and prepared peer data, not live session startup. */
bool sv_synthetic_profile(const char *root);
bool sv_synthetic_start(SvApp *app);
SvAlertSink sv_synthetic_alert_sink(void);
#endif
