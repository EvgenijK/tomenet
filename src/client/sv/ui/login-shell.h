#ifndef SV_LOGIN_SHELL_H
#define SV_LOGIN_SHELL_H
#include <SDL3/SDL.h>
#include "ui/motd.h"
#include "input/native-endpoint.h"
bool sv_login_shell_open(SDL_Renderer *renderer, const char *library);
bool sv_login_shell_reset(SDL_Renderer *renderer);
bool sv_login_shell_torches(SDL_Renderer *renderer, float left, float right, float top, float scale);
/* Shared presentation geometry, in logical UI units. */
bool sv_login_shell_roundrect(SDL_Renderer *renderer,SDL_FRect rect,float radius,
                              float scale,SDL_Color color,bool outline);
bool sv_login_shell_measure(SDL_Renderer *renderer,const char *value,float size,
                            float scale,bool heading,float *width);
/* Public, non-secret presentation text only (cached). */
bool sv_login_shell_text(SDL_Renderer *renderer, const char *value, float x, float y,
                         float size, float scale, SDL_Color color);
bool sv_login_shell_heading(SDL_Renderer *renderer,const char *value,float x,float y,
                            float size,float scale,SDL_Color color);
bool sv_login_shell_motd(SDL_Renderer *renderer,const SvMotd *motd);
bool sv_login_shell_render(SDL_Renderer *renderer, const SvEndpoint *endpoint, const SvEndpointInput *input);
#endif
