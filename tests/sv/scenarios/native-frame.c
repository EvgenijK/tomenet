#include "native-frame.h"
#include "hp-scenario.h"
int sv_scenario_draw(void *context, SvAppView view)
{
    SvUi *ui = context;
    SDL_PumpEvents();
    if (!sv_ui_draw(ui, view)) return 0;
    float scale = sv_ui_scale(ui);
    SDL_Rect bounds = {(int)(40 * scale), (int)(155 * scale), (int)(700 * scale), (int)(30 * scale)};
    if (!sv_hp_check_pixels(ui->renderer, &bounds)) return 0;
    if (!view.active) {
        bounds.y = (int)(190 * scale);
        if (!sv_hp_check_pixels(ui->renderer, &bounds)) return 0;
    }
    for (size_t row = 0; row < view.messages.count; ++row) {
        bounds.y = (int)((454 + row * 34) * scale);
        if (!sv_hp_check_pixels(ui->renderer, &bounds)) return 0;
    }
    if (view.request.pending && view.request.prompt[0]) {
        bounds.y = (int)(305 * scale);
        if (!sv_hp_check_pixels(ui->renderer, &bounds)) return 0;
    }
    return 1;
}
int sv_scenario_frame(void *context, SvAppView view)
{
    SvUi *ui = context;
    return sv_scenario_draw(context, view) && SDL_RenderPresent(ui->renderer);
}
