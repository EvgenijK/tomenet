/* Insert after write_setting. No alternate audio implementation: fixtures feed
 * actual sv_endpoint_run/config/audio/preparation production paths. */
static bool c011_audio_mode(const char *mode)
{ return !strncmp(mode,"c011-audio-",11); }
static const char *c011_audio_config(const char *mode)
{
    return !strcmp(mode,"c011-audio-missing-pack")?
        "svSchemaVersion 1\nsound 1\naudioSampleRate 48000\naudioChannels 4\naudioBuffer 128\ncacheAudio 1\nsoundpackFolder c011-absent\nmusicpackFolder c011-music\nsoundpackSubset 2\nmusicpackSubset 3\n":
        "svSchemaVersion 1\nsound 1\naudioSampleRate 48000\naudioChannels 4\naudioBuffer 128\ncacheAudio 1\nsoundpackFolder c011-sound\nmusicpackFolder c011-music\nsoundpackSubset 2\nmusicpackSubset 3\n";
}
static void c011_audio_write(const char *root,const char *relative,const void *bytes,size_t size)
{
    char path[4096]; assert(snprintf(path,sizeof(path),"%s/%s",root,relative)<(int)sizeof(path));
    FILE *file=fopen(path,"wb");assert(file);
    assert(fwrite(bytes,1,size,file)==size && !fclose(file));
}
static void c011_audio_mkdir(const char *root,const char *relative)
{
    char path[4096]; assert(snprintf(path,sizeof(path),"%s/%s",root,relative)<(int)sizeof(path));
    assert(!mkdir(path,0700) || errno==EEXIST);
}
static void c011_audio_fixture(const char *root,const char *library,const char *mode)
{
    if(!c011_audio_mode(mode))return;
    const char sound[]="packname = Sound base\n2|packname = Sound chosen\nversion = 2\n2|bell = tone.wav\n";
    const char music[]="packname = Music base\n3|packname = Music chosen\nversion = 3\n3|town_generic = tone.wav\n";
    const unsigned char wave[]={
        'R','I','F','F',38,0,0,0,'W','A','V','E','f','m','t',' ',
        16,0,0,0,1,0,1,0,0x44,0xac,0,0,0x88,0x58,1,0,
        2,0,16,0,'d','a','t','a',2,0,0,0,0,0};
    c011_audio_mkdir(root,"xtra");c011_audio_mkdir(root,"xtra/c011-sound");
    c011_audio_mkdir(library,"xtra/c011-music");
    c011_audio_write(root,"xtra/c011-sound/sound.cfg",sound,sizeof(sound)-1);
    c011_audio_write(root,"xtra/c011-sound/tone.wav",wave,sizeof(wave));
    /* Config .default and sample are both absent in U and resolved from B. */
    c011_audio_write(library,"xtra/c011-music/music.cfg.default",music,sizeof(music)-1);
    c011_audio_write(library,"xtra/c011-music/tone.wav",wave,sizeof(wave));
    write_setting(root,"tomenet.cfg",c011_audio_config(mode));
}
static void c011_audio_assert(const char *root,const char *library,const char *mode,
                              const SvPreparationView *view,const SvPreparationResources *resources)
{
    if(!c011_audio_mode(mode))return;
    const SvSettingsAudioSnapshot *audio=&view->audio;
    bool enabled=!strcmp(mode,"c011-audio-enabled");
    SvSettingsAudioStatus expected=enabled?SV_AUDIO_READY:
        !strcmp(mode,"c011-audio-missing-pack")?SV_AUDIO_PACK_FAILED:SV_AUDIO_NO_DEVICE;
    assert(audio->status==expected && audio->sfx==(enabled?1:0) && audio->music==(enabled?1:0));
    assert(view->startup_complete && view->identity_ready && view->profile_ready && view->complete);
    assert(audio->requested_rate==48000 && audio->requested_tracks==4 &&
        audio->requested_buffer==128 && audio->requested_cache);
    assert(!strcmp(resources->requested_sound_pack,!strcmp(mode,"c011-audio-missing-pack")?
        "c011-absent":"c011-sound") && !strcmp(resources->requested_music_pack,"c011-music"));
    assert(resources->requested_sound_subset==2 && resources->requested_music_subset==3);
    if(enabled) {
        assert(audio->effective_rate>0 && audio->effective_tracks==4 &&
            audio->effective_speakers==2 && audio->buffer_supported && audio->effective_buffer>0);
        assert(audio->cache_started && (SDL_WasInit(SDL_INIT_AUDIO)&SDL_INIT_AUDIO));
        assert(!strcmp(audio->sound_name,"Sound chosen") && !strcmp(audio->sound_version,"2"));
        assert(!strcmp(audio->music_name,"Music chosen") && !strcmp(audio->music_version,"3"));
        char expected_path[4096];
        assert(snprintf(expected_path,sizeof(expected_path),"%s/xtra/c011-sound/sound.cfg",root)<(int)sizeof(expected_path));
        assert(!strcmp(audio->sound_config,expected_path));
        assert(snprintf(expected_path,sizeof(expected_path),"%s/xtra/c011-music/music.cfg.default",library)<(int)sizeof(expected_path));
        assert(!strcmp(audio->music_config,expected_path));
    } else {
        assert(!audio->effective_rate && !audio->effective_tracks && !audio->effective_speakers &&
            !audio->effective_buffer && !audio->cache_started && audio->diagnostic[0]);
        assert(!(SDL_WasInit(SDL_INIT_AUDIO)&SDL_INIT_AUDIO));
    }
    /* Config is requested state: neither negotiation nor optional degradation
     * may rewrite it. Excludes changes made by the fixture itself above. */
    char path[4096],actual[1024];
    assert(snprintf(path,sizeof(path),"%s/sv/tomenet.cfg",root)<(int)sizeof(path));
    FILE *file=fopen(path,"rb");assert(file);
    size_t n=fread(actual,1,sizeof(actual)-1,file);assert(!ferror(file)&&!fclose(file));actual[n]=0;
    assert(!strcmp(actual,c011_audio_config(mode)));
    printf("C011 audio status=%d requested=%d/%d/%d effective=%d/%d/%d stereo=%d counts=%d/%d cache=%d complete=%d\n",
        audio->status,audio->requested_rate,audio->requested_tracks,audio->requested_buffer,
        audio->effective_rate,audio->effective_tracks,audio->effective_buffer,audio->effective_speakers,
        audio->sfx,audio->music,audio->cache_started,view->complete);
}
/* Editor integration: call fixture(root,library,argv[3]) after ordinary CFG;
 * use argv[3] retained as mode or Peer.audio_mode. All audio modes reuse PEER_C011
 * success conversation, with peer advertised counts/metadata expected according
 * to enabled boolean. Call assert() after native return and resource getter;
 * after sv_session_preparation_destroy assert SDL_WasInit(AUDIO)==0. Python sets
 * SDL_AUDIODRIVER=sv-unavailable-driver only for c011-audio-no-device. */
