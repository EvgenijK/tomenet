#include "input/native-macro-loader.h"
#include <stdio.h>
#include <string.h>

void sv_native_macro_loader_reset(SvNativeMacroLoader *loader)
{
    if (!loader) return;
    sv_preference_runtime_cancel_load(loader->job);
    *loader = (SvNativeMacroLoader){0};
}

bool sv_native_macro_loader_event(SvNativeMacroLoader *loader, SvPreferenceRuntime *runtime,
                                  SvApp *app, const SvNativeInput *input,
                                  const SDL_Event *event)
{
    if (!loader || !runtime || !app || !input || !event) return false;
    if (sv_native_input_stale(input, app, event)) return true;
    if (loader->active && sv_native_macro_loader_committing(loader) &&
        (sv_app_view(app).request.pending || sv_app_view(app).paused) &&
        (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_TEXT_INPUT))
        return false; /* The request or pause owns its reply while replay continues. */
    if (loader->active && !sv_native_macro_loader_committing(loader) &&
        (sv_app_view(app).request.pending || sv_app_view(app).paused)) {
        sv_native_macro_loader_reset(loader);
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
        if (loader->job) return true;
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
        if (sv_preference_runtime_load_committed(loader->job))
            loader->close_after_commit = true;
        else sv_native_macro_loader_reset(loader);
    } else if (event->key.key == SDLK_BACKSPACE) {
        if (loader->job) return true;
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
        loader->job = sv_preference_runtime_start_load(runtime, loader->name,
                                                       loader->class_load);
        if (loader->job) snprintf(loader->status, sizeof(loader->status), "Loading...");
        else {
            snprintf(loader->status, sizeof(loader->status), "Load failed to start");
            loader->attempted = true;
        }
    }
    return true;
}

void sv_native_macro_loader_frame(SvNativeMacroLoader *loader,
                                  SvPreferenceRuntime *runtime, SvApp *app,
                                  const SvNativeInput *input)
{
    if (!loader || !loader->job) return;
    if (!sv_app_view(app).active || input->generation != sv_app_view(app).generation ||
        (!sv_preference_runtime_load_committed(loader->job) &&
         (sv_app_view(app).paused || sv_app_view(app).request.pending))) {
        sv_native_macro_loader_reset(loader);
        return;
    }
    SvPrefReport report = {0};
    SvResult result = sv_preference_runtime_finish_load(runtime, loader->job, &report);
    if (result == SV_WAITING || result == SV_BUSY) {
        if (sv_preference_runtime_load_committed(loader->job))
            snprintf(loader->status, sizeof(loader->status), "Applying PRF effects...");
        return;
    }
    sv_preference_runtime_cancel_load(loader->job);
    loader->job = NULL;
    snprintf(loader->status, sizeof(loader->status),
             "%s: %zu files, %zu warnings (%s)",
             result == SV_OK && report.files && report.complete ? "Loaded" :
             result == SV_OK && report.files ? "Load incomplete" : "Load failed",
             report.files, report.warnings, sv_result_text(result));
    loader->attempted = true;
    if (loader->close_after_commit ||
        (result == SV_OK && report.files && report.complete))
        sv_native_macro_loader_reset(loader);
}

bool sv_native_macro_loader_committing(const SvNativeMacroLoader *loader)
{
    return loader && sv_preference_runtime_load_committed(loader->job);
}

bool sv_native_macro_loader_dispatch_input(const SvNativeMacroLoader *loader,
                                           const SvApp *app)
{
    return !sv_native_macro_loader_committing(loader) || sv_app_view(app).request.pending;
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
