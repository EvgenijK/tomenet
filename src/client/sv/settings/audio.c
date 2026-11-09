#include "settings/audio.h"
#include "files/files.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Startup-only adaptation of snd-sdl3.c. Event indices come from the shipped
 * audio.lua data tables, not a second registry maintained for tests. */
#define SFX_EVENTS 500
#define MUSIC_EVENTS 1000
#define EVENT_FILES 100
#define AUDIO_LINE 4096
#define CONFIG_LIMIT (1024u * 1024u)

typedef struct AudioAsset {
    struct AudioAsset *next;
    char *path;
    MIX_Audio *decoded;
    int event;
    bool music, initial, reference;
} AudioAsset;
typedef struct {
    int event;
    bool initial;
    char name[160];
} AudioReference;
struct SvSettingsAudio {
    SvSettingsAudioSnapshot snapshot;
    MIX_Mixer *mixer;
    MIX_Track *tracks[32];
    SDL_Thread *cache_thread;
    SDL_AtomicInt stopping;
    bool sdl_audio, mixer_initialized;
    AudioAsset *assets, *tail;
    char sfx_names[SFX_EVENTS][80], music_names[MUSIC_EVENTS][80];
    int sfx_names_count, music_names_count;
    unsigned sfx_files[SFX_EVENTS], music_files[MUSIC_EVENTS];
    bool sfx_disabled[SFX_EVENTS], music_disabled[MUSIC_EVENTS];
};

static bool copy_text(char *out, size_t capacity, const char *in)
{
    size_t size = strlen(in);
    if (size >= capacity) return false;
    memcpy(out, in, size + 1);
    return true;
}
static char *trim(char *text)
{
    while (isspace((unsigned char)*text)) ++text;
    size_t size = strlen(text);
    while (size && isspace((unsigned char)text[size - 1])) text[--size] = 0;
    return text;
}
static bool join(char out[4096], const char *base, const char *name)
{
    if (name[0] == '/') return copy_text(out, 4096, name);
    int count = snprintf(out, 4096, "%s%s%s", base,
        *base && base[strlen(base) - 1] == '/' ? "" : "/", name);
    return count >= 0 && count < 4096;
}
static bool readable(const char *path)
{
    struct stat info;
    SvFilePath resolved={.owner=SV_FILE_SHARED};
    if (!copy_text(resolved.path,sizeof(resolved.path),path)) return false;
    return sv_file_stat(&resolved,&info)==0 && S_ISREG(info.st_mode);
}
static void report(SvSettingsAudio *audio, SvSettingsAudioStatus status, const char *reason)
{
    audio->snapshot.status = status;
    (void)copy_text(audio->snapshot.diagnostic, sizeof(audio->snapshot.diagnostic), reason);
}
static bool roots(char user[4096], char bundled[4096], const char *u,
                  const char *b, const char *folder)
{
    char ux[4096], bx[4096];
    return join(ux, u, "xtra") && join(bx, b, "xtra") &&
        join(user, ux, folder) && join(bundled, bx, folder);
}
/* Keep overlay precedence explicit; reading .default does not copy or create
 * a shared resource file as legacy my_fopen did during initialization. */
static FILE *pack_config(char resolved[4096], const char *user,
                         const char *bundled, bool music)
{
    const char *name = music ? "music.cfg" : "sound.cfg";
    const char *fallback = music ? "music.cfg.default" : "sound.cfg.default";
    const char *bases[4] = {user, bundled, user, bundled};
    for (unsigned i = 0; i < 4; ++i) {
        if (!join(resolved, bases[i], i < 2 ? name : fallback)) return NULL;
        SvFilePath path={.owner=(i%2)?SV_FILE_BUNDLED:SV_FILE_SHARED};
        if (!copy_text(path.path,sizeof(path.path),resolved)) return NULL;
        FILE *file = sv_file_open(&path, "rb");
        if (file) return file;
        if (errno != ENOENT && errno != ENOTDIR) return NULL;
    }
    return NULL;
}
/* This is deliberately a literal data reader, not an executable Lua loader.
 * Reject unsupported table expressions rather than inventing event indices. */
static bool registry(SvSettingsAudio *audio, const char *library)
{
    char path[4096], line[AUDIO_LINE];
    if (!join(path, library, "scpt/audio.lua")) return false;
    SvFilePath source={.owner=SV_FILE_BUNDLED};
    if (!copy_text(source.path,sizeof(source.path),path)) return false;
    FILE *file = sv_file_open(&source,"rb");
    if (!file) return false;
    int table = 0;
    bool sound_seen = false, music_seen = false, ok = true;
    size_t bytes = 0;
    while (fgets(line, sizeof(line), file)) {
        bytes += strlen(line);
        if (bytes > CONFIG_LIMIT) { ok = false; break; }
        char *p = trim(line);
        if (!table) {
            if (!strcmp(p, "audio_sfx = {")) { table = 1; sound_seen = true; }
            else if (!strcmp(p, "audio_bgm = {")) { table = 2; music_seen = true; }
            continue;
        }
        if (!*p || !strncmp(p, "--", 2)) continue;
        if (*p == '}') { table = 0; continue; }
        if (*p++ != '"') { ok = false; break; }
        char *end = strchr(p, '"');
        if (!end || memchr(p, '\\', (size_t)(end - p))) { ok = false; break; }
        *end++ = 0;
        end = trim(end);
        if (*end == ',') end = trim(end + 1);
        if (*end && strncmp(end, "--", 2)) { ok = false; break; }
        int *count = table == 1 ? &audio->sfx_names_count : &audio->music_names_count;
        int max = table == 1 ? SFX_EVENTS : MUSIC_EVENTS;
        char (*names)[80] = table == 1 ? audio->sfx_names : audio->music_names;
        if (*count == max || !copy_text(names[*count], sizeof(names[0]), p)) {
            ok = false; break;
        }
        ++*count;
    }
    ok = ok && !ferror(file) && !table && sound_seen && music_seen;
    fclose(file);
    return ok;
}
static int event_index(SvSettingsAudio *audio, const char *name, bool music)
{
    char (*names)[80] = music ? audio->music_names : audio->sfx_names;
    int count = music ? audio->music_names_count : audio->sfx_names_count;
    for (int i = count - 1; i >= 0; --i)
        if (!strcmp(names[i], name)) return i;
    return -1;
}
static bool add_asset(SvSettingsAudio *audio, const char *path, int event,
                      bool music, bool initial, bool reference)
{
    unsigned *count = music ? &audio->music_files[event] : &audio->sfx_files[event];
    if (*count >= EVENT_FILES) { ++audio->snapshot.warnings; return false; }
    AudioAsset *asset = calloc(1, sizeof(*asset));
    if (!asset) return false;
    asset->path = malloc(strlen(path) + 1);
    if (!asset->path) { free(asset); return false; }
    strcpy(asset->path, path);
    asset->event = event; asset->music = music;
    asset->initial = initial; asset->reference = reference;
    if (audio->tail) audio->tail->next = asset;
    else audio->assets = asset;
    audio->tail = asset;
    ++*count;
    return true;
}
/* Config comments preserve quoted '#', and legacy -#/--# markers. */
static void strip_comment(char *line)
{
    bool quoted = false;
    for (char *p = line; *p; ++p) {
        if (*p == '"') quoted = !quoted;
        if (*p == '#' && !quoted) {
            if (p > line && p[-1] == '-') --p;
            if (p > line && p[-1] == '-') --p;
            *p = 0; break;
        }
    }
}
static char *token(char **cursor, bool music, bool *initial, bool *reference)
{
    char *p = trim(*cursor);
    *initial = *reference = false;
    if (music && *p == '!') { *initial = true; ++p; }
    if (music && *p == '+') { *reference = true; ++p; }
    if (!*p) { *cursor = p; return NULL; }
    char *start;
    if (*p == '"') {
        start = ++p;
        p = strchr(p, '"');
        if (!p) { *cursor = start + strlen(start); return NULL; }
    } else {
        start = p;
        while (*p && !isspace((unsigned char)*p)) ++p;
    }
    if (*p) *p++ = 0;
    *cursor = p;
    return start;
}
static bool pack(SvSettingsAudio *audio, const SvSettingsConfig *config,
                  const char *user_root, const char *library, bool music)
{
    char user[4096], bundled[4096], line[AUDIO_LINE], combined[AUDIO_LINE] = {0};
    const char *folder = music ? config->musicpack_folder : config->soundpack_folder;
    int subset = music ? config->musicpack_subset : config->soundpack_subset;
    if (!roots(user, bundled, user_root, library, folder)) return false;
    char *resolved = music ? audio->snapshot.music_config : audio->snapshot.sound_config;
    FILE *file = pack_config(resolved, user, bundled, music);
    if (!file) return false;
    AudioReference *references = music ? calloc(MUSIC_EVENTS, sizeof(*references)) : NULL;
    if (music && !references) { fclose(file); return false; }
    unsigned reference_count = 0;
    char base_name[1024] = {0}, selected_name[1024] = {0};
    char *name = music ? audio->snapshot.music_name : audio->snapshot.sound_name;
    char *version = music ? audio->snapshot.music_version : audio->snapshot.sound_version;
    size_t bytes = 0;
    bool ok = true, continued = false;
    while (fgets(line, sizeof(line), file)) {
        size_t length = strlen(line);
        bytes += length;
        if (bytes > CONFIG_LIMIT) { ok = false; break; }
        if (length == sizeof(line) - 1 && line[length - 1] != '\n') {
            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF) {}
            ++audio->snapshot.warnings; combined[0] = 0; continued = false; continue;
        }
        strip_comment(line);
        char *part = trim(line);
        length = strlen(part);
        bool more = length >= 2 && part[length - 1] == '\\' &&
            isspace((unsigned char)part[length - 2]);
        if (more) part[length - 2] = 0;
        size_t have = strlen(combined), add = strlen(part);
        if (have + add + 2 > sizeof(combined)) { ok = false; break; }
        if (continued && have) strcat(combined, " ");
        strcat(combined, part);
        continued = more;
        if (more) continue;
        char *entry = trim(combined);
        bool disabled = *entry == ';';
        if (disabled) entry = trim(entry + 1);
        int set = 0;
        char *equals = strchr(entry, '='), *pipe = strchr(entry, '|');
        if (!equals) { combined[0] = 0; continue; }
        if (pipe && pipe < equals) {
            *pipe = 0;
            char *end;
            long parsed = strtol(entry, &end, 10);
            if (*trim(end) || parsed < 0 || parsed > 9) {
                ++audio->snapshot.warnings; combined[0] = 0; continue;
            }
            set = (int)parsed; entry = trim(pipe + 1);
        }
        *equals = 0;
        char *key = trim(entry), *value = trim(equals + 1);
        if (!*key || !*value) { combined[0] = 0; continue; }
        bool metadata = !strncmp(key, "packname", 8) || !strncmp(key, "version", 7) ||
            !strncmp(key, "author", 6) || !strncmp(key, "description", 11);
        if (metadata) {
            if (!music && set && set != subset) { combined[0] = 0; continue; }
            if (!strncmp(key, "packname", 8)) {
                char *target = music ? (set ? selected_name : base_name) : name;
                if ((!music || !set || set == subset) && !copy_text(target, 1024, value)) ok = false;
            }
            if (!strncmp(key, "version", 7) && !copy_text(version, 1024, value)) ok = false;
            combined[0] = 0;
            if (!ok) break;
            continue;
        }
        if (set && set != subset) { combined[0] = 0; continue; }
        int event = event_index(audio, key, music);
        if (event < 0) { ++audio->snapshot.warnings; combined[0] = 0; continue; }
        char *cursor = value, *item;
        bool counted = false, initial, reference;
        while ((item = token(&cursor, music, &initial, &reference))) {
            if (music && reference) {
                if (reference_count < MUSIC_EVENTS && copy_text(
                    references[reference_count].name, sizeof(references[0].name), item)) {
                    references[reference_count].event = event;
                    references[reference_count++].initial = initial;
                } else ++audio->snapshot.warnings;
                /* Baseline counts a reference directive even if unresolved. */
                counted = true;
                continue;
            }
            char path[4096];
            if (!join(path, user, item)) { ++audio->snapshot.warnings; continue; }
            if (!readable(path)) {
                if (!join(path, bundled, item) || !readable(path)) {
                    ++audio->snapshot.warnings; continue;
                }
            }
            if (add_asset(audio, path, event, music, initial, false)) counted = true;
            else { ++audio->snapshot.warnings; break; }
        }
        if (counted) {
            int16_t *count = music ? &audio->snapshot.music : &audio->snapshot.sfx;
            if (*count == INT16_MAX) { ok = false; break; }
            ++*count;
        }
        if (disabled) (music ? audio->music_disabled : audio->sfx_disabled)[event] = true;
        combined[0] = 0;
    }
    ok = ok && !ferror(file) && !continued;
    fclose(file);
    if (music && ok) {
        (void)copy_text(name, 1024, *selected_name ? selected_name : base_name);
        for (unsigned i = 0; i < reference_count; ++i) {
            int target = event_index(audio, references[i].name, true);
            if (target < 0) { ++audio->snapshot.warnings; continue; }
            /* Resolve in source order, retaining non-initial unique songs. */
            AudioAsset *stop = audio->tail;
            for (AudioAsset *asset = audio->assets; asset;) {
                AudioAsset *next = asset->next;
                bool last = asset == stop;
                if (asset->music && asset->event == target && !asset->initial) {
                    bool duplicate = false;
                    for (AudioAsset *existing = audio->assets; existing; existing = existing->next)
                        if (existing->music && existing->event == references[i].event &&
                            !strcmp(existing->path, asset->path)) { duplicate = true; break; }
                    if (!duplicate) (void)add_asset(audio, asset->path, references[i].event,
                        true, references[i].initial, true);
                }
                if (last) break;
                asset = next;
            }
        }
    }
    free(references);
    return ok;
}
static int SDLCALL cache_samples(void *context)
{
    SvSettingsAudio *audio = context;
    for (AudioAsset *asset = audio->assets; asset && !SDL_GetAtomicInt(&audio->stopping);
         asset = asset->next) {
        if (!asset->music) asset->decoded = MIX_LoadAudio(audio->mixer, asset->path, true);
    }
    return 0;
}
static void close_device(SvSettingsAudio *audio)
{
    if (audio->cache_thread) {
        SDL_SetAtomicInt(&audio->stopping, 1);
        SDL_WaitThread(audio->cache_thread, NULL);
        audio->cache_thread = NULL;
    }
    for (AudioAsset *asset = audio->assets; asset; asset = asset->next) {
        if (asset->decoded) MIX_DestroyAudio(asset->decoded);
        asset->decoded = NULL;
    }
    for (unsigned i = 0; i < 32; ++i) {
        if (audio->tracks[i]) MIX_DestroyTrack(audio->tracks[i]);
        audio->tracks[i] = NULL;
    }
    if (audio->mixer) MIX_DestroyMixer(audio->mixer);
    audio->mixer = NULL;
    if (audio->mixer_initialized) MIX_Quit();
    audio->mixer_initialized = false;
    if (audio->sdl_audio) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    audio->sdl_audio = false;
    audio->snapshot.effective_rate = 0;
    audio->snapshot.effective_tracks = 0;
    audio->snapshot.effective_speakers = 0;
    audio->snapshot.effective_buffer = 0;
    audio->snapshot.buffer_supported = false;
}
SvSettingsAudio *sv_settings_audio_start(const SvSettingsConfig *config,
    const char *user_root, const char *library_root)
{
    if (!config || !user_root || !library_root) return NULL;
    SvSettingsAudio *audio = calloc(1, sizeof(*audio));
    if (!audio) return NULL;
    SvSettingsAudioSnapshot *state = &audio->snapshot;
    state->requested_rate = config->audio_sample_rate;
    state->requested_tracks = config->audio_channels;
    state->requested_buffer = config->audio_buffer;
    state->requested_cache = config->cache_audio != 0;
    if (!config->sound) { report(audio, SV_AUDIO_DISABLED, "Audio disabled by requested settings"); return audio; }
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        report(audio, SV_AUDIO_NO_DEVICE, "Audio subsystem unavailable; session continues"); return audio;
    }
    audio->sdl_audio = true;
    if (!MIX_Init()) {
        report(audio, SV_AUDIO_NO_DEVICE, "Audio mixer unavailable; session continues");
        close_device(audio); return audio;
    }
    audio->mixer_initialized = true;
    SDL_AudioSpec requested = {SDL_AUDIO_S16, 2, config->audio_sample_rate}, effective;
    char frames[32];
    (void)snprintf(frames, sizeof(frames), "%d", config->audio_buffer);
    const char *prior = SDL_GetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES);
    char *saved_hint = prior ? SDL_strdup(prior) : NULL;
    if (prior && !saved_hint) {
        report(audio, SV_AUDIO_RESOURCE_FAILED, "Audio buffer hint allocation failed");
        close_device(audio); return audio;
    }
    /* NORMAL preserves an explicit environment/override-priority SDL choice.
     * Scope this process-global hint to the initial open and restore it even
     * when the provider fails. SDL may negotiate a different effective size. */
    bool hint_applied = SDL_SetHintWithPriority(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES,
                                               frames, SDL_HINT_NORMAL);
    audio->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &requested);
    if (hint_applied) {
        if (saved_hint) SDL_SetHintWithPriority(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES,
                                               saved_hint, SDL_HINT_NORMAL);
        else SDL_ResetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES);
    }
    SDL_free(saved_hint);
    if (!audio->mixer || !MIX_GetMixerFormat(audio->mixer, &effective)) {
        report(audio, SV_AUDIO_NO_DEVICE, "Audio device unavailable; session continues");
        close_device(audio); return audio;
    }
    state->effective_rate = effective.freq;
    state->effective_speakers = effective.channels;
    SDL_AudioDeviceID device = (SDL_AudioDeviceID)SDL_GetNumberProperty(
        MIX_GetMixerProperties(audio->mixer), MIX_PROP_MIXER_DEVICE_NUMBER, 0);
    SDL_AudioSpec device_format;
    state->buffer_supported = device && SDL_GetAudioDeviceFormat(device,
        &device_format, &state->effective_buffer);
    if (!hint_applied || !state->buffer_supported) ++state->warnings;
    if (config->audio_channels < 4 || config->audio_channels > 32) {
        report(audio, SV_AUDIO_RESOURCE_FAILED, "Invalid requested audio tracks");
        close_device(audio); return audio;
    }
    for (int i = 0; i < config->audio_channels; ++i) {
        audio->tracks[i] = MIX_CreateTrack(audio->mixer);
        if (!audio->tracks[i]) {
            report(audio, SV_AUDIO_RESOURCE_FAILED, "Audio track allocation failed; session continues");
            close_device(audio); return audio;
        }
        ++state->effective_tracks;
    }
    if (!registry(audio, library_root)) {
        report(audio, SV_AUDIO_REGISTRY_FAILED, "Audio event registry missing or unsupported; session continues");
        close_device(audio); return audio;
    }
    if (!pack(audio, config, user_root, library_root, false) ||
        !pack(audio, config, user_root, library_root, true)) {
        state->sfx = state->music = 0;
        report(audio, SV_AUDIO_PACK_FAILED, "Requested audio pack unavailable or malformed; session continues");
        close_device(audio); return audio;
    }
    if (config->cache_audio) {
        audio->cache_thread = SDL_CreateThread(cache_samples, "SV initial audio cache", audio);
        state->cache_started = audio->cache_thread != NULL;
        if (!state->cache_started) ++state->warnings;
    }
    if (!state->buffer_supported)
        report(audio, SV_AUDIO_READY, "Audio initialized; effective device buffer unavailable");
    else if (!hint_applied)
        report(audio, SV_AUDIO_READY, "Audio initialized; external SDL audio buffer override takes precedence");
    else if (state->effective_buffer != state->requested_buffer)
        report(audio, SV_AUDIO_READY, "Audio initialized; device negotiated a different buffer size");
    else report(audio, SV_AUDIO_READY, "Audio initialized");
    return audio;
}
const SvSettingsAudioSnapshot *sv_settings_audio_snapshot(const SvSettingsAudio *audio)
{
    return audio ? &audio->snapshot : NULL;
}
void sv_settings_audio_destroy(SvSettingsAudio *audio)
{
    if (!audio) return;
    close_device(audio);
    while (audio->assets) {
        AudioAsset *asset = audio->assets;
        audio->assets = asset->next;
        free(asset->path); free(asset);
    }
    free(audio);
}
