#ifndef SV_SETTINGS_SCENE_H
#define SV_SETTINGS_SCENE_H
#include "settings.h"
#include "ui/font.h"
#include <SDL3/SDL.h>

typedef struct {
    SvSettings settings;
    bool open, close_prompt;
    char status[160];
    SvProfile *live_profile;
    SvFont **live_font;
    SvFont *opening_font;
    const char *library;
} SvSettingsScene;

bool sv_settings_scene_open(SvSettingsScene *scene, const char *root,
                            const char *library, SvProfile *profile,
                            const SvOptions *options, SvFont **font);
void sv_settings_scene_end(SvSettingsScene *scene);
/* A true result means the event belongs to the settings child. */
bool sv_settings_scene_event(SvSettingsScene *scene, SDL_Window *window,
                             const SDL_Event *event);
bool sv_settings_scene_preview_font(SvSettingsScene *scene, const char *requested);
bool sv_settings_scene_draw(const SvSettingsScene *scene, SDL_Renderer *renderer,
                            SvFont *font);
#endif
