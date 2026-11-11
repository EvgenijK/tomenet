#ifndef SV_LOGIN_PREPARATION_INTERNAL_H
#define SV_LOGIN_PREPARATION_INTERNAL_H
#include "login/preparation.h"
#include "ui/native-controls.h"
struct SvLoginFlow;
struct SvSessionPreparation {
    struct SvLoginFlow *owner;
    SvApp *app;
    SvPreparationResources resources;
    SvGraphics *graphics;
    SvPreferences *application;
    SvStartupIns *inscriptions;
    unsigned char *acked[2];
    size_t effect_count[2];
    size_t keymap_cursor;
    SvLuaReload *lua;
    bool owns_lua, moved, packet_active, identity_reported, native_bound;
    SvPreparationView view;
    unsigned char input[128 * 1024];
    size_t input_size;
    unsigned char deferred[SV_PREPARATION_DEFERRED_BYTES];
    SvPreparationFrame frames[SV_PREPARATION_DEFERRED_FRAMES];
    size_t deferred_size, deferred_at, frame_count, frame_at;
    char *root, *library;
    SvNativeInput native_input;
    SvNativeControls controls;
    void *native_owner;
    void (*destroy_native)(void *);
};
SvSessionPreparation *sv_preparation_create(struct SvLoginFlow *);
SvResult sv_preparation_poll(SvSessionPreparation *);
#endif
