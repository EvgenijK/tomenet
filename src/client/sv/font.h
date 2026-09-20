#ifndef SV_FONT_H
#define SV_FONT_H
#include <SDL3/SDL.h>

typedef struct SvFont SvFont;
/* Scoped Stage A UI profile: ASCII text, no game-byte charset inference. */
SvFont *sv_font_open(const char *user_root, const char *library);
void sv_font_close(SvFont *font);
const char *sv_font_resource(const SvFont *font);
bool sv_font_draw(SvFont *font, SDL_Renderer *renderer, const char *text,
                  int x, int y, float output_scale, SDL_Color color);
#endif
