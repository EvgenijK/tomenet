#define _POSIX_C_SOURCE 200809L
#include "settings/audio.h"
#include "protocol/login.h"
#include "../../src/common/pack.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void write_file(const char *root, const char *name, const void *bytes, size_t size)
{
    char path[4096];
    assert(snprintf(path, sizeof(path), "%s/%s", root, name) < (int)sizeof(path));
    FILE *file = fopen(path, "wb");
    assert(file && fwrite(bytes, 1, size, file) == size);
    assert(fclose(file) == 0);
}
static void packets(void)
{
    const int versions[][6] = {{4,7,2,9,0,0}, {4,7,3,0,0,0},
        {4,9,2,1,0,0}, {4,9,2,1,0,1}};
    for (unsigned v = 0; v < 4; ++v) for (unsigned enabled = 0; enabled < 2; ++enabled) {
        const unsigned char iaddr[6] = {0};
        unsigned char bytes[32768], overview[29] = {PKT_SERVERDETAILS};
        SvLogin *login = sv_login_create(versions[v], iaddr); assert(login);
        assert(sv_login_take_output(login, bytes, sizeof(bytes)).result == SV_OK);
        SvSettingsAudioSnapshot audio = {0};
        audio.status = enabled ? SV_AUDIO_READY : SV_AUDIO_NO_DEVICE;
        audio.sfx = 1; audio.music = 2;
        char *names[] = {audio.sound_name, audio.sound_version, audio.music_name, audio.music_version};
        for (unsigned i = 0; i < 4; ++i) {
            memset(names[i], 'A' + i, 120); names[i][120] = 0;
        }
        assert(sv_login_set_audio(login, &audio) == SV_OK);
        assert(sv_login_send_audio(login) == SV_INVALID); /* no playing packet before ACK */
        overview[17] = PKT_LOGIN;
        assert(sv_login_receive(login, overview, sizeof(overview)) == SV_OK);
        assert(sv_login_create_character(login, "Alice", false) == SV_OK);
        assert(sv_login_take_output(login, bytes, sizeof(bytes)).result == SV_OK);
        const unsigned char accepted = E_NEED_INFO;
        assert(sv_login_receive(login, &accepted, 1) == SV_OK);
        const int16_t stats[6] = {0};
        assert(sv_login_start_play(login, 0, 0, 0, 0, stats, "f") == SV_OK);
        assert(sv_login_take_output(login, bytes, sizeof(bytes)).result == SV_OK);
        assert(bytes[9] == 0 && bytes[10] == (enabled ? 1 : 0));
        assert(bytes[11] == 0 && bytes[12] == (enabled ? 2 : 0));
        assert(sv_login_send_audio(login) == SV_INVALID);
        const unsigned char ack[] = {PKT_REPLY, PKT_PLAY, PKT_SUCCESS};
        assert(sv_login_receive(login, ack, sizeof(ack)) == SV_OK);
        assert(sv_login_send_audio(login) == SV_OK);
        SvOutput output = sv_login_take_output(login, bytes, sizeof(bytes));
        if (!v) assert(output.result == SV_WAITING);
        else {
            size_t wanted = v == 3 ? (enabled ? 325 : 9) : 5;
            assert(output.result == SV_OK && output.size == wanted && bytes[0] == PKT_AUDIO);
            assert(bytes[1] == 0 && bytes[2] == (enabled ? 1 : 0));
            assert(bytes[3] == 0 && bytes[4] == (enabled ? 2 : 0));
            if (v == 3) {
                size_t at = 5;
                for (unsigned i = 0; i < 4; ++i) {
                    size_t size = enabled ? 79 : 0;
                    assert(!memcmp(bytes + at, names[i], size) && !bytes[at + size]);
                    at += size + 1;
                    assert(strlen(names[i]) == 120); /* reporting did not truncate owner */
                }
            }
        }
        sv_login_destroy(login);
    }
}
int main(void)
{
    packets();
    char root[] = "/tmp/sv-settings-audio-XXXXXX", path[4096];
    assert(mkdtemp(root));
    const char *dirs[] = {"xtra", "xtra/sound", "xtra/music"};
    for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); ++i) {
        snprintf(path, sizeof(path), "%s/%s", root, dirs[i]);
        assert(mkdir(path, 0700) == 0);
    }
    const char sound[] = "packname = Sound base\n1|packname = Sound chosen\n"
        "version = 2\n;1|bell = \"tone #1.wav\"\n2|page = unavailable.wav\n";
    const char music[] = "1|packname = Music chosen\npackname = Music base\n"
        "version = 3\ngeneric = +town_generic\ntown_generic = \"tone #1.wav\"\n";
    const unsigned char wave[] = {
        'R','I','F','F',38,0,0,0,'W','A','V','E','f','m','t',' ',
        16,0,0,0,1,0,1,0,0x44,0xac,0,0,0x88,0x58,1,0,
        2,0,16,0,'d','a','t','a',2,0,0,0,0,0
    };
    write_file(root, "xtra/sound/sound.cfg", sound, sizeof(sound) - 1);
    write_file(root, "xtra/music/music.cfg", music, sizeof(music) - 1);
    write_file(root, "xtra/sound/tone #1.wav", wave, sizeof(wave));
    write_file(root, "xtra/music/tone #1.wav", wave, sizeof(wave));
    assert(SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy"));
    SvSettingsConfig config;
    sv_settings_config_defaults(&config);
    const int rates[] = {4000, 44100, 48000};
    const int buffers[] = {128, 1024, 8192};
    for (unsigned i = 0; i < 3; ++i) {
        config.audio_sample_rate = rates[i]; config.audio_buffer = buffers[i];
        config.audio_channels = i ? 32 : 4; config.cache_audio = i == 1;
        assert(SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "512"));
        SvSettingsAudio *audio = sv_settings_audio_start(&config, root, "lib");
        assert(audio);
        const SvSettingsAudioSnapshot *state = sv_settings_audio_snapshot(audio);
        assert(state->status == SV_AUDIO_READY);
        assert(state->sfx == 1 && state->music == 2); /* disabled still provided */
        assert(!strcmp(state->sound_name, "Sound chosen"));
        assert(!strcmp(state->music_name, "Music chosen"));
        assert(state->requested_rate == rates[i] && state->effective_rate > 0);
        assert(state->effective_tracks == config.audio_channels);
        assert(state->effective_speakers == 2 && state->buffer_supported);
        assert(state->requested_buffer == buffers[i] && state->effective_buffer > 0);
        assert(!strcmp(SDL_GetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES), "512"));
        printf("audio open requested=%d/%d/%d effective=%d/%d/%d stereo=%d cache=%d\n",
            state->requested_rate, state->requested_tracks, state->requested_buffer,
            state->effective_rate, state->effective_tracks, state->effective_buffer,
            state->effective_speakers, state->cache_started);
        sv_settings_audio_destroy(audio);
    }
    strcpy(config.soundpack_folder, "nonexistent-pack");
    SvSettingsAudio *audio = sv_settings_audio_start(&config, root, "lib");
    assert(audio && sv_settings_audio_snapshot(audio)->status == SV_AUDIO_PACK_FAILED);
    assert(sv_settings_audio_snapshot(audio)->sfx == 0 && !strcmp(config.soundpack_folder, "nonexistent-pack"));
    sv_settings_audio_destroy(audio);
    config.sound = 0;
    audio = sv_settings_audio_start(&config, root, "lib");
    assert(audio && sv_settings_audio_snapshot(audio)->status == SV_AUDIO_DISABLED);
    sv_settings_audio_destroy(audio);
    config.sound = 1;
    assert(SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "sv-unavailable-driver"));
    audio = sv_settings_audio_start(&config, root, "lib");
    assert(audio && sv_settings_audio_snapshot(audio)->status == SV_AUDIO_NO_DEVICE);
    assert(sv_settings_audio_snapshot(audio)->sfx == 0 && sv_settings_audio_snapshot(audio)->music == 0);
    sv_settings_audio_destroy(audio);
    sv_settings_config_dispose(&config);
    const char *files[] = {"xtra/sound/sound.cfg", "xtra/music/music.cfg",
        "xtra/sound/tone #1.wav", "xtra/music/tone #1.wav"};
    for (unsigned i = 0; i < 4; ++i) {
        snprintf(path, sizeof(path), "%s/%s", root, files[i]); assert(unlink(path) == 0);
    }
    for (unsigned i = 3; i > 0; --i) {
        snprintf(path, sizeof(path), "%s/%s", root, dirs[i - 1]); assert(rmdir(path) == 0);
    }
    assert(rmdir(root) == 0);
    puts("settings audio: OK");
    return 0;
}
