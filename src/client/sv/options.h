#ifndef SV_OPTIONS_H
#define SV_OPTIONS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The legacy option packet reserves 199 slots; option_info currently names 188. */
#define SV_OPTION_COUNT 199
typedef struct { bool value[SV_OPTION_COUNT]; } SvOptions;

void sv_options_defaults(SvOptions *options);
/* Both loaders are read-only. Character must be called after the name is known,
 * before the first options packet, with a fresh base snapshot for each login. */
bool sv_options_load_base(SvOptions *options, const char *user_root);
bool sv_options_load_own_layers(SvOptions *options, const char *user_root);
bool sv_options_load_character(SvOptions *options, const char *user_root,
                               const char *character);
bool sv_options_safe_name(const char *name);
/* Explicit named OPT load; does not change automatic startup layer order. */
bool sv_options_load_named(SvOptions *options, const char *user_root,
                           const char *filename);
bool sv_options_get(const SvOptions *options, const char *name, bool *value);
bool sv_options_set(SvOptions *options, const char *name, bool value);
bool sv_options_apply_pref(SvOptions *options, char directive, const char *name);
const char *sv_options_name(size_t index);
/* Complete packet, including PKT_OPTIONS; zero means insufficient capacity. */
size_t sv_options_packet(const SvOptions *options, const int version[6],
                         uint8_t *out, size_t capacity);
#endif
