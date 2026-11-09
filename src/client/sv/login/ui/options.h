#ifndef SV_LOGIN_UI_OPTIONS_H
#define SV_LOGIN_UI_OPTIONS_H
#include <stdbool.h>
typedef struct {
    bool reduced_motion;
    bool background_selected;
    unsigned background;
    const char *text_font_path; /* resolved effective selection; NULL keeps B2 font */
} SvLoginShellOptions;
#endif
