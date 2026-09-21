#include "ui.h"

static bool rectangle(SDL_Renderer *renderer, float scale, float x, float y, float w, float h)
{
    SDL_FRect rect = {SDL_roundf(x * scale), SDL_roundf(y * scale),
                     SDL_roundf(w * scale), SDL_roundf(h * scale)};
    return SDL_SetRenderDrawColor(renderer, 29, 37, 46, 255) && SDL_RenderFillRect(renderer, &rect);
}

bool sv_ui_draw(SvUi *ui, SvAppView view)
{
    SDL_Window *window = ui->window;
    SDL_Renderer *renderer = ui->renderer;
    SvFont *font = ui->font;
    int w, h;
    float scale = SDL_GetWindowDisplayScale(window);
    if (scale <= 0 || !SDL_GetRenderOutputSize(renderer, &w, &h)) return false;
    unsigned changed = sv_status_prepare(&ui->status, view,
        (SvPresentationKey){w, h, scale, ui->font_revision});
    if (changed & SV_LAYOUT_CHANGED) ui->logical_width = w / scale;
    float logical_w = ui->logical_width;
    SvStatus status = ui->status.status;
    const SDL_Color title = {225, 232, 237, 255}, text = {180, 196, 208, 255};
    if (!SDL_SetRenderDrawColor(renderer, 15, 21, 28, 255) || !SDL_RenderClear(renderer)) return false;
    if (!rectangle(renderer, scale, 24, 90, logical_w - 48, 144) ||
        !rectangle(renderer, scale, 24, 254, logical_w - 48, 144)) return false;
    const char *lines[] = {
        "TomeNET SV - synthetic shell",
        "Native surfaces",
        ui->status.text,
        view.active ? "No server connection or gameplay is active." : sv_result_text(view.reason),
        view.executor_failed ? "Optional alert executor failed; session continues." : "Isolated profile. Fullscreen default. UI scale 100%.",
        "One SDL window. Close or press Escape to exit."
    };
    const int ys[] = {30, 112, 155, 190, 278, 330};
    for (unsigned i = 0; i < SDL_arraysize(lines); ++i)
        if (!sv_font_draw(font, renderer, lines[i], (int)SDL_roundf(40 * scale),
                          (int)SDL_roundf(ys[i] * scale), scale, i == 0 ? title : text)) return false;
    if (status.bar) {
        float fraction = status.maximum > 0 ? (float)status.current / status.maximum : 0;
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;
        SDL_FRect bar = {40 * scale, 217 * scale, 240 * fraction * scale, 8 * scale};
        if (!SDL_SetRenderDrawColor(renderer, 80, 190, 110, 255) || !SDL_RenderFillRect(renderer, &bar)) return false;
    }
    return true;
}
