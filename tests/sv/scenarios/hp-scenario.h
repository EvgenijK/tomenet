#ifndef SV_HP_SCENARIO_H
#define SV_HP_SCENARIO_H
#include <SDL3/SDL.h>
#include "app.h"
typedef int (*SvScenarioFrame)(void *context, SvAppView view);
bool sv_hp_check_pixels(SDL_Renderer *renderer, const SDL_Rect *bounds);
int sv_hp_scenario(SvApp *app, SvScenarioFrame submit, void *context);
#endif
