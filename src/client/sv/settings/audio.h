#ifndef SV_SETTINGS_AUDIO_H
#define SV_SETTINGS_AUDIO_H

#include "settings/config.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct SvSettingsAudio SvSettingsAudio;
typedef enum {
    SV_AUDIO_DISABLED, SV_AUDIO_READY, SV_AUDIO_NO_DEVICE,
    SV_AUDIO_PACK_FAILED, SV_AUDIO_REGISTRY_FAILED, SV_AUDIO_RESOURCE_FAILED
} SvSettingsAudioStatus;

typedef struct {
    SvSettingsAudioStatus status;
    int16_t sfx, music;
    /* Full local identities; protocol owner limits only its reporting copy. */
    char sound_name[1024], sound_version[1024];
    char music_name[1024], music_version[1024];
    char sound_config[4096], music_config[4096];
    int requested_rate, effective_rate, requested_tracks, effective_tracks;
    int requested_buffer, effective_buffer; /* 0: no queried effective device */
    int effective_speakers;
    bool requested_cache, cache_started, buffer_supported;
    unsigned warnings;
    char diagnostic[256];
} SvSettingsAudioSnapshot;

/* U is shared SDL root; B is selected library. The config remains requested
 * state. Audio failure is optional degradation, never a login failure.
 * No CFG/OPT or resource writes, no reconfigure/UI/gameplay playback. */
SvSettingsAudio *sv_settings_audio_start(const SvSettingsConfig *config,
    const char *user_root, const char *library_root);
const SvSettingsAudioSnapshot *sv_settings_audio_snapshot(const SvSettingsAudio *audio);
void sv_settings_audio_destroy(SvSettingsAudio *audio);

#endif
