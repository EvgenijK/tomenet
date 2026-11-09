#include "settings/startup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static bool copy(char *out, size_t capacity, const char *value)
{
    size_t length = strlen(value);
    if (length >= capacity || strchr(value, '\n') || strchr(value, '\r')) return false;
    memcpy(out, value, length + 1);
    return true;
}
static bool number(const char *text, int minimum, int maximum, int *out)
{
    char *end;
    long value = strtol(text, &end, 10);
    if (!*text || *end || value < minimum || value > maximum) return false;
    *out = (int)value;
    return true;
}
void sv_settings_invocation_clear_password(SvSettingsInvocation *invocation)
{
    volatile char *secret = invocation->password;
    for (size_t i = 0; i < sizeof(invocation->password); ++i) secret[i] = 0;
    invocation->password_present = false;
}
bool sv_settings_startup_load(SvSettingsConfig *config, const char *root)
{
    char path[4096];
    int size = snprintf(path, sizeof(path), "%s/sv/tomenet.cfg", root);
    if (size < 0 || (size_t)size >= sizeof(path)) return false;
    SvSettingsConfigResult result = sv_settings_config_load(config, path);
    /* Malformed/future files have already supplied safe defaults and diagnostics. */
    return result != SV_SETTINGS_CONFIG_MEMORY;
}
bool sv_settings_startup_arguments(SvSettingsConfig *config,
    SvSettingsInvocation *invocation, int argc, char *const argv[])
{
    memset(invocation, 0, sizeof(*invocation));
    int graphics = -1;
    /* Baseline client.c modus: -l starts account/password positional parsing;
     * all following positionals become server names after the password. */
    int login_argument = 0;
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i], *value = arg + 2;
        if (arg[0] != '-') {
            if (login_argument == 1) {
                if (!*arg || !copy(config->nick, sizeof(config->nick), arg)) return false;
                login_argument = 2;
            } else if (login_argument == 2) {
                if (!*arg || !copy(invocation->password, sizeof(invocation->password), arg)) return false;
                invocation->password_present = true;
                login_argument = 3;
            } else if (!sv_settings_config_server(config, arg)) return false;
            continue;
        }
        if (arg[1] == '-') {
            if (!strcmp(arg, "--endpoint")) continue;
            if (!strcmp(arg, "--password-stdin")) { invocation->password_stdin = true; continue; }
            if (i + 1 >= argc) return false;
            value = argv[++i];
            if (!strcmp(arg, "--server")) {
                if (!sv_settings_config_server(config, value)) return false;
            } else if (!strcmp(arg, "--metaserver")) {
                if (!copy(config->meta, sizeof(config->meta), value)) return false;
            } else if (!strcmp(arg, "--port")) {
                if (!number(value, 1, 65535, &config->port)) return false;
            } else if (!strcmp(arg, "--account")) {
                if (!copy(config->nick, sizeof(config->nick), value)) return false;
            } else if (!strcmp(arg, "--real-name")) {
                if (!copy(config->realname, sizeof(config->realname), value)) return false;
            } else if (!strcmp(arg, "--character")) {
                if (!copy(config->name, sizeof(config->name), value)) return false;
            } else if (!strcmp(arg, "--library")) {
                if (!copy(invocation->library_override, sizeof(invocation->library_override), value)) return false;
            } else if (strcmp(arg, "--profile-root") && strcmp(arg, "--server-list") &&
                       strcmp(arg, "--fixture-window") && strcmp(arg, "--frames")) return false;
            continue;
        }
        switch (arg[1]) {
        case 'i':
            if (*value) return false;
            /* The baseline reset leaves identity/root selection intact. */
            {
                char realname[SV_SETTINGS_TEXT_MAX], path[SV_SETTINGS_TEXT_MAX];
                int overwrite_allowed = config->overwrite_allowed;
                memcpy(realname, config->realname, sizeof(realname));
                memcpy(path, config->path, sizeof(path));
                sv_settings_config_dispose(config);
                sv_settings_config_defaults(config);
                memcpy(config->realname, realname, sizeof(realname));
                memcpy(config->path, path, sizeof(path));
                config->nick[0] = config->name[0] = config->server[0] = config->meta[0] = 0;
                /* Ignoring loaded values does not authorize replacing a newer
                 * incompatible on-disk schema at the normal Save destination. */
                config->overwrite_allowed = overwrite_allowed;
            }
            invocation->quiet = false;
            sv_settings_invocation_clear_password(invocation);
            invocation->password_stdin = false;
            invocation->reset = true;
            break;
        case 'f': {
            if (!*value) return false;
            SvSettingsConfigResult result = sv_settings_config_overlay_load(config, value);
            if (result == SV_SETTINGS_CONFIG_MEMORY) return false;
            break;
        }
        case 'p': if (!number(value, 1, 65535, &config->port)) return false; break;
        case 'F': if (!number(value, 0, INT_MAX, &config->fps)) return false; break;
        case 'P': if (!*value || !copy(invocation->library_override, sizeof(invocation->library_override), value)) return false; break;
        case 'n':
            if (*value) {
                if (!copy(config->realname, sizeof(config->realname), value)) return false;
            } else invocation->force_cui = true; /* baseline fall-through to -c */
            break;
        case 'R':
            invocation->auto_reincarnation = true;
            /* Baseline fall-through to -N; keep name bytes without a guessed + prefix. */
            if (!copy(config->name, sizeof(config->name), value)) return false;
            break;
        case 'N': if (!copy(config->name, sizeof(config->name), value)) return false; break;
        case 'l':
            if (*value) {
                if (!copy(config->nick, sizeof(config->nick), value)) return false;
                login_argument = 2;
            } else login_argument = 1;
            break;
        case 'c': if (*value) return false; invocation->force_cui = true; break;
        case 'C': if (*value) return false; invocation->protocol_override = 1; break;
        case 'w': if (*value) return false; invocation->no_weather = true; break;
        case 'u': if (*value) return false; invocation->no_lua_updates = true; break;
        case 'X': if (*value) return false; invocation->quit_no_prompt = true; break;
        case 'v': if (*value) return false; invocation->chat_log_mode = 1; break;
        case 'V': if (*value) return false; invocation->chat_log_mode = 2; break;
        case 'x': if (*value) return false; invocation->chat_log_mode = 3; break;
        case 'e': if (*value) return false; invocation->redirect_log = true; break;
        case 't': if (*value) return false; invocation->disable_tileset_cache = true; break;
        case 'T': if (*value) return false; invocation->disable_tile_cache = true; break;
        case 's': if (*value) return false; invocation->ignore_scaling = true; break;
        case 'h': if (*value) return false; invocation->help = true; break;
        case 'q': if (*value) return false; invocation->quiet = true; break;
        case 'm': if (*value) return false; invocation->skip_motd = true; break;
        case 'a': if (*value) return false; graphics = 0; break;
        case 'g': if (*value) return false; graphics = 1; break;
        case 'G': if (*value) return false; graphics = 2; break;
        default: return false;
        }
    }
    if (graphics >= 0) config->graphics = graphics;
    if (login_argument == 1) { invocation->help = true; return false; }
    /* An explicit source choice avoids sending a different positional secret
     * when the existing --password-stdin route was requested. */
    if (invocation->password_stdin && invocation->password_present) {
        sv_settings_invocation_clear_password(invocation);
        return false;
    }
    /* quiet is an invocation override: keep requested sound choice intact. */
    return true;
}
