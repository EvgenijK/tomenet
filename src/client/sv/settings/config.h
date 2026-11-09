#ifndef SV_SETTINGS_CONFIG_H
#define SV_SETTINGS_CONFIG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Requested choices only. Resource owners keep their effective fallbacks separate. */
#define SV_SETTINGS_TEXT_MAX 1024

typedef enum {
    SV_SETTINGS_CONFIG_OK,
    SV_SETTINGS_CONFIG_MISSING,
    SV_SETTINGS_CONFIG_IO,
    SV_SETTINGS_CONFIG_INVALID,
    SV_SETTINGS_CONFIG_FUTURE_SCHEMA,
    SV_SETTINGS_CONFIG_MEMORY
} SvSettingsConfigResult;

typedef struct SvSettingsConfigRecord {
    struct SvSettingsConfigRecord *next;
    char *line;
    int inert; /* recognized compatibility data without an SV consumer */
} SvSettingsConfigRecord;

typedef struct {
    char nick[SV_SETTINGS_TEXT_MAX], name[SV_SETTINGS_TEXT_MAX];
    char meta[SV_SETTINGS_TEXT_MAX], server[SV_SETTINGS_TEXT_MAX];
    char realname[SV_SETTINGS_TEXT_MAX], path[SV_SETTINGS_TEXT_MAX];
    int port, fullauto, fps;
    char layout[16], window_mode[16];
    int window_width, window_height, ui_scale_percent;
    char text_font[SV_SETTINGS_TEXT_MAX], map_font[SV_SETTINGS_TEXT_MAX];
    char text_profile[SV_SETTINGS_TEXT_MAX], map_profile[SV_SETTINGS_TEXT_MAX];
    char graphics_filter[16], pcf_filter[16];
    int graphics, graphics_force_outline, graphic_resize_type;
    char graphic_tiles[SV_SETTINGS_TEXT_MAX];
    int graphic_subsets[10], lighter_dark_blue, window_decorations;
    uint32_t palette[16];
    int sound, cache_audio, audio_sample_rate, audio_channels, audio_buffer;
    char soundpack_folder[SV_SETTINGS_TEXT_MAX], musicpack_folder[SV_SETTINGS_TEXT_MAX];
    int soundpack_subset, musicpack_subset;
    int audio_master, audio_music, audio_sound, audio_weather;
    int volume_master, volume_music, volume_sound, volume_weather;
    int schema_version, overwrite_allowed;
    unsigned diagnostics, unsupported_records;
    SvSettingsConfigRecord *records, *records_tail;
} SvSettingsConfig;

/* Apply one server assignment at its position in CFG/CLI order. A compatible
 * single-colon host:port also assigns port; failure leaves both unchanged. */
bool sv_settings_config_server(SvSettingsConfig *config, const char *value);

/* Initialize a fresh object; dispose it before reinitializing. These APIs never write. */
void sv_settings_config_defaults(SvSettingsConfig *config);
void sv_settings_config_dispose(SvSettingsConfig *config);
/* Parse/load replace the initialized snapshot, including retained records. */
SvSettingsConfigResult sv_settings_config_parse(SvSettingsConfig *config,
    const void *bytes, size_t size);
SvSettingsConfigResult sv_settings_config_load(SvSettingsConfig *config,
    const char *resolved_path);

/* -fFILE applies at its CLI position without resetting earlier assignments. */
SvSettingsConfigResult sv_settings_config_overlay_parse(SvSettingsConfig *config,
    const void *bytes, size_t size);
SvSettingsConfigResult sv_settings_config_overlay_load(SvSettingsConfig *config,
    const char *resolved_path);

#endif
