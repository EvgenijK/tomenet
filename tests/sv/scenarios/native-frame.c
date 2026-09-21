#include "native-frame.h"
#include "hp-scenario.h"
int sv_scenario_frame(void *context, SvAppView view)
{
    SvUi *ui = context;
    SDL_PumpEvents();
    if (!sv_ui_draw(ui, view)) return 0;
    float scale = SDL_GetWindowDisplayScale(ui->window);
    SDL_Rect bounds = {(int)(40 * scale), (int)(155 * scale), (int)(700 * scale), (int)(30 * scale)};
    if (!sv_hp_check_pixels(ui->renderer, &bounds)) return 0;
    if (!view.active) {
        bounds.y = (int)(190 * scale);
        if (!sv_hp_check_pixels(ui->renderer, &bounds)) return 0;
    }
    return SDL_RenderPresent(ui->renderer);
}
