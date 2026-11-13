#ifndef SV_RESOURCE_REPORT_H
#define SV_RESOURCE_REPORT_H
/* Shared SV wire implementation for startup and live resource reports.
 * Include after baseline defines.h/pack.h; never mutates a resource owner. */
#include "settings/audio.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    char version[80], os[80], tag[80];
    int branch, build, language;
    int16_t sfx, music, graphics;
    char audio[4][80], tiles[80], font[80];
} SvResourceReport;
static inline void sv_report_copy(char out[80], const char *name)
{
    size_t size = strlen(name);
    if (size > 79) size = 79;
    memcpy(out, name, size); out[size] = 0;
}
static inline void sv_report_init(SvResourceReport *report)
{
    memset(report, 0, sizeof(*report));
    char version[256];
    snprintf(version, sizeof(version), "%s %d.%d.%d.%d.%d.%d%s client (Compiled %s %s for SDL3)",
        TOMENET_VERSION_SHORT, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
        VERSION_EXTRA, VERSION_BRANCH, VERSION_BUILD, SERVER_VERSION_TAG, __DATE__, __TIME__);
    sv_report_copy(report->version, version);
#ifdef WIN32
    sv_report_copy(report->os, "Windows");
#else
    sv_report_copy(report->os, "Linux");
#endif
#ifdef CLIENT_VERSION_TAG
    sv_report_copy(report->tag, CLIENT_VERSION_TAG);
#elif defined(TEST_CLIENT)
    sv_report_copy(report->tag, "-Test");
#endif
    report->branch = VERSION_BRANCH; report->build = VERSION_BUILD;
    report->language = -1; /* baseline: no detected system language */
    /* No map consumer yet. This is not the independently loaded UI text font. */
    sv_report_copy(report->tiles, "NO_GRAPHICS");
}
static inline bool sv_report_audio(SvResourceReport *report, const SvSettingsAudioSnapshot *audio)
{
    if (!audio || audio->sfx < 0 || audio->music < 0) return false;
    const char *names[4] = {audio->sound_name, audio->sound_version,
        audio->music_name, audio->music_version};
    for (unsigned i = 0; i < 4; ++i) if (!memchr(names[i], 0, 1024)) return false;
    bool ready = audio->status == SV_AUDIO_READY;
    report->sfx = ready ? audio->sfx : 0;
    report->music = ready ? audio->music : 0;
    for (unsigned i = 0; i < 4; ++i) sv_report_copy(report->audio[i], ready ? names[i] : "");
    return true;
}
static inline int sv_report_compare(const int a[6], const int b[6])
{
    for (unsigned i = 0; i < 6; ++i) if (a[i] != b[i]) return a[i] > b[i] ? 1 : -1;
    return 0;
}
static inline void sv_report_u16(unsigned char **at, int value)
{
    *(*at)++ = (unsigned char)((unsigned)value >> 8); *(*at)++ = (unsigned char)value;
}
static inline void sv_report_u32(unsigned char **at, int value)
{
    uint32_t v = (uint32_t)value;
    *(*at)++ = (unsigned char)(v >> 24); *(*at)++ = (unsigned char)(v >> 16);
    *(*at)++ = (unsigned char)(v >> 8); *(*at)++ = (unsigned char)v;
}
static inline void sv_report_string(unsigned char **at, const char *value)
{
    size_t size = strlen(value) + 1; memcpy(*at, value, size); *at += size;
}
/* Caller supplies >= 1024 bytes. Maximum packet is < 1024 (seven 80-byte
 * strings plus integers); append atomically only after capacity is checked. */
static inline size_t sv_report_version(const SvResourceReport *r, const int version[6],
    const int samples[60], unsigned char out[1024])
{
    const int supported[6] = {4,8,0,0,0,0}, ping[6] = {4,9,1,0,0,0};
    const int guide[6] = {4,9,2,0,0,0}, packs[6] = {4,9,2,1,0,1}, compact[6] = {4,9,2,1,0,2};
    if (sv_report_compare(version, supported) <= 0) return 0;
    unsigned char *at = out; *at++ = PKT_VERSION;
    sv_report_string(&at, r->version); sv_report_string(&at, r->os);
    if (sv_report_compare(version, ping) > 0) {
        int64_t sum = 0; int count = 0;
        for (unsigned i = 0; i < 60; ++i) if (samples[i] > 0) { sum += samples[i]; ++count; }
        sv_report_u32(&at, count ? (int)(sum / count) : -1);
        if (sv_report_compare(version, guide) > 0) {
            sv_report_u32(&at, -1); /* Guide unavailable; no content I/O */
            if (sv_report_compare(version, compact) < 0) {
                sv_report_u32(&at, r->branch); sv_report_u32(&at, r->build);
            }
            sv_report_string(&at, r->tag); sv_report_u32(&at, r->language);
            if (sv_report_compare(version, packs) >= 0)
                for (unsigned i = 0; i < 4; ++i) sv_report_string(&at, r->audio[i]);
        }
    }
    return (size_t)(at - out);
}
static inline size_t sv_report_audio_packet(const SvResourceReport *r,
    const int version[6], unsigned char out[1024])
{
    const int supported[6] = {4,7,3,0,0,0}, packs[6] = {4,9,2,1,0,1};
    if (sv_report_compare(version, supported) < 0) return 0;
    unsigned char *at = out; *at++ = PKT_AUDIO;
    sv_report_u16(&at, r->sfx); sv_report_u16(&at, r->music);
    if (sv_report_compare(version, packs) >= 0)
        for (unsigned i = 0; i < 4; ++i) sv_report_string(&at, r->audio[i]);
    return (size_t)(at - out);
}
static inline size_t sv_report_font(const SvResourceReport *r,
    const int version[6], unsigned char out[1024])
{
    const int supported[6] = {4,8,1,2,0,0};
    if (sv_report_compare(version, supported) < 0) return 0;
    unsigned char *at = out; *at++ = PKT_FONT;
    sv_report_u16(&at, r->graphics);
    sv_report_string(&at, r->tiles); sv_report_string(&at, r->font);
    return (size_t)(at - out);
}
#endif
