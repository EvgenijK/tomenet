#include "ui.h"
#include "message-text.h"

static bool rectangle(SDL_Renderer *renderer, float scale, float x, float y, float w, float h)
{
    SDL_FRect rect = {SDL_roundf(x * scale), SDL_roundf(y * scale),
                     SDL_roundf(w * scale), SDL_roundf(h * scale)};
    return SDL_SetRenderDrawColor(renderer, 29, 37, 46, 255) && SDL_RenderFillRect(renderer, &rect);
}


static bool draw_messages(SvUi *ui, float scale)
{
    static const SDL_Color colors[16] = {
        {0,0,0,255},{255,255,255,255},{128,128,128,255},{255,128,0,255},
        {192,0,0,255},{0,128,64,255},{0,0,255,255},{128,64,0,255},
        {64,64,64,255},{192,192,192,255},{255,0,255,255},{255,255,0,255},
        {255,64,64,255},{0,255,0,255},{0,255,255,255},{192,128,64,255}
    };
    int cell = ui->message_cell;
    for (size_t row = 0; row < ui->messages.count; ++row) {
        SvMessageText line = ui->messages.lines[row];
        line.length = ui->message_lengths[row];
        for (size_t start = 0; start < line.length;) {
            size_t end = start + 1;
            while (end < line.length && line.colors[end] == line.colors[start]) ++end;
            char saved = line.text[end];
            line.text[end] = 0;
            if (!sv_font_draw(ui->font, ui->renderer, line.text + start,
                    (int)SDL_roundf(40 * scale) + (int)start * cell,
                    (int)SDL_roundf((454 + row * 34) * scale), scale,
                    colors[line.colors[start]])) return false;
            line.text[end] = saved;
            start = end;
        }
    }
    return true;
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
    int messages_changed = sv_messages_prepare(&ui->messages, view.generation, view.messages);
    if (messages_changed || (changed & SV_LAYOUT_CHANGED)) {
        ui->message_cell = sv_font_cell_width(font, scale);
        if (ui->message_cell <= 0) return false;
        float available = (ui->logical_width - 80) * scale;
        size_t limit = available > 0 ? (size_t)(available / ui->message_cell) : 0;
        for (size_t i = 0; i < ui->messages.count; ++i)
            ui->message_lengths[i] = ui->messages.lines[i].length < limit ? ui->messages.lines[i].length : limit;
    }
    if (ui->request_generation != view.generation || ui->request_sequence != view.request.sequence) {
        SvMessage prompt = {0};
        while (prompt.length < SV_REQUEST_BYTES && view.request.prompt[prompt.length]) ++prompt.length;
        SDL_memcpy(prompt.bytes, view.request.prompt, prompt.length);
        ui->prompt = sv_message_text(&prompt);
        ui->request_generation = view.generation;
        ui->request_sequence = view.request.sequence;
    }
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
        view.request.pending ? "Server key request" :
        (view.executor_failed ? "Optional alert executor failed; session continues." : "Isolated profile. Fullscreen default. UI scale 100%."),
        view.request.pending ? "Press a key to answer; Escape cancels the request." : "One SDL window. Close or press Escape to exit."
    };
    const int ys[] = {30, 112, 155, 190, 278, 365};
    for (unsigned i = 0; i < SDL_arraysize(lines); ++i)
        if (!sv_font_draw(font, renderer, lines[i], (int)SDL_roundf(40 * scale),
                          (int)SDL_roundf(ys[i] * scale), scale, i == 0 ? title : text)) return false;
    if (view.request.pending) {
        int cell = sv_font_cell_width(font, scale);
        if (cell <= 0) return false;
        size_t columns = (size_t)((logical_w - 80) * scale / cell);
        if (!columns) return false;
        for (size_t start = 0, row = 0; start < ui->prompt.length; start += columns, ++row) {
            char line[SV_REQUEST_BYTES + 1];
            size_t count = ui->prompt.length - start;
            if (count > columns) count = columns;
            SDL_memcpy(line, ui->prompt.text + start, count); line[count] = 0;
            if (!sv_font_draw(font, renderer, line, (int)SDL_roundf(40 * scale),
                    (int)SDL_roundf((305 + row * 28) * scale), scale, title)) return false;
        }
    }
    if (status.bar) {
        float fraction = status.maximum > 0 ? (float)status.current / status.maximum : 0;
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;
        SDL_FRect bar = {40 * scale, 217 * scale, 240 * fraction * scale, 8 * scale};
        if (!SDL_SetRenderDrawColor(renderer, 80, 190, 110, 255) || !SDL_RenderFillRect(renderer, &bar)) return false;
    }
    if (!rectangle(renderer, scale, 24, 418, logical_w - 48, 260)) return false;
    return draw_messages(ui, scale);
}
