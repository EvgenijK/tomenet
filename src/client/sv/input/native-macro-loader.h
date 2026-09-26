#ifndef SV_NATIVE_MACRO_LOADER_H
#define SV_NATIVE_MACRO_LOADER_H
#include "preferences-runtime.h"
#include "ui/font.h"
typedef struct {
    bool active, class_load, attempted;
    char name[501], status[160];
    size_t size;
} SvNativeMacroLoader;
/* Returns true when the event belongs to the loader, including its opener. */
bool sv_native_macro_loader_event(SvNativeMacroLoader *loader, SvPreferenceRuntime *runtime,
                                  SvApp *app, const SDL_Event *event);
bool sv_native_macro_loader_draw(const SvNativeMacroLoader *loader, SDL_Renderer *renderer,
                                 SvFont *font, float scale);
#endif
