#ifndef SV_FONT_H
#define SV_FONT_H
#include <SDL3/SDL.h>

typedef struct SvFont SvFont;
/* Scoped Stage A UI profile: ASCII text, no game-byte charset inference. */
SvFont *sv_font_open(const char *user_root, const char *library);
SvFont *sv_font_open_selected(const char *user_root, const char *library, const char *requested);
/* Reopen an already resolved effective resource without another fallback search. */
SvFont *sv_font_open_file(const char *resolved_path);
SDL_Surface *sv_font_render_surface(SvFont *font, const char *text, float points,
                                    float output_scale, SDL_Color color, bool bold);
bool sv_font_measure(SvFont *font, const char *text, float points, float output_scale,
                     int *width, int *height, bool bold);
void sv_font_close(SvFont *font);
const char *sv_font_resource(const SvFont *font);
bool sv_font_draw(SvFont *font, SDL_Renderer *renderer, const char *text,
                  int x, int y, float output_scale, SDL_Color color);
int sv_font_cell_width(SvFont *font, float output_scale);
#endif
