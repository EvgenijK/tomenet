#include "input/native-macro-loader.h"
#include <stdio.h>
#include <string.h>

bool sv_native_macro_loader_event(SvNativeMacroLoader *loader, SvPreferenceRuntime *runtime,
                                  SvApp *app, const SDL_Event *event)
{
    if (!loader || !runtime || !app || !event) return false;
    if (loader->active && (sv_app_view(app).request.pending || sv_app_view(app).paused)) {
        *loader = (SvNativeMacroLoader){0};
        return false;
    }
    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        return false;
    if (!loader->active) {
        if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat ||
            !(event->key.mod & SDL_KMOD_CTRL) ||
            (event->key.key != SDLK_F7 && event->key.key != SDLK_F8)) return false;
        if (sv_app_view(app).request.pending || sv_app_view(app).paused ||
            !sv_app_macro_idle(app)) return false;
        *loader = (SvNativeMacroLoader){.active = true,
                                        .class_load = event->key.key == SDLK_F8};
        return true;
    }
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        const char *text = event->text.text;
        if (!text || strlen(text) > sizeof(loader->name) - 1 - loader->size)
            return true;
        loader->attempted = false;
        loader->status[0] = 0;
        for (size_t i = 0; text[i]; ++i) {
            unsigned char byte = (unsigned char)text[i];
            if (byte < 32 || byte == 127 || byte == '/' || byte == '\\' || byte == ':' ||
                loader->size == sizeof(loader->name) - 1) continue;
            loader->name[loader->size++] = (char)byte;
        }
        loader->name[loader->size] = 0;
        return true;
    }
    if (event->type != SDL_EVENT_KEY_DOWN) return true;
    if (event->key.key == SDLK_ESCAPE) {
        *loader = (SvNativeMacroLoader){0};
    } else if (event->key.key == SDLK_BACKSPACE) {
        loader->attempted = false;
        loader->status[0] = 0;
        if (loader->size) {
            --loader->size;
            while (loader->size &&
                   ((unsigned char)loader->name[loader->size] & 0xc0) == 0x80)
                --loader->size;
            loader->name[loader->size] = 0;
        }
    } else if ((event->key.key == SDLK_RETURN || event->key.key == SDLK_KP_ENTER) &&
               loader->size && !loader->attempted) {
        SvPrefReport report = {0};
        SvResult result = loader->class_load ?
            sv_preference_runtime_class(runtime, loader->name, &report) :
            sv_preference_runtime_named(runtime, loader->name, &report);
        snprintf(loader->status, sizeof(loader->status),
                 "%s: %zu files, %zu warnings (%s)",
                 result == SV_OK && report.files && report.complete ? "Loaded" :
                 result == SV_OK && report.files ? "Load incomplete" : "Load failed",
                 report.files, report.warnings, sv_result_text(result));
        loader->attempted = result != SV_BUSY;
        if (result == SV_OK && report.files && report.complete)
            *loader = (SvNativeMacroLoader){0};
    }
    return true;
}

bool sv_native_macro_loader_draw(const SvNativeMacroLoader *loader, SDL_Renderer *renderer,
                                 SvFont *font, float scale)
{
    if (!loader || !loader->active) return true;
    SDL_FRect panel = {32 * scale, 420 * scale, 850 * scale, 165 * scale};
    SDL_Color title = {225, 232, 237, 255}, text = {180, 196, 208, 255};
    char line[560];
    snprintf(line, sizeof(line), "%s: %s", loader->class_load ? "Class macros" :
             "Named macro file", loader->name);
    return SDL_SetRenderDrawColor(renderer, 29, 37, 46, 255) &&
        SDL_RenderFillRect(renderer, &panel) &&
        sv_font_draw(font, renderer, "Ctrl+F7 file / Ctrl+F8 class; Enter loads, Escape closes",
                     (int)(44 * scale), (int)(440 * scale), scale, title) &&
        sv_font_draw(font, renderer, line, (int)(44 * scale), (int)(475 * scale), scale, text) &&
        sv_font_draw(font, renderer, loader->status, (int)(44 * scale),
                     (int)(515 * scale), scale, text);
}
