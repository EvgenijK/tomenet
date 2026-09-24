#ifndef SV_PROFILE_H
#define SV_PROFILE_H
#include <stdbool.h>

/* Startup preferences are owned by U/sv. These are requested values; effective
 * resource/device state is reported separately by the consumer. */
typedef struct {
    bool windowed, wide, cache_audio;
    int ui_scale, graphics, fps;
    int audio_rate, audio_tracks, audio_buffer;
    int sound_subset, music_subset;
    char text_font[4096], map_font[4096], tiles[4096];
    char graphics_filter[16], pcf_filter[16];
    char sound_pack[4096], music_pack[4096];
    bool incompatible_schema;
} SvProfile;

void sv_profile_defaults(SvProfile *profile);
/* Missing or unreadable files keep defaults. Invalid fields are reported and
 * reset. No file is created or modified. */
bool sv_profile_load(SvProfile *profile, const char *user_root);
void sv_profile_report(const SvProfile *profile);

#endif
