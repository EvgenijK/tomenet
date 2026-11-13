#ifndef SV_SETTINGS_STARTUP_H
#define SV_SETTINGS_STARTUP_H
#include "settings/config.h"
#include <stdbool.h>

/* CLI reduction over loaded requested data. No writes or UI side effects.
 * library_override is separate: loading a CFG never relocates U or B. */
typedef struct {
    char library_override[4096];
    bool quiet, skip_motd, reset;
    /* Secret invocation input never enters CFG records, diagnostics or Save.
     * It lives only until the caller copies it into the credential owner. */
    char password[16];
    bool password_present, password_stdin;
    bool auto_reincarnation, no_weather, no_lua_updates, quit_no_prompt;
    bool force_cui, redirect_log, disable_tileset_cache, disable_tile_cache;
    bool ignore_scaling, help;
    int protocol_override, chat_log_mode;
} SvSettingsInvocation;
void sv_settings_invocation_clear_password(SvSettingsInvocation *invocation);
bool sv_settings_startup_load(SvSettingsConfig *config, const char *user_root);
bool sv_settings_startup_arguments(SvSettingsConfig *config,
    SvSettingsInvocation *invocation, int argc, char *const argv[]);
#endif
