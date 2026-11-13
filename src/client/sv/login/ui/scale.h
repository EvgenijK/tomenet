#ifndef SV_LOGIN_UI_SCALE_H
#define SV_LOGIN_UI_SCALE_H
#include <SDL3/SDL.h>

#define SV_LOGIN_UI_SCALE_PROPERTY "sv.login.uiScalePercent"
#define SV_LOGIN_IGNORE_DPI_PROPERTY "sv.login.ignoreDisplayScale"
static inline bool sv_login_ui_ignore_dpi(SDL_Window *window, bool ignore)
{
    return window && SDL_SetBooleanProperty(SDL_GetWindowProperties(window),
                                            SV_LOGIN_IGNORE_DPI_PROPERTY,ignore);
}
static inline float sv_login_display_scale(SDL_Window *window)
{
    if (SDL_GetBooleanProperty(SDL_GetWindowProperties(window),SV_LOGIN_IGNORE_DPI_PROPERTY,false)) return 1;
    float display=SDL_GetWindowDisplayScale(window);
    return display>0?display:1;
}
/* Startup assigns the requested scale once; both rendering and pointer routing
 * derive logical UI coordinates from the same window-owned value. */
static inline bool sv_login_ui_set_scale(SDL_Window *window, int percent)
{
    if (!window || percent<50 || percent>200 || percent%5) return false;
    return SDL_SetNumberProperty(SDL_GetWindowProperties(window),SV_LOGIN_UI_SCALE_PROPERTY,percent);
}
static inline float sv_login_ui_scale(SDL_Window *window)
{
    if (!window) return 0;
    float display=sv_login_display_scale(window);
    Sint64 percent=SDL_GetNumberProperty(SDL_GetWindowProperties(window),SV_LOGIN_UI_SCALE_PROPERTY,100);
    return display*(float)percent/100.f;
}
#endif
