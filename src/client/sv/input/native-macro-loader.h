#ifndef SV_NATIVE_MACRO_LOADER_H
#define SV_NATIVE_MACRO_LOADER_H
#include "preferences-runtime.h"
#include "input/native-input.h"
#include "ui/font.h"
typedef struct {
    bool active, class_load, attempted, close_after_commit;
    char name[501], status[160];
    size_t size;
    SvPrefLoadJob *job;
} SvNativeMacroLoader;
void sv_native_macro_loader_reset(SvNativeMacroLoader *loader);
/* Returns true when the event belongs to the loader, including its opener. */
bool sv_native_macro_loader_event(SvNativeMacroLoader *loader, SvPreferenceRuntime *runtime,
                                  SvApp *app, const SvNativeInput *input,
                                  const SDL_Event *event);
void sv_native_macro_loader_frame(SvNativeMacroLoader *loader,
                                  SvPreferenceRuntime *runtime, SvApp *app,
                                  const SvNativeInput *input);
bool sv_native_macro_loader_committing(const SvNativeMacroLoader *loader);
bool sv_native_macro_loader_dispatch_input(const SvNativeMacroLoader *loader,
                                           const SvApp *app);
bool sv_native_macro_loader_draw(const SvNativeMacroLoader *loader, SDL_Renderer *renderer,
                                 SvFont *font, float scale);
#endif
