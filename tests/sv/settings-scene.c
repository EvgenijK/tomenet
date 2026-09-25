#include "ui/settings-scene.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void key(SvSettingsScene *scene, SDL_Window *window, SDL_Keycode code)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = code;
    assert(sv_settings_scene_event(scene, window, &event));
}

int main(int argc, char **argv)
{
    assert(argc == 3);
    assert(SDL_Init(SDL_INIT_VIDEO) && TTF_Init());
    SDL_Window *window = SDL_CreateWindow("settings-check", 1024, 768, 0);
    assert(window);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    assert(renderer);
    SvFont *font = sv_font_open_requested(argv[1], argv[2],
                                           "CascadiaMono-Regular.ttf");
    assert(font);
    SvProfile profile;
    SvOptions options;
    sv_profile_defaults(&profile);
    profile.windowed = true;
    sv_options_defaults(&options);
    SvSettingsScene scene = {0};
    assert(sv_settings_scene_open(&scene, argv[1], argv[2], &profile,
                                  &options, &font));
    key(&scene, window, SDLK_PLUS);
    assert(profile.ui_scale == 105 && sv_settings_dirty(&scene.settings));
    SvFont *before_font = font;
    assert(!sv_settings_scene_preview_font(&scene, "missing-font.ttf"));
    assert(font == before_font &&
           !strcmp(profile.text_font, "CascadiaMono-Regular.ttf"));
    key(&scene, window, SDLK_T);
    assert(font != before_font && !strcmp(profile.text_font, "16x24x.pcf"));
    assert(strstr(sv_font_resource(font), "16x24x.pcf"));
    assert(sv_settings_scene_draw(&scene, renderer, font));
    key(&scene, window, SDLK_ESCAPE);
    assert(scene.close_prompt && scene.open);
    key(&scene, window, SDLK_R);
    assert(!scene.close_prompt && scene.open && profile.ui_scale == 105);
    key(&scene, window, SDLK_ESCAPE);
    key(&scene, window, SDLK_C);
    assert(!scene.open && profile.ui_scale == 100);
    assert(font == before_font &&
           !strcmp(profile.text_font, "CascadiaMono-Regular.ttf"));
    char cfg[4096];
    assert(snprintf(cfg, sizeof(cfg), "%s/sv/tomenet.cfg", argv[1]) < (int)sizeof(cfg));
    FILE *file = fopen(cfg, "rb");
    assert(!file);

    assert(sv_settings_scene_open(&scene, argv[1], argv[2], &profile,
                                  &options, &font));
    key(&scene, window, SDLK_PLUS);
    key(&scene, window, SDLK_S);
    assert(!sv_settings_dirty(&scene.settings));
    key(&scene, window, SDLK_PLUS);
    assert(profile.ui_scale == 110);
    key(&scene, window, SDLK_ESCAPE);
    key(&scene, window, SDLK_C);
    assert(!scene.open && profile.ui_scale == 100);
    file = fopen(cfg, "rb");
    assert(file);
    char bytes[512];
    size_t count = fread(bytes, 1, sizeof(bytes) - 1, file);
    assert(!fclose(file));
    bytes[count] = 0;
    assert(strstr(bytes, "svUiScalePercent\t105\n"));
    assert(sv_profile_load(&profile, argv[1]) && profile.ui_scale == 105);

    assert(sv_settings_scene_open(&scene, argv[1], argv[2], &profile,
                                  &options, &font));
    key(&scene, window, SDLK_PLUS);
    assert(profile.ui_scale == 110);
    sv_settings_scene_end(&scene); /* Normal exit: no CFG write or prompt. */
    file = fopen(cfg, "rb");
    assert(file);
    count = fread(bytes, 1, sizeof(bytes) - 1, file);
    assert(!fclose(file));
    bytes[count] = 0;
    assert(strstr(bytes, "svUiScalePercent\t105\n"));
    assert(!strstr(bytes, "110"));

    sv_font_close(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    puts("SV-B-004 native settings child checks passed");
    return 0;
}
