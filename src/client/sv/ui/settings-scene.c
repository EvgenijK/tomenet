#include "settings-scene.h"
#include <stdio.h>
#include <string.h>

bool sv_settings_scene_open(SvSettingsScene *scene, const char *root,
                            const char *library, SvProfile *profile,
                            const SvOptions *options, SvFont **font)
{
    if (!scene || scene->open || !font || !*font || !library) return false;
    memset(scene, 0, sizeof(*scene));
    if (!sv_settings_begin(&scene->settings, root, profile, options)) return false;
    scene->live_profile = profile;
    scene->live_font = font;
    scene->opening_font = *font;
    scene->library = library;
    scene->open = true;
    return true;
}

void sv_settings_scene_end(SvSettingsScene *scene)
{
    if (!scene) return;
    if (scene->live_font && scene->opening_font &&
        *scene->live_font != scene->opening_font)
        sv_font_close(scene->opening_font);
    sv_settings_end(&scene->settings);
    scene->open = scene->close_prompt = false;
    scene->live_profile = NULL;
    scene->live_font = NULL;
    scene->opening_font = NULL;
    scene->library = NULL;
}

static void cancel(SvSettingsScene *scene, SDL_Window *window)
{
    bool fullscreen = !scene->settings.opening_profile.windowed;
    if (!SDL_SetWindowFullscreen(window, fullscreen)) {
        snprintf(scene->status, sizeof(scene->status),
                 "Cannot restore window mode: %.100s", SDL_GetError());
        return;
    }
    sv_settings_cancel(&scene->settings);
    if (*scene->live_font != scene->opening_font) {
        sv_font_close(*scene->live_font);
        *scene->live_font = scene->opening_font;
    }
    *scene->live_profile = scene->settings.profile;
    sv_settings_scene_end(scene);
}

static void save(SvSettingsScene *scene)
{
    SvSettingsResult result = sv_settings_save_cfg(&scene->settings);
    if (result == SV_SETTINGS_OK) {
        snprintf(scene->status, sizeof(scene->status), "Saved to SV profile");
        scene->close_prompt = false;
    } else if (result == SV_SETTINGS_CONFLICT)
        snprintf(scene->status, sizeof(scene->status),
                 "External edit won. Draft remains unsaved; Cancel or reopen to resolve.");
    else snprintf(scene->status, sizeof(scene->status),
                  "Save failed. Draft is active and unsaved.");
}

bool sv_settings_scene_preview_font(SvSettingsScene *scene, const char *requested)
{
    if (!scene || !scene->open || !requested) return false;
    SvFont *prepared = sv_font_open_exact(scene->settings.root, scene->library,
                                          requested);
    if (!prepared) {
        snprintf(scene->status, sizeof(scene->status),
                 "Font preview failed; previous font remains active.");
        return false;
    }
    if (!sv_settings_edit_cfg(&scene->settings, "svTextFont", requested)) {
        sv_font_close(prepared);
        return false;
    }
    if (*scene->live_font != scene->opening_font)
        sv_font_close(*scene->live_font);
    *scene->live_font = prepared;
    *scene->live_profile = scene->settings.profile;
    return true;
}

bool sv_settings_scene_event(SvSettingsScene *scene, SDL_Window *window,
                             const SDL_Event *event)
{
    if (!scene || !scene->open || !window || !event) return false;
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat) return true;
    SDL_Keycode key = event->key.key;
    if (scene->close_prompt) {
        if (key == SDLK_R || key == SDLK_ESCAPE) scene->close_prompt = false;
        else if (key == SDLK_C) cancel(scene, window);
        else if (key == SDLK_S) {
            save(scene);
            if (!sv_settings_dirty(&scene->settings)) sv_settings_scene_end(scene);
        }
        return true;
    }
    if (key == SDLK_ESCAPE || key == SDLK_F10) {
        if (sv_settings_dirty(&scene->settings)) scene->close_prompt = true;
        else sv_settings_scene_end(scene);
        return true;
    }
    if (key == SDLK_S) { save(scene); return true; }
    if (key == SDLK_T) {
        const char *next = !strcmp(scene->settings.profile.text_font,
                                   "16x24x.pcf") ?
                           "CascadiaMono-Regular.ttf" : "16x24x.pcf";
        (void)sv_settings_scene_preview_font(scene, next);
        return true;
    }
    if (key == SDLK_W) {
        bool target = !scene->settings.profile.windowed;
        if (!SDL_SetWindowFullscreen(window, !target)) {
            snprintf(scene->status, sizeof(scene->status),
                     "Window mode preview failed: %.100s", SDL_GetError());
        } else {
            (void)sv_settings_edit_cfg(&scene->settings, "svWindowMode",
                                       target ? "window" : "fullscreen");
            *scene->live_profile = scene->settings.profile;
        }
        return true;
    }
    if (key == SDLK_PLUS || key == SDLK_EQUALS || key == SDLK_KP_PLUS ||
        key == SDLK_MINUS || key == SDLK_KP_MINUS) {
        int step = (key == SDLK_MINUS || key == SDLK_KP_MINUS) ? -5 : 5;
        int target = scene->settings.profile.ui_scale + step;
        char text[12];
        snprintf(text, sizeof(text), "%d", target);
        if (!sv_settings_edit_cfg(&scene->settings, "svUiScalePercent", text))
            snprintf(scene->status, sizeof(scene->status),
                     "UI scale must be 50..200 in steps of 5");
        else *scene->live_profile = scene->settings.profile;
        return true;
    }
    return true;
}

static bool line(SDL_Renderer *renderer, SvFont *font, const char *message,
                 int y, float scale, SDL_Color colour)
{
    return sv_font_draw(font, renderer, message, (int)(40 * scale),
                        (int)(y * scale), scale, colour);
}

bool sv_settings_scene_draw(const SvSettingsScene *scene, SDL_Renderer *renderer,
                            SvFont *font)
{
    if (!scene || !scene->open || !renderer || !font) return false;
    float scale = SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer)) *
                  scene->settings.profile.ui_scale / 100.0f;
    if (scale <= 0) scale = 1;
    SDL_Color title = {226,235,245,255}, normal = {185,205,223,255},
              changed = {255,210,106,255}, error = {255,135,135,255};
    if (!SDL_SetRenderDrawColor(renderer, 15,21,28,255) ||
        !SDL_RenderClear(renderer)) return false;
    char row[160];
    snprintf(row, sizeof(row), "Window mode: %s (W toggles)",
             scene->settings.profile.windowed ? "window" : "fullscreen");
    if (!line(renderer, font, "TomeNET SV - Settings", 34, scale, title) ||
        !line(renderer, font, row, 100, scale, normal)) return false;
    snprintf(row, sizeof(row), "UI scale: %d%% (+/- adjusts)",
             scene->settings.profile.ui_scale);
    if (!line(renderer, font, row, 145, scale, normal) ||
        !line(renderer, font, "T Toggle text font; S Save; F10/Escape Close",
              220, scale, normal))
        return false;
    if (sv_settings_dirty(&scene->settings) &&
        !line(renderer, font, "Unsaved settings preview is active", 265, scale, changed))
        return false;
    if (scene->close_prompt &&
        !line(renderer, font, "Save (S) / Cancel changes (C) / Return (R)",
              320, scale, changed)) return false;
    if (scene->status[0] &&
        !line(renderer, font, scene->status, 370, scale, error)) return false;
    return SDL_RenderPresent(renderer);
}
