#include "login/native-run.h"
#include "settings/startup-ins.h"
#include "login/preparation.h"
#include "protocol/character-identity.h"
#include "login/creation/birth.h"
#include "ui/native-controls.h"
#include "login/ui/endpoint-scene.h"
#include "../../src/common/pack.h"
#include "../../src/common/md5.h"
#include <SDL3/SDL.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>

typedef enum { PEER_SUCCESS, PEER_DISCONNECT, PEER_QUIT, PEER_RETRY, PEER_C010,
    PEER_C011, PEER_C011_REPAIR, PEER_C011_FILE_FAILURE, PEER_C011_QUIT, PEER_C011_CANCEL,
    PEER_C011_RETRY, PEER_C011_CONTROLS, PEER_C011_BIRTH, PEER_C011_AUDIO_ENABLED,
    PEER_C011_AUDIO_MISSING, PEER_C011_AUDIO_NO_DEVICE } PeerMode;
typedef struct {
    int listener;
    PeerMode mode;
    bool contact_exact, verify_exact, login_exact, ping_exact, choice_exact, options_exact, closed;
    const char *root, *library, *case_name;
    bool audio_seen, font_seen;
    unsigned audio_sfx, audio_music;
    char audio_metadata[4][80];
    FILE *transcript;
    unsigned file_acks;
    unsigned char deferred[1024];
    size_t deferred_sizes[24], deferred_size, deferred_count;
    unsigned glyph_width, graphics;
    unsigned char visual_digest[16];
} Peer;

static void write_setting(const char *root, const char *name, const char *text)
{
    char path[4096];
    assert(snprintf(path, sizeof(path), "%s/sv/%s", root, name) < (int)sizeof(path));
    FILE *file = fopen(path, "w"); assert(file);
    assert(fputs(text, file) >= 0 && !fclose(file));
}

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

static void ins_fixture(const char *root,const char *name,const char *text)
{
    SvFilePath path; assert(sv_file_shared_user(&path,root,name));
    FILE *file=sv_file_open(&path,"w"); assert(file);
    assert(fputs(text,file)>=0 && !fclose(file));
}
static void startup_ins_checks(const char *profile)
{
    char root[4096],user[4096];
    assert(snprintf(root,sizeof(root),"%s/ins-fixture",profile)<(int)sizeof(root));
    assert(snprintf(user,sizeof(user),"%s/user",root)<(int)sizeof(user));
    assert(!mkdir(root,0700)); assert(!mkdir(user,0700));
    ins_fixture(root,"global.ins","Auto-Inscriptions file for TomeNET v4.9.4.0.0.0\nGlobal\ng\n  -,-,-,-\n");
    ins_fixture(root,"Warrior.ins","Auto-Inscriptions file for TomeNET v4.9.4.0.0.0\nClass\nc\n  -,-,-,-\n");
    ins_fixture(root,"Hero.ins","Auto-Inscriptions file for TomeNET v4.9.4.0.0.0\n### ignored\nSword\nfirst\n  F,a,b,X\nShield\nsecond\n  -,A,-,-\nSword\nreplacement\n  -,i,-,-\n");
    SvStartupIns *ins=sv_startup_ins_load(root,"Hero","Warrior",NULL,NULL); assert(ins);
    assert(ins->status==SV_INS_READY && ins->count==2 && strstr(ins->source.path,"Hero.ins"));
    assert(!strcmp(ins->rules[0].match,"Sword") && !strcmp(ins->rules[0].tag,"replacement"));
    assert(ins->rules[0].ignore && !ins->rules[0].force && !ins->rules[0].autopickup);
    assert(!strcmp(ins->rules[1].match,"Shield") && ins->rules[1].autodestroy);
    assert(!ins->converted && !ins->conversion_failed); sv_startup_ins_destroy(ins);
    ins_fixture(root,"Truncated.ins","Auto-Inscriptions file for TomeNET v4.9.4.0.0.0\nSword\nkept\n  F,a,b,X\nSword\n");
    ins=sv_startup_ins_load(root,"Truncated",NULL,NULL,NULL);
    assert(ins && ins->status==SV_INS_PARTIAL && ins->count==1);
    assert(!strcmp(ins->rules[0].tag,"kept") && ins->rules[0].force && ins->rules[0].autopickup);
    sv_startup_ins_destroy(ins);
    ins_fixture(root,"Truncated.ins","Auto-Inscriptions file for TomeNET v4.9.4.0.0.0\nSword\nkept\n  F,a,b,X\nSword\nnew\n");
    ins=sv_startup_ins_load(root,"Truncated",NULL,NULL,NULL);
    assert(ins && ins->status==SV_INS_PARTIAL && ins->count==1);
    assert(!strcmp(ins->rules[0].tag,"new") && ins->rules[0].autopickup && ins->rules[0].bags_only);
    sv_startup_ins_destroy(ins);
    ins=sv_startup_ins_load(root,"Absent","Warrior",NULL,NULL); assert(ins && ins->count==1);
    assert(!strcmp(ins->rules[0].match,"Class")); sv_startup_ins_destroy(ins);
    ins_fixture(root,"Ancient.ins","Auto-Inscriptions file for TomeNET v4.7.1a\n!Potion?\nkeep\n");
    ins=sv_startup_ins_load(root,"Ancient",NULL,NULL,NULL); assert(ins && ins->converted);
    assert(ins->format==1 && ins->count==1 && ins->rules[0].force && !strcmp(ins->rules[0].match,"Potion#"));
    sv_startup_ins_destroy(ins);
    ins=sv_startup_ins_load(root,"Ancient",NULL,NULL,NULL); assert(ins && ins->format==5 && !ins->converted);
    assert(ins->count==1 && ins->rules[0].force && !strcmp(ins->rules[0].tag,"keep"));
    sv_startup_ins_destroy(ins);
    /* A directory opens on Linux but fails on read: retain explicit I/O
     * disposition while allowing the baseline most-specific fallback. */
    char path[4096]; assert(snprintf(path,sizeof(path),"%s/Broken.ins",user)<(int)sizeof(path));
    assert(!mkdir(path,0700));
    ins=sv_startup_ins_load(root,"Broken",NULL,NULL,NULL);
    assert(ins && ins->status==SV_INS_READY && ins->warnings && !strcmp(ins->rules[0].match,"Global"));
    sv_startup_ins_destroy(ins);
    SvFilePath global; assert(sv_file_shared_user(&global,root,"global.ins") && !sv_file_remove(&global));
    ins=sv_startup_ins_load(root,"Broken",NULL,NULL,NULL);
    assert(ins && ins->status==SV_INS_IO && ins->warnings && !ins->count); sv_startup_ins_destroy(ins);
}

static bool exact(int socket, void *out, size_t size)
{
    unsigned char *bytes = out;
    while (size) {
        ssize_t count = recv(socket, bytes, size, 0);
        if (count <= 0) return false;
        bytes += count;
        size -= (size_t)count;
    }
    return true;
}

static bool field(int socket, char *out, size_t capacity)
{
    size_t at = 0;
    do {
        if (at + 1 >= capacity || !exact(socket, out + at, 1)) return false;
    } while (out[at++]);
    return true;
}

static void put16(unsigned char **at, uint16_t value)
{
    *(*at)++ = (unsigned char)(value >> 8);
    *(*at)++ = (unsigned char)value;
}

static void put32(unsigned char **at, uint32_t value)
{
    *(*at)++ = (unsigned char)(value >> 24);
    *(*at)++ = (unsigned char)(value >> 16);
    *(*at)++ = (unsigned char)(value >> 8);
    *(*at)++ = (unsigned char)value;
}

static void identity_checks(void)
{
    const int versions[5][6]={{4,4,5,10,0,0},{4,5,2,0,0,0},
        {4,7,2,0,0,0},{4,7,3,0,0,0},{4,9,2,1,0,1}};
    for(unsigned branch=0;branch<5;branch++) {
        unsigned char packet[80],*at=packet;*at++=PKT_CHAR_INFO;
        put16(&at,0);put16(&at,0);
        if(branch>0)put16(&at,0);
        put16(&at,1);
        if(branch==4)put32(&at,0x101);else put16(&at,0x1401);
        if(branch>=3)put16(&at,4);
        if(branch>=2) { memcpy(at,"ServerName",11);at+=11; }
        size_t size=(size_t)(at-packet),used=999;
        SvCharacterIdentity identity={.revision=99};
        assert(sv_character_identity_decode(versions[branch],packet,size-1,&identity,&used)==SV_WAITING);
        assert(identity.revision==99 && !used);
        assert(sv_character_identity_decode(versions[branch],packet,size,&identity,&used)==SV_OK);
        assert(used==size && identity.sex==1 && identity.trait==0);
        assert(identity.trait_available==(branch>0) && identity.name_available==(branch>=2));
        assert(identity.lives==(branch>=3?4:-1));
        if(branch<4)assert(identity.admin_wiz && identity.privileged==1 && identity.mode==1);
        if(branch>=2)assert(!strcmp(identity.name,"ServerName"));
        SvApp *app=sv_app_create((SvAlertSink){0});assert(app && sv_app_open(app,versions[branch])==SV_OK);
        uint64_t generation=sv_app_view(app).generation;
        SvContactSetup setup={.race_count=1,.class_count=1,.trait_count=1};
        assert(sv_app_set_character_setup(app,generation,&setup)==SV_OK);
        assert(sv_app_set_identity_name(app,generation,"Selected")==SV_OK);
        assert(sv_app_receive(app,generation,packet,size-1)==SV_OK);
        assert(!sv_app_step(app,1).processed && !sv_app_view(app).identity.available);
        assert(sv_app_receive(app,generation,packet+size-1,1)==SV_OK && sv_app_step(app,1).processed==1);
        assert(!strcmp(sv_app_view(app).identity.name,branch>=2?"ServerName":"Selected"));
        SvCharacterIdentity first=sv_app_view(app).identity;
        /* A second complete accepted occurrence updates the semantic region,
         * without issuing commands or loading startup data as a view effect. */
        packet[branch>0?8:6]=0; /* sex low byte */
        assert(sv_app_receive(app,generation,packet,size)==SV_OK && sv_app_step(app,1).processed==1);
        SvCharacterIdentity repeated=sv_app_view(app).identity;
        assert(repeated.available && repeated.generation==generation &&
            repeated.revision==first.revision+1 && repeated.sex==0);
        assert(!strcmp(repeated.name,first.name) && repeated.mode==first.mode);
        unsigned char unexpected[64];
        assert(sv_app_take_output(app,generation,unexpected,sizeof(unexpected)).result==SV_WAITING);
        packet[2]=1; /* index 1 is outside the single-entry setup */
        assert(sv_app_receive(app,generation,packet,size)==SV_OK);
        assert(sv_app_step(app,1).result==SV_DECODE_ERROR);
        assert(sv_app_close(app)==SV_OK && sv_app_open(app,versions[branch])==SV_OK);
        assert(!sv_app_view(app).identity.available);
        assert(sv_app_receive(app,generation,packet,size)==SV_STALE);
        assert(sv_app_destroy(app)==SV_OK);
    }
}

typedef struct { unsigned submissions; SvAppView view; } C011RedrawProjection;
static bool c011_redraw_projection(void *context,SvAppView view,bool pause)
{
    C011RedrawProjection *projection=context;
    assert(!pause);projection->submissions++;projection->view=view;return true;
}
/* Include protocol/startup-prefix.h, add helper near existing identity_checks,
 * invoke once beside it. Two necessary untested framing paths, no rendering or
 * alternate behavior model. */
static void c011_prefix_boundary_checks(void)
{
    SvStartupPrefixConfig config={.version={4,4,3,0,0,5},.graphics=0,.char_transfer_bytes=4};
    const unsigned char old[]={PKT_LINE_INFO,0,1,'#',0x41,80};
    size_t consumed=99;
    assert(sv_startup_prefix_decode(&config,old,sizeof(old)-1,&consumed)==SV_WAITING && !consumed);
    assert(sv_startup_prefix_decode(&config,old,sizeof(old),&consumed)==SV_OK && consumed==sizeof(old));
    config=(SvStartupPrefixConfig){.version={4,9,4,0,0,0},.graphics=2,.char_transfer_bytes=4};
    /* One 80-column run: foreground u32, RLE marker, background u32/attr,
     * then true foreground attr and repeat. Coalesced following byte is retained. */
    const unsigned char dual[]={PKT_LINE_INFO,0,2,0,1,2,3,255,0,4,5,6,2,1,80,PKT_CHAR_INFO};
    consumed=99;
    assert(sv_startup_prefix_decode(&config,dual,sizeof(dual)-2,&consumed)==SV_WAITING && !consumed);
    assert(sv_startup_prefix_decode(&config,dual,sizeof(dual),&consumed)==SV_OK && consumed==sizeof(dual)-1);
    unsigned char malformed[sizeof(dual)];memcpy(malformed,dual,sizeof(dual));malformed[sizeof(dual)-2]=81;
    assert(sv_startup_prefix_decode(&config,malformed,sizeof(malformed),&consumed)==SV_DECODE_ERROR && !consumed);
    unsigned char equip[116]={PKT_EQUIP,'a'};
    memset(equip+14,'E',100);equip[114]=0;equip[115]=PKT_CHAR_INFO;
    assert(sv_startup_prefix_decode(&config,equip,114,&consumed)==SV_WAITING && !consumed);
    assert(sv_startup_prefix_decode(&config,equip,sizeof(equip),&consumed)==SV_OK && consumed==115);
    unsigned char equip_wide[35]={PKT_EQUIP_WIDE,'a'};
    equip_wide[14]='B';equip_wide[34]=PKT_CHAR_INFO;
    assert(sv_startup_prefix_decode(&config,equip_wide,33,&consumed)==SV_WAITING && !consumed);
    assert(sv_startup_prefix_decode(&config,equip_wide,sizeof(equip_wide),&consumed)==SV_OK && consumed==34);
}

static void redraw_checks(void)
{
    const int version[6]={4,9,4,0,0,0};
    SvApp *app=sv_app_create((SvAlertSink){0});assert(app && sv_app_open(app,version)==SV_OK);
    uint64_t generation=sv_app_view(app).generation;
    unsigned char key=18,output[65536];
    assert(sv_app_begin_quantity(app,generation,1,99,false)==SV_OK);
    assert(sv_app_physical(app,generation,&key,1)==SV_OK);sv_app_dispatch_input(app,8);
    assert(!sv_app_view(app).redraw_revision);
    assert(sv_app_take_output(app,generation,output,sizeof(output)).result==SV_WAITING);
    assert(sv_app_quantity_edit(app,generation,SV_EDIT_CANCEL)==SV_OK);
    int32_t quantity;assert(sv_app_take_quantity(app,generation,&quantity)==SV_OK);
    C011RedrawProjection projection={0};
    assert(sv_app_set_control_sink(app,(SvControlSink){&projection,c011_redraw_projection,NULL})==SV_OK);
    const unsigned char initial[]={PKT_HP,0,200,0,100,0,PKT_MESSAGE,'b','e','f','o','r','e',0};
    assert(sv_app_receive(app,generation,initial,sizeof(initial))==SV_OK);
    assert(sv_app_step(app,2).processed==2);
    assert(sv_app_view(app).status.current==100 && sv_app_view(app).messages.count==1);
    size_t filled=0;
    while(sv_app_raw_key(app,generation,'x')==SV_OK) { assert(++filled<65536); }
    assert(filled);
    assert(sv_app_physical(app,generation,&key,1)==SV_OK);sv_app_dispatch_input(app,8);
    assert(!sv_app_view(app).redraw_revision);
    const unsigned char updated[]={PKT_HP,0,200,0,127,0,PKT_MESSAGE,'a','f','t','e','r',0};
    assert(sv_app_receive(app,generation,updated,sizeof(updated))==SV_OK);
    assert(sv_app_take_output(app,generation,output,sizeof(output)).size>0);
    assert(sv_app_step(app,2).processed==2);
    SvOutput redraw=sv_app_take_output(app,generation,output,sizeof(output));
    assert(redraw.size==2 && output[0]==PKT_REDRAW && !output[1]);
    assert(sv_app_view(app).redraw_revision==1);
    /* The queued redraw precedes the newer network occurrences. Its render
     * submission contains one coherent old view, never a mix of both states. */
    assert(projection.submissions==1 && projection.view.status.current==100 &&
        projection.view.messages.count==1 &&
        !strcmp((const char *)projection.view.messages.lines[0].bytes,"before"));
    assert(sv_app_view(app).status.current==127 && sv_app_view(app).messages.count==2);
    sv_app_step(app,1);assert(sv_app_take_output(app,generation,output,sizeof(output)).result==SV_WAITING);
    assert(sv_app_physical(app,generation,&key,1)==SV_OK);
    SvOutput refreshed=sv_app_take_output(app,generation,output,sizeof(output));
    assert(refreshed.result==SV_OK && refreshed.size==2 && output[0]==PKT_REDRAW && !output[1]);
    assert(projection.submissions==2 && projection.view.redraw_revision==2 &&
        projection.view.status.current==127 && projection.view.messages.count==2 &&
        !strcmp((const char *)projection.view.messages.lines[1].bytes,"after"));
    SvMessage message;
    assert(sv_app_take_message(app,generation,&message)==SV_OK && !strcmp((const char *)message.bytes,"before"));
    assert(sv_app_take_message(app,generation,&message)==SV_OK && !strcmp((const char *)message.bytes,"after"));
    assert(sv_app_take_message(app,generation,&message)==SV_WAITING); /* redraw never consumes feed delivery */
    assert(sv_app_close(app)==SV_OK && sv_app_open(app,version)==SV_OK);
    assert(sv_app_physical(app,generation,&key,1)==SV_STALE);
    assert(!sv_app_view(app).redraw_revision);assert(sv_app_destroy(app)==SV_OK);
}

static void committed_birth_check(void)
{
    const int version[6]={4,9,4,0,0,0};
    const uint64_t generation=7;
    SvContactSetup setup={.race_count=1,.class_count=1,.trait_count=1,.body_step_available=1};
    setup.races[0].choice=1;setup.traits[0].choice=1;
    strcpy(setup.races[0].title,"Human");strcpy(setup.classes[0].title,"Warrior");
    strcpy(setup.traits[0].title,"None");
    uint32_t flags[4]={0};int16_t stats[6]={0};SvBirth birth;
    assert(sv_birth_begin(&birth,generation,&setup,flags,false,0,false,SV_BIRTH_SEX_MALE,
        false,0,false,0,false,0,false,SV_BIRTH_BODY_NORMAL,false,stats,false,SV_BIRTH_MODE_NORMAL)==SV_OK);
    SvBirthPlayDraft draft;
    assert(!sv_birth_committed_play(&birth,generation,&draft));
    for(unsigned step=0;step<32 && !sv_birth_committed_play(&birth,generation,&draft);step++) {
        SvBirthView view;sv_birth_view_prepare(&view,&birth);
        if(view.owner==SV_BIRTH_STATS && view.legal_stat_count)
            sv_birth_command(&birth,generation,(SvBirthCommand){.kind=SV_BIRTH_SELECT_STAT,.stat=view.legal_stats[0]});
        sv_birth_command(&birth,generation,(SvBirthCommand){.kind=SV_BIRTH_CONFIRM});
    }
    assert(sv_birth_committed_play(&birth,generation,&draft));
    unsigned char iaddr[6]={0},bytes[32768],*at=bytes;
    SvLogin *login=sv_login_create(version,iaddr);assert(login);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_OK);
    at=bytes;*at++=PKT_SERVERDETAILS;
    for(unsigned i=0;i<4;i++)put32(&at,0);
    *at++=PKT_LOGIN;put16(&at,0);*at++=0;*at++=0;
    put16(&at,0);put16(&at,0);put16(&at,0);*at++=0;
    assert(sv_login_receive(login,bytes,(size_t)(at-bytes))==SV_OK);
    assert(sv_login_create_character(login,"Born",false)==SV_OK);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_OK);
    const unsigned char accepted=0;
    assert(sv_login_receive(login,&accepted,1)==SV_OK && sv_login_state(login)==SV_LOGIN_SELECTED);
    assert(sv_login_start_play(login,draft.mode,draft.race,draft.class_id,draft.trait,draft.stats,"map.fon")==SV_OK);
    SvOutput play=sv_login_take_output(login,bytes,sizeof(bytes));
    assert(play.result==SV_OK && play.size>16000 && bytes[0]==PKT_PLAY);
    assert(bytes[1]==(unsigned char)(draft.mode>>8) && bytes[2]==(unsigned char)draft.mode);
    assert(bytes[4]==draft.race && bytes[6]==draft.class_id && bytes[8]==draft.trait);
    assert(sv_login_start_play(login,draft.mode,draft.race,draft.class_id,draft.trait,draft.stats,"map.fon")==SV_INVALID);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_WAITING);
    sv_birth_close(&birth,generation);assert(!sv_birth_committed_play(&birth,generation,&draft));
    sv_login_destroy(login);
}

static void push_event(Uint32 type, SDL_Keycode key)
{
    SDL_Event event = {0};
    event.type = type;
    if (type == SDL_EVENT_KEY_DOWN) event.key.key = key;
    assert(SDL_PushEvent(&event));
}

static void push_text(const char *text)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_TEXT_INPUT;
    event.text.text = text;
    assert(SDL_PushEvent(&event));
}


#ifdef SV_C011_TEST_WRAPPERS
/* Wrappers observe real owner-thread execution. They neither decode packets nor
 * mutate production state; only the semantic-view driver queues native input. */
static struct {
    bool enabled;
    SvNativeControls *active, *latest;
    SvApp *app;
    uint64_t previous_deadline, last_present, pause_epoch[3];
    unsigned present_count, pause_presents, stale_inputs, fresh_acks;
    bool stale_first_sent, fresh_first_sent, stale_second_sent, fresh_second_sent, text_stopped;
    unsigned flushes, paced, unpaced;
    struct { uint64_t frame_ms,due_ms,deadline_ns,present_ns; } flush[16];
} controls_probe;
static SDL_AtomicInt controls_ack_count;
static const char *capture_pending;
static unsigned capture_seen;
static bool capture_identity_ready;
static void capture_presented_frame(SDL_Renderer *renderer)
{
    const char *directory=getenv("SV_C011_CAPTURE_DIR");
    if(!directory || !capture_pending)return;
    char path[4096];assert(snprintf(path,sizeof(path),"%s/%s.bmp",directory,capture_pending)<(int)sizeof(path));
    SDL_Surface *pixels=SDL_RenderReadPixels(renderer,NULL);assert(pixels);
    assert(SDL_SaveBMP(pixels,path));SDL_DestroySurface(pixels);capture_pending=NULL;
}

bool __real_SDL_RenderPresent(SDL_Renderer *);
bool __wrap_SDL_RenderPresent(SDL_Renderer *renderer)
{
    capture_presented_frame(renderer);
    bool result=__real_SDL_RenderPresent(renderer);
    if(result && controls_probe.enabled && controls_probe.active &&
       renderer==controls_probe.active->renderer) {
        controls_probe.last_present=SDL_GetTicksNS();controls_probe.present_count++;
        SvAppView view=sv_app_view(controls_probe.app);
        if(view.paused && view.pause_sequence>controls_probe.pause_presents) {
            assert(view.pause_sequence<3);
            controls_probe.pause_presents=(unsigned)view.pause_sequence;
        }
    }
    return result;
}
SvStep __real_sv_native_controls_step(SvNativeControls *,SvApp *,size_t);
SvStep __wrap_sv_native_controls_step(SvNativeControls *controls,SvApp *app,size_t budget)
{
    if(!controls_probe.enabled) {
        SvStep step=__real_sv_native_controls_step(controls,app,budget);
        if(sv_app_view(app).identity.available)capture_identity_ready=true;
        return step;
    }
    SvAppView before=sv_app_view(app);
    controls_probe.active=controls;controls_probe.latest=controls;controls_probe.app=app;
    uint64_t start=SDL_GetTicksNS();
    SvStep step=__real_sv_native_controls_step(controls,app,budget);
    SvAppView after=sv_app_view(app);
    controls_probe.active=NULL;
    if(after.paused && after.pause_sequence<3)
        controls_probe.pause_epoch[after.pause_sequence]=controls->input->since_ns;
    if(after.flush_sequence>before.flush_sequence) {
        assert(after.flush_sequence==before.flush_sequence+1 && controls_probe.flushes<16);
        assert(controls_probe.last_present>=start);
        if(controls_probe.previous_deadline)assert(controls_probe.last_present>=controls_probe.previous_deadline);
        unsigned i=controls_probe.flushes++;
        controls_probe.flush[i].frame_ms=controls->frame_ms;
        controls_probe.flush[i].due_ms=after.flush_due_ms;
        controls_probe.flush[i].deadline_ns=controls->due_ns;
        controls_probe.flush[i].present_ns=controls_probe.last_present;
        if(after.flush_due_ms>controls->frame_ms)controls_probe.paced++;
        else controls_probe.unpaced++;
        controls_probe.previous_deadline=controls->due_ns;
    }
    return step;
}
bool __real_sv_native_input(SvNativeInput *,SvApp *,const SDL_Event *);
bool __wrap_sv_native_input(SvNativeInput *input,SvApp *app,const SDL_Event *event)
{
    bool paused=sv_app_view(app).paused;
    if(controls_probe.enabled) { controls_probe.active=controls_probe.latest;controls_probe.app=app; }
    bool handled=__real_sv_native_input(input,app,event);
    if(controls_probe.enabled) {
        controls_probe.active=NULL;
        if(paused && handled && input->last_result==SV_STALE)controls_probe.stale_inputs++;
        if(paused && handled && input->last_result==SV_OK) {
            assert(event->common.timestamp);
            controls_probe.fresh_acks++;
            SvAppView after=sv_app_view(app);
            if(after.paused && after.pause_sequence<3)
                controls_probe.pause_epoch[after.pause_sequence]=input->since_ns;
            SDL_SetAtomicInt(&controls_ack_count,(int)controls_probe.fresh_acks);
        }
    }
    return handled;
}
static void controls_queue_key(uint64_t timestamp,SDL_Keycode key)
{
    SDL_Event event={.type=SDL_EVENT_KEY_DOWN};
    event.key.key=key;event.key.scancode=key==SDLK_RETURN?SDL_SCANCODE_RETURN:SDL_SCANCODE_SPACE;event.key.timestamp=timestamp;
    assert(SDL_PeepEvents(&event,1,SDL_ADDEVENT,0,0)==1);
}
static void controls_native_gestures(void)
{
    if(!controls_probe.enabled || !controls_probe.latest || !controls_probe.app)return;
    SvAppView view=sv_app_view(controls_probe.app);
    if(!view.paused) {
        if(controls_probe.fresh_acks==2) {
            assert(!SDL_TextInputActive(SDL_GetRenderWindow(controls_probe.latest->renderer)));
            controls_probe.text_stopped=true;
        }
        return;
    }
    assert(SDL_TextInputActive(SDL_GetRenderWindow(controls_probe.latest->renderer)));
    uint64_t epoch=controls_probe.latest->input->since_ns;
    if(view.pause_sequence==1 && !controls_probe.stale_first_sent) {
        controls_probe.stale_first_sent=true;controls_queue_key(epoch-1,SDLK_RETURN);
    } else if(view.pause_sequence==1 && controls_probe.stale_inputs>=1 && !controls_probe.fresh_first_sent) {
        controls_probe.fresh_first_sent=true;assert(SDL_GetTicksNS()>epoch);
        uint64_t timestamp=SDL_GetTicksNS();controls_queue_key(timestamp,SDLK_SPACE);
        SDL_Event text={.type=SDL_EVENT_TEXT_INPUT};text.text.text=" ";text.text.timestamp=timestamp;
        assert(SDL_PeepEvents(&text,1,SDL_ADDEVENT,0,0)==1);
    } else if(view.pause_sequence==2 && !controls_probe.stale_second_sent) {
        controls_probe.stale_second_sent=true;
        SDL_Event tail={.type=SDL_EVENT_TEXT_INPUT};tail.text.text=" ";
        tail.text.timestamp=controls_probe.pause_epoch[1];
        assert(SDL_PeepEvents(&tail,1,SDL_ADDEVENT,0,0)==1);
    } else if(view.pause_sequence==2 && controls_probe.stale_inputs>=2 && !controls_probe.fresh_second_sent) {
        controls_probe.fresh_second_sent=true;assert(SDL_GetTicksNS()>epoch);controls_queue_key(SDL_GetTicksNS(),SDLK_RETURN);
    }
}
/* Test observation/SDL gestures only. Based on the accepted birth-complete.c
 * rendered-view driver; no mutation of production birth state. Include after
 * push_text declaration. Set native_birth_driver.enabled before endpoint run. */
#include "login/ui/endpoint-scene.h"
#include "login/login-view.h"
#include "login/creation/birth-interaction.h"
static struct {
    bool enabled, name_started, name_submitted, mode_confirmed, prepared_visible, motd_acknowledged;
    unsigned submitted_owners, stat_step;
    bool stat_selected;
    unsigned rendered_owners;
} native_birth_driver;

static void native_birth_key(SDL_Keycode key,SDL_Keymod modifiers)
{
    SDL_Event event={.type=SDL_EVENT_KEY_DOWN};
    event.key.key=key;event.key.mod=modifiers;
    assert(SDL_PushEvent(&event));
}

bool __real_sv_endpoint_draw(SDL_Renderer *,SvFont *,const SvEndpoint *,
    const SvEndpointView *,SvEndpointRenderMetrics *);
bool __wrap_sv_endpoint_draw(SDL_Renderer *renderer,SvFont *font,const SvEndpoint *endpoint,
    const SvEndpointView *view,SvEndpointRenderMetrics *metrics)
{
    bool drawn=__real_sv_endpoint_draw(renderer,font,endpoint,view,metrics);
    controls_native_gestures();
    if(drawn && getenv("SV_C011_CAPTURE_DIR")) {
        const SvBirthView *snapshot=view->birth_view;
        unsigned flag=snapshot && snapshot->owner==SV_BIRTH_SEX?1:
            snapshot && snapshot->owner==SV_BIRTH_STATS?2:
            snapshot && snapshot->owner==SV_BIRTH_MODE?4:
            capture_identity_ready && view->preparation_identity?8:0;
        if(flag && !(capture_seen&flag)) {
            capture_seen|=flag;capture_pending=flag==1?"birth-sex":flag==2?"birth-stats":
                flag==4?"birth-mode":"prepared";
        }
    }
    if(!drawn || !native_birth_driver.enabled)return drawn;
    const SvLoginView *login=view->login_view;
    if(view->preparation_identity) {
        assert(login && login->live_handoff && !view->birth_view && !view->birth_viewport);
        assert(view->contact_status && strstr(view->contact_status,"Accepted"));
        native_birth_driver.prepared_visible=true;
    }
    if(login && login->motd && native_birth_driver.mode_confirmed && !native_birth_driver.motd_acknowledged) {
        assert(!view->preparation_identity);
        native_birth_driver.motd_acknowledged=true;native_birth_key(SDLK_SPACE,0);
    }
    if(login && login->overview && !native_birth_driver.name_started) {
        native_birth_driver.name_started=true;
        native_birth_key(SDLK_N,SDL_KMOD_SHIFT);
    } else if(login && login->ordinary_name_entry && !native_birth_driver.name_submitted) {
        native_birth_driver.name_submitted=true;
        push_text("Born");native_birth_key(SDLK_RETURN,0);
    }
    const SvBirthView *birth=view->birth_view;
    if(!birth)return drawn;
    native_birth_driver.rendered_owners|=1u<<birth->owner;
    SDL_Keycode key=SDLK_UNKNOWN;
    if(birth->owner==SV_BIRTH_MODE) {
        assert(birth->stats_confirmed && birth->stats_order_count==6);
        native_birth_driver.stat_step=birth->stats_order_count;
        if(!native_birth_driver.mode_confirmed) {
            native_birth_driver.mode_confirmed=true;key=SDLK_RETURN;
        }
    } else if(birth->owner==SV_BIRTH_STATS) {
        assert(!birth->stats_point_buy);
        if(birth->stats_order_count>native_birth_driver.stat_step) {
            assert(birth->stats_order_count==native_birth_driver.stat_step+1);
            native_birth_driver.stat_step++;native_birth_driver.stat_selected=false;
        }
        if(!native_birth_driver.stat_selected) {
            key=SDLK_A+native_birth_driver.stat_step;
            native_birth_driver.stat_selected=true;
        } else if(birth->highlighted_stat==(int)native_birth_driver.stat_step &&
            !(native_birth_driver.submitted_owners&(1u<<(16+native_birth_driver.stat_step)))) {
            native_birth_driver.submitted_owners|=1u<<(16+native_birth_driver.stat_step);
            key=SDLK_RETURN;
        }
    } else if(!(native_birth_driver.submitted_owners&(1u<<birth->owner))) {
        switch(birth->owner) {
        case SV_BIRTH_SEX: case SV_BIRTH_CLASS: case SV_BIRTH_RACE:
        case SV_BIRTH_TRAIT: case SV_BIRTH_BODY:key=SDLK_RETURN;break;
        default:break;
        }
        native_birth_driver.submitted_owners|=1u<<birth->owner;
    }
    if(key!=SDLK_UNKNOWN)native_birth_key(key,0);
    return drawn;
}

#endif

static int credential_run(void *unused)
{
    (void)unused;
    for (int i = 0; i < 200 && !(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO); ++i)
        SDL_Delay(5);
    /* Wait for the native editor's actual readiness; bootstrap resources now
     * run between VIDEO init and the first input boundary. */
    SDL_Window *window = NULL;
    for (int i = 0; i < 2000; ++i) {
        int count = 0;
        SDL_Window **windows = SDL_GetWindows(&count);
        window = count == 1 ? windows[0] : NULL;
        SDL_free(windows);
        if (window && SDL_TextInputActive(window)) break;
        SDL_Delay(5);
    }
    assert(window && SDL_TextInputActive(window));
    SDL_Delay(100);
    /* Settings bootstrap now prefills the configured account. Replace it
     * through the production editor, just as a user entering this fixture ID. */
    SDL_Event select_all = {.type = SDL_EVENT_KEY_DOWN};
    select_all.key.key = SDLK_A;
    select_all.key.scancode = SDL_SCANCODE_A;
    select_all.key.mod = SDL_KMOD_CTRL;
    assert(SDL_PushEvent(&select_all));
    push_text("Test");
    push_event(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
    push_text("pw");
    push_event(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
    return 0;
}

/* FILE fixture input/observations only; all client behavior remains production. */
static void wire_log(Peer *peer, const char *direction, const unsigned char *bytes, size_t size)
{
    assert(peer->transcript);
    fprintf(peer->transcript, "%s ", direction);
    for (size_t i=0;i<size;++i) fprintf(peer->transcript,"%02x",bytes[i]);
    fputc('\n',peer->transcript); fflush(peer->transcript);
}
static void send_all(int socket, const void *data, size_t size)
{
    const unsigned char *bytes=data;
    while(size) { ssize_t sent=send(socket,bytes,size,MSG_NOSIGNAL); assert(sent>0);
        bytes+=sent;size-=(size_t)sent; }
}
static void file_send(Peer *peer,int socket,const unsigned char *packet,size_t size)
{ wire_log(peer,"send",packet,size);send_all(socket,packet,size); }
static void first_packet_byte(int socket,unsigned char *byte)
{
    do { assert(exact(socket,byte,1)); } while(*byte==PKT_KEEPALIVE);
}
static void file_reply(Peer *peer,int socket,unsigned command,unsigned id,
                       const unsigned char *digest)
{
    unsigned char packet[20];first_packet_byte(socket,packet);
    assert(packet[0]==PKT_FILE && exact(socket,packet+1,3));
    assert(packet[1]==command && packet[2]==(unsigned char)(id>>8) && packet[3]==(unsigned char)id);
    size_t size=command==PKT_FILE_SUM?20:4;
    if(size>4) assert(exact(socket,packet+4,16) && !memcmp(packet+4,digest,16));
    wire_log(peer,"recv",packet,size);
    if(command==PKT_FILE_ACK) ++peer->file_acks;
}
static void no_file_reply(int socket)
{
    Uint64 until=SDL_GetTicks()+40;
    while(SDL_GetTicks()<until) {
        Uint64 now=SDL_GetTicks();if(now>=until) break;
        struct pollfd wait={.fd=socket,.events=POLLIN};
        int result=poll(&wait,1,(int)(until-now));assert(result>=0);
        if(!result) break;
        unsigned char byte;assert(exact(socket,&byte,1) && byte==PKT_KEEPALIVE);
    }
}
static unsigned char *read_guide(const Peer *peer,size_t *size)
{
    char path[4096];assert(snprintf(path,sizeof(path),"%s/scpt/guide.lua",peer->library)<(int)sizeof(path));
    FILE *file=fopen(path,"rb");assert(file && !fseek(file,0,SEEK_END));
    long length=ftell(file);assert(length>1024 && length<1024*1024 && !fseek(file,0,SEEK_SET));
    unsigned char *bytes=malloc((size_t)length+128);assert(bytes);
    assert(fread(bytes,1,(size_t)length,file)==(size_t)length && !fclose(file));
    *size=(size_t)length;return bytes;
}
static void guide_matches(const Peer *peer,const unsigned char *expected,size_t size)
{
    size_t actual_size;unsigned char *actual=read_guide(peer,&actual_size);
    assert(actual_size==size && !memcmp(actual,expected,size));free(actual);
}
static void transfer_conversation(Peer *peer,int socket)
{
    const unsigned id=73;
    unsigned char packet[1030],*at=packet;
    *at++=PKT_SERVERDETAILS;put32(&at,8);put32(&at,0x84);put32(&at,0);put32(&at,0x1f);
    send_all(socket,packet,(size_t)(at-packet));
    size_t original_size;unsigned char *original=read_guide(peer,&original_size);
    unsigned char digest[16];MD5_CTX md5;MD5Init(&md5);MD5Update(&md5,original,(unsigned)original_size);MD5Final(digest,&md5);
    at=packet;*at++=PKT_FILE;*at++=PKT_FILE_CHECK;put16(&at,id);memcpy(at,"scpt/guide.lua",15);at+=15;
    file_send(peer,socket,packet,(size_t)(at-packet));file_reply(peer,socket,PKT_FILE_SUM,id,digest);
    packet[1]=PKT_FILE_INIT;file_send(peer,socket,packet,(size_t)(at-packet));
    file_reply(peer,socket,PKT_FILE_ACK,id,NULL);
    static const char marker[]="\n_ALERT(\"C010 native transferred guide marker\")\n";
    unsigned char *changed=malloc(original_size+sizeof(marker));assert(changed);
    memcpy(changed,original,original_size);memcpy(changed+original_size,marker,sizeof(marker)-1);
    size_t changed_size=original_size+sizeof(marker)-1;
    for(size_t offset=0;offset<changed_size;) {
        size_t amount=changed_size-offset;if(amount>1024) amount=1024;
        at=packet;*at++=PKT_FILE;*at++=PKT_FILE_DATA;put16(&at,id);put16(&at,(uint16_t)amount);
        memcpy(at,changed+offset,amount);size_t frame=6+amount;
        if(!offset) {
            wire_log(peer,"send",packet,frame);
            send_all(socket,packet,9);no_file_reply(socket);guide_matches(peer,original,original_size);
            send_all(socket,packet+9,frame-9);
        } else file_send(peer,socket,packet,frame);
        file_reply(peer,socket,PKT_FILE_ACK,id,NULL);offset+=amount;
    }
    at=packet;*at++=PKT_FILE;*at++=PKT_FILE_END;put16(&at,id);
    wire_log(peer,"send",packet,4);send_all(socket,packet,3);no_file_reply(socket);
    guide_matches(peer,original,original_size);send_all(socket,packet+3,1);
    file_reply(peer,socket,PKT_FILE_ACK,id,NULL);guide_matches(peer,changed,changed_size);
    fprintf(peer->transcript,"publication bytes=%zu data_acks=%u split_data=9+1021 split_end=3+1\n",changed_size,peer->file_acks-2);
    fflush(peer->transcript);free(changed);free(original);
}

static void preparation_transfer(Peer *peer,int socket,unsigned attempt)
{
    unsigned char packet[1100],*at=packet;
    *at++=PKT_SERVERDETAILS;put32(&at,8);put32(&at,0);put32(&at,0);put32(&at,0);
    file_send(peer,socket,packet,(size_t)(at-packet));
    at=packet;*at++=PKT_FILE;*at++=PKT_FILE_INIT;put16(&at,91);
    const char *target=peer->mode==PEER_C011_REPAIR?"scpt/c-init.lua":
        peer->mode==PEER_C011_RETRY?(attempt?"c011-after-retry.txt":"scpt/guide.lua"):"scpt/blocked.lua";
    memcpy(at,target,strlen(target)+1);at+=strlen(target)+1;
    file_send(peer,socket,packet,(size_t)(at-packet));file_reply(peer,socket,PKT_FILE_ACK,91,NULL);
    if(peer->mode==PEER_C011_REPAIR || (peer->mode==PEER_C011_RETRY && !attempt)) {
        char path[4096];assert(snprintf(path,sizeof(path),"%s/%s",peer->library,
            peer->mode==PEER_C011_REPAIR?"scpt/c-init.restore":"scpt/guide.lua")<(int)sizeof(path));
        FILE *source=fopen(path,"rb");assert(source);
        size_t n;
        while((n=fread(packet+6,1,1024,source))) {
            at=packet;*at++=PKT_FILE;*at++=PKT_FILE_DATA;put16(&at,91);put16(&at,(uint16_t)n);
            file_send(peer,socket,packet,n+6);file_reply(peer,socket,PKT_FILE_ACK,91,NULL);
        }
        assert(!ferror(source) && !fclose(source));
    }
    at=packet;*at++=PKT_FILE;*at++=PKT_FILE_END;put16(&at,91);
    file_send(peer,socket,packet,4);
    file_reply(peer,socket,peer->mode==PEER_C011_FILE_FAILURE?PKT_FILE_ERR:PKT_FILE_ACK,91,NULL);
}

static void preparation_file_reply(Peer *peer,int socket,unsigned command,unsigned id,
    const unsigned char digest[16])
{
    for(;;) {
        unsigned char response[1024];first_packet_byte(socket,response);
        size_t size=1;
        if(response[0]==PKT_FONT || response[0]==PKT_AUDIO) {
            size_t fixed=response[0]==PKT_FONT?2:4;
            unsigned strings=response[0]==PKT_FONT?2:4;
            assert(exact(socket,response+size,fixed));size+=fixed;
            for(unsigned i=0;i<strings;i++) {
                assert(field(socket,(char *)response+size,sizeof(response)-size));
                size+=strlen((const char *)response+size)+1;
            }
            wire_log(peer,"report",response,size);continue;
        }
        assert(response[0]==PKT_FILE && exact(socket,response+1,3));
        assert(response[1]==command && response[2]==(unsigned char)(id>>8) && response[3]==(unsigned char)id);
        size=4;
        if(command==PKT_FILE_SUM) {
            assert(exact(socket,response+4,16) && !memcmp(response+4,digest,16));size=20;
        }
        wire_log(peer,"postEND-reply",response,size);return;
    }
}

/* Legal modern server startup frames. These remain un-applied until their
 * owning gameplay modules adopt them; expected bytes are the actual fixture. */
static void startup_prefix(Peer *peer)
{
    unsigned char *at=peer->deferred,*begin=at;
    *at++=PKT_PARTY;*at++=0;*at++=0;*at++=0;
    peer->deferred_sizes[0]=(size_t)(at-begin);begin=at;
    *at++=PKT_GUILD;*at++=0;*at++=0;*at++=0;
    peer->deferred_sizes[1]=(size_t)(at-begin);begin=at;
    *at++=PKT_GUILD_CFG;
    for(unsigned i=0;i<7;i++)put32(&at,i==3?5:0);
    for(unsigned i=0;i<5;i++)*at++=0;
    peer->deferred_sizes[2]=(size_t)(at-begin);begin=at;
    *at++=PKT_WEATHERCOL;for(unsigned i=0;i<4;i++)*at++=0;
    peer->deferred_sizes[3]=(size_t)(at-begin);begin=at;
    *at++=PKT_PALETTE;*at++=1;*at++=32;*at++=64;*at++=96;
    peer->deferred_sizes[4]=(size_t)(at-begin);begin=at;
    *at++=PKT_POWERS_INFO;for(unsigned i=0;i<4;i++)put32(&at,0);
    peer->deferred_sizes[5]=(size_t)(at-begin);begin=at;
    *at++=PKT_SKILL_INIT;
    for(unsigned i=0;i<4;i++)put16(&at,0);
    put32(&at,0);*at++=0;
    memcpy(at,"Skill",6);at+=6;memset(at,'A',120);at+=120;*at++=0;*at++=0;
    peer->deferred_sizes[6]=(size_t)(at-begin);begin=at;
    *at++=PKT_SKILL_MOD;for(unsigned i=0;i<21;i++)*at++=0;
    peer->deferred_sizes[7]=(size_t)(at-begin);begin=at;
    *at++=PKT_SKILL_PTS;put32(&at,0);
    peer->deferred_sizes[8]=(size_t)(at-begin);begin=at;
    *at++=PKT_TECHNIQUE_INFO;put32(&at,0);put32(&at,0);
    peer->deferred_sizes[9]=(size_t)(at-begin);begin=at;
    *at++=PKT_BONI_COL;for(unsigned i=0;i<43;i++)*at++=0;
    peer->deferred_sizes[10]=(size_t)(at-begin);begin=at;
    /* Required equipment prefix is framed and retained, not applied as C021.
     * A legitimate item name longer than MAX_CHARS exercises the %I limit. */
    *at++=PKT_EQUIP;*at++='a';for(unsigned i=0;i<12;++i)*at++=0;
    memset(at,'E',100);at+=100;*at++=0;
    peer->deferred_sizes[11]=(size_t)(at-begin);begin=at;
    /* Additional actual pre-identity frames: retain exact protocol bytes for
     * their later owners, without applying audio/inventory/knowledge effects. */
    *at++=PKT_SFX_VOLUME;*at++=100;*at++=50;
    peer->deferred_sizes[12]=(size_t)(at-begin);begin=at;
    *at++=PKT_SFX_AMBIENT;put32(&at,0);
    peer->deferred_sizes[13]=(size_t)(at-begin);begin=at;
    *at++=PKT_MUSIC;for(unsigned i=0;i<12;++i)*at++=0;
    peer->deferred_sizes[14]=(size_t)(at-begin);begin=at;
    *at++=PKT_UNIQUE_MONSTER;for(unsigned i=0;i<8;++i)*at++=0;*at++='U';*at++=0;
    peer->deferred_sizes[15]=(size_t)(at-begin);begin=at;
    *at++=PKT_SOUND;for(unsigned i=0;i<28;++i)*at++=0;
    peer->deferred_sizes[16]=(size_t)(at-begin);begin=at;
    *at++=PKT_MUSIC_VOL;for(unsigned i=0;i<13;++i)*at++=0;
    peer->deferred_sizes[17]=(size_t)(at-begin);begin=at;
    *at++=PKT_INVEN_WIDE;*at++='a';for(unsigned i=0;i<29;++i)*at++=0;
    *at++='B';*at++=0;*at++=0;
    peer->deferred_sizes[18]=(size_t)(at-begin);begin=at;
    *at++=PKT_EQUIP_WIDE;*at++='a';for(unsigned i=0;i<12;++i)*at++=0;
    *at++='B';*at++=0;for(unsigned i=0;i<18;++i)*at++=0;
    peer->deferred_sizes[19]=(size_t)(at-begin);begin=at;
    *at++=PKT_LINE_INFO;put16(&at,0);
    unsigned width=peer->glyph_width?peer->glyph_width:1;
    for(unsigned i=width;i>0;i--)*at++=i==1?'.':0;
    *at++=255;
    if(peer->graphics==2) {
        for(unsigned i=width;i>0;i--)*at++=i==1?' ':0;
        *at++=1;
    }
    *at++=1;*at++=80;
    peer->deferred_sizes[20]=(size_t)(at-begin);
    peer->deferred_count=21;
    peer->deferred_size=(size_t)(at-peer->deferred);
}

static void peer_read_prepared_play(Peer *peer,int socket,unsigned attempt)
{
    unsigned char play[17000]; first_packet_byte(socket,play); assert(play[0]==PKT_PLAY);
        assert(exact(socket,play+1,14)); /* sex/race/class/trait/audio/graphics */
        char tiles[80],font[80]; assert(field(socket,tiles,sizeof(tiles)) && field(socket,font,sizeof(font)));
        size_t prefix=15;
        memcpy(play+prefix,tiles,strlen(tiles)+1);prefix+=strlen(tiles)+1;
        memcpy(play+prefix,font,strlen(font)+1);prefix+=strlen(font)+1;
        /* Six stats, 199 option slots, viewport, 3199 wide glyph entries. */
        size_t rest=12+199+8+(127+512+1280+1280)*5;
        assert(prefix+rest<=sizeof(play) && exact(socket,play+prefix,rest));
        wire_log(peer,"PLAY",play,prefix+rest);
        const unsigned char *options=play+prefix+12;
        peer->options_exact=options[0]==0 && options[2]==(unsigned char)(peer->mode==PEER_RETRY && attempt?1:0);
        if(peer->mode>=PEER_C011)assert(options[69]==(peer->mode==PEER_C011_CONTROLS) &&
            options[70]!=(peer->mode==PEER_C011_CONTROLS));
        assert(peer->options_exact);
        peer->graphics=(unsigned)play[13]<<8|play[14];
        if(c011_audio_mode(peer->case_name)) {
            bool enabled=peer->mode==PEER_C011_AUDIO_ENABLED;
            assert(play[9]==0 && play[10]==enabled && play[11]==0 && play[12]==enabled);
        }
        size_t visual_at=prefix+12+199+8;
        MD5_CTX visual_hash;MD5Init(&visual_hash);
        MD5Update(&visual_hash,play+visual_at,(unsigned)(prefix+rest-visual_at));
        MD5Final(peer->visual_digest,&visual_hash);
        uint32_t max_glyph=0;
        for(size_t i=prefix+12+199+8;i<prefix+rest;i+=5) {
            uint32_t glyph=(uint32_t)play[i+1]<<24|(uint32_t)play[i+2]<<16|
                (uint32_t)play[i+3]<<8|play[i+4];
            if(glyph>max_glyph)max_glyph=glyph;
        }
        for(;max_glyph;max_glyph>>=8)peer->glyph_width++;
}

static int peer_run(void *opaque)
{
    Peer *peer = opaque;
    int attempt = 0;
retry:
    ;
    if(peer->mode==PEER_C010) { struct pollfd wait={.fd=peer->listener,.events=POLLIN};
        assert(poll(&wait,1,8000)==1); }
    int socket = accept(peer->listener, NULL, NULL);
    assert(socket >= 0);
    struct timeval timeout = {.tv_sec = 8};
    assert(!setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));

    unsigned char number[40];
    char real[80], account[80], host[80], password[80];
    assert(exact(socket, number, 4) && field(socket, real, sizeof(real)) &&
           exact(socket, number + 4, 3) && field(socket, account, sizeof(account)) &&
           field(socket, host, sizeof(host)) && exact(socket, number + 7, 26));
    peer->contact_exact = !strcmp(real, "PLAYER") && !strcmp(account, "Test") &&
                          !strcmp(host, "localhost") && number[0] == 0 && number[1] == 0 &&
                          number[2] == 0x30 && number[3] == 0x39;

    unsigned char contact[34], *out = contact;
    *out++ = 255; *out++ = 0; put32(&out, 0); put32(&out, 2);
    const uint32_t version[6] = {4, 9, 4, 0, 0, 0};
    for (size_t i = 0; i < 6; ++i) put32(&out, version[i]);
    assert(send(socket, contact, sizeof(contact), 0) == (ssize_t)sizeof(contact));

    unsigned char verify_type;
    assert(exact(socket, &verify_type, 1) && field(socket, real, sizeof(real)) &&
           field(socket, account, sizeof(account)) && field(socket, password, sizeof(password)));
    peer->verify_exact = verify_type == 1 && !strcmp(real, "PLAYER") &&
                         !strcmp(account, "Test") && !strcmp(password, "Z]");
    static const unsigned char verified[] = {2, 1, 122, 6, 0, 0, 0x30, 0x39};
    assert(send(socket, verified, sizeof(verified), 0) == (ssize_t)sizeof(verified));

    static const unsigned char motd[] = "Managed peer MOTD";
    unsigned char setup[512]; out = setup;
    put32(&out, sizeof(motd) - 1); put16(&out, 20);
    *out++ = peer->mode>=PEER_C011; *out++ = peer->mode>=PEER_C011;
    *out++ = peer->mode>=PEER_C011; put32(&out, peer->mode>=PEER_C011?58:13);
    if(peer->mode>=PEER_C011) {
        memset(out,50,6);out+=6;memcpy(out,"Human",6);out+=6;put32(&out,1);
        memset(out,50,6);out+=6;memcpy(out,"Warrior",8);out+=8;memset(out,0,6);out+=6;
        memcpy(out,"None",5);out+=5;put32(&out,1);
    }
    memcpy(out, motd, sizeof(motd) - 1); out += sizeof(motd) - 1;
    size_t setup_size = (size_t)(out - setup);
    for (size_t at = 0; at < setup_size; at += 3) {
        size_t count = setup_size - at < 3 ? setup_size - at : 3;
        assert(send(socket, setup + at, count, 0) == (ssize_t)count);
        SDL_Delay(1);
    }

    unsigned char login_start[8];
    assert(exact(socket, login_start, sizeof(login_start)));
    peer->login_exact = login_start[0] == PKT_LOGIN && login_start[1] == 0 &&
                        login_start[2] == 0xf4 && login_start[3] == 0x43;


    if(peer->mode==PEER_C010) transfer_conversation(peer,socket);
    if(peer->mode==PEER_C011_REPAIR || peer->mode==PEER_C011_FILE_FAILURE)
        preparation_transfer(peer,socket,attempt);
    if(peer->mode==PEER_C011_RETRY)preparation_transfer(peer,socket,attempt);

    unsigned char conversation[256]; out = conversation;
    if(peer->mode!=PEER_C010 && peer->mode!=PEER_C011_REPAIR && peer->mode!=PEER_C011_FILE_FAILURE &&
       peer->mode!=PEER_C011_RETRY) {
        *out++ = PKT_SERVERDETAILS;
        put32(&out, 8); put32(&out, 0); put32(&out, 0); put32(&out, 0);
    }
    *out++ = PKT_KEEPALIVE;
    unsigned char *ping = out;
    if(peer->mode!=PEER_C010) {
        *out++ = PKT_PING; *out++ = 0;
        put32(&out, 7); put32(&out, 8); put32(&out, 9); *out++ = 0;
    }
    *out++ = PKT_LOGIN; put16(&out, 1);
    *out++ = 0xff; *out++ = 'W'; *out++ = 0;
    memcpy(out, "Hero", 5); out += 5;
    put16(&out, 42); put16(&out, 0); put16(&out, 0);
    memcpy(out, "at home", 8); out += 8;
    *out++ = PKT_LOGIN; put16(&out, 0); *out++ = 0; *out++ = 0;
    put16(&out, 0); put16(&out, 0); put16(&out, 0); *out++ = 0;
    if(peer->mode==PEER_C010) {
        ping=out;*out++=PKT_PING;*out++=0;put32(&out,7);put32(&out,8);put32(&out,9);*out++=0;
    }
    size_t conversation_size = (size_t)(out - conversation);
    assert(send(socket, conversation, conversation_size, 0) == (ssize_t)conversation_size);

    unsigned char echo[15];
    if(peer->mode==PEER_C010) { first_packet_byte(socket,echo); assert(exact(socket,echo+1,sizeof(echo)-1)); }
    else assert(exact(socket, echo, sizeof(echo)));
    peer->ping_exact = !memcmp(echo, ping, sizeof(echo)) && echo[1] == 1;
    /* pong is the only changed byte. */
    peer->ping_exact = peer->ping_exact ||
        (echo[0] == ping[0] && echo[1] == 1 && !memcmp(echo + 2, ping + 2, 13));

    if (peer->mode == PEER_QUIT || peer->mode==PEER_C010) {
        if(peer->mode!=PEER_C010) SDL_Delay(150);
        push_event(SDL_EVENT_WINDOW_RESIZED, 0);
        push_event(SDL_EVENT_KEY_DOWN, SDLK_Q);
        unsigned char byte;
        ssize_t remaining;
        do { remaining=recv(socket,&byte,1,0); assert(remaining<=0 || byte==PKT_KEEPALIVE); }
        while(remaining>0);
        peer->closed=remaining==0;
        close(socket);
        return 0;
    }

    unsigned char choice_type;
    char choice[80];
    assert(exact(socket, &choice_type, 1) && field(socket, choice, sizeof(choice)));
    peer->choice_exact = choice_type == PKT_LOGIN && !strcmp(choice,peer->mode==PEER_C011_BIRTH?"Born":"Hero");
    if (peer->mode == PEER_DISCONNECT) {
        close(socket);
        peer->closed = true;
        SDL_Delay(150);
        push_event(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE);
        return 0;
    }

    const unsigned char accepted = 0;
    assert(send(socket, &accepted, 1, 0) == 1);
    if(peer->mode==PEER_C011_RETRY && !attempt) {
        static const unsigned char rejected[]={PKT_QUIT,'P','r','e','p','a','r','a','t','i','o','n',' ','r','e','t','r','y',0};
        file_send(peer,socket,rejected,sizeof(rejected));
        SDL_Delay(200);push_event(SDL_EVENT_KEY_DOWN,SDLK_R);
        unsigned char byte;while(recv(socket,&byte,1,0)>0) {}
        close(socket);++attempt;goto retry;
    }
    if(peer->mode>=PEER_C011) {
        if(peer->mode!=PEER_C011_BIRTH) { SDL_Delay(60);push_event(SDL_EVENT_KEY_DOWN,SDLK_SPACE); }
        peer_read_prepared_play(peer,socket,attempt);
        unsigned char packet[1024],*at=packet;
        *at++=PKT_REPLY;*at++=PKT_PLAY;*at++=PKT_SUCCESS;*at++=PKT_END;
        if(peer->mode==PEER_C011_QUIT || peer->mode==PEER_C011_CANCEL) {
            if(peer->mode==PEER_C011_CANCEL) {
                file_send(peer,socket,packet,3); /* no END: preparation is cancelled */
                SDL_Delay(100);push_event(SDL_EVENT_QUIT,0);
            } else {
                *at++=PKT_QUIT;memcpy(at,"Preparation rejected",21);at+=21;
                size_t n=(size_t)(at-packet);
                file_send(peer,socket,packet,n-4);SDL_Delay(20);file_send(peer,socket,packet+n-4,4);
                SDL_Delay(120);push_event(SDL_EVENT_KEY_DOWN,SDLK_ESCAPE);
            }
            unsigned char byte;while(recv(socket,&byte,1,0)>0) {}
            peer->closed=true;close(socket);return 0;
        }
        if(peer->mode==PEER_C011) {
            /* PU_LUA invokes the same C010 owner after startup END. Demand a
             * real checksum response before sending the following frames. */
            *at++=PKT_FILE;*at++=PKT_FILE_CHECK;put16(&at,92);
            memcpy(at,"scpt/guide.lua",15);at+=15;
            file_send(peer,socket,packet,(size_t)(at-packet));at=packet;
            size_t guide_size;unsigned char *guide=read_guide(peer,&guide_size),digest[16];
            MD5_CTX md5;MD5Init(&md5);MD5Update(&md5,guide,(unsigned)guide_size);MD5Final(digest,&md5);
            free(guide);preparation_file_reply(peer,socket,PKT_FILE_SUM,92,digest);
            *at++=PKT_FILE;*at++=PKT_FILE_INIT;put16(&at,93);
            static const char filename[]="c011-startup.txt";
            memcpy(at,filename,sizeof(filename));at+=sizeof(filename);
            file_send(peer,socket,packet,(size_t)(at-packet));preparation_file_reply(peer,socket,PKT_FILE_ACK,93,NULL);
            static const unsigned char content[]="C011 post-END publication\n";
            at=packet;*at++=PKT_FILE;*at++=PKT_FILE_DATA;put16(&at,93);put16(&at,sizeof(content)-1);
            memcpy(at,content,sizeof(content)-1);at+=sizeof(content)-1;
            file_send(peer,socket,packet,(size_t)(at-packet));preparation_file_reply(peer,socket,PKT_FILE_ACK,93,NULL);
            at=packet;*at++=PKT_FILE;*at++=PKT_FILE_END;put16(&at,93);
            file_send(peer,socket,packet,(size_t)(at-packet));preparation_file_reply(peer,socket,PKT_FILE_ACK,93,NULL);
            char published[4096];assert(snprintf(published,sizeof(published),"%s/%s",peer->library,filename)<(int)sizeof(published));
            FILE *file=fopen(published,"rb");assert(file);
            unsigned char actual[sizeof(content)];
            assert(fread(actual,1,sizeof(actual),file)==sizeof(content)-1 && !memcmp(actual,content,sizeof(content)-1));
            assert(!fclose(file));at=packet;
        }
        if(peer->mode==PEER_C011_CONTROLS) {
            /* PAUSE is coalesced with startup END. The second occurrence is
             * nested, with an unanswered request and an interleaved update. */
            *at++=PKT_REQUEST_KEY;put32(&at,7);*at++='?';*at++=0;
            *at++=PKT_PAUSE;
            *at++=PKT_SFLAGS;put32(&at,3);put32(&at,0);put32(&at,0);put32(&at,0);
            *at++=PKT_PAUSE;
            for(unsigned i=0;i<12;++i)*at++=PKT_FLUSH;
            file_send(peer,socket,packet,(size_t)(at-packet));at=packet;
#ifdef SV_C011_TEST_WRAPPERS
            uint64_t deadline=SDL_GetTicks()+4000;
            while(SDL_GetAtomicInt(&controls_ack_count)<2 && SDL_GetTicks()<deadline)SDL_Delay(1);
            assert(SDL_GetAtomicInt(&controls_ack_count)==2);
#endif
        } else { *at++=PKT_FLUSH;*at++=PKT_FLUSH; }
        *at++=PKT_MESSAGE;memcpy(at,"C011 before identity",21);at+=21;
        if(peer->mode==PEER_C011) {
            startup_prefix(peer);
            memcpy(at,peer->deferred,peer->deferred_size);at+=peer->deferred_size;
            /* Split inside the final RLE run; CHAR_INFO follows only after
             * completing its true 80-column protocol frame. */
            size_t first=(size_t)(at-packet)-2;
            file_send(peer,socket,packet,first);SDL_Delay(30);
            memmove(packet,packet+first,2);at=packet+2;
        }
        *at++=PKT_CHAR_INFO;put16(&at,0);put16(&at,0);put16(&at,0);put16(&at,1);
        put32(&at,0x101);put16(&at,3);memcpy(at,"Accepted",9);at+=9;
        const unsigned char tail[]={PKT_INVEN,0xa1,0xb2,0xc3};
        memcpy(at,tail,sizeof(tail));at+=sizeof(tail);
        /* TCP split inside identity; publication must await the complete name. */
        size_t size=(size_t)(at-packet);file_send(peer,socket,packet,size-8);
        SDL_Delay(30);file_send(peer,socket,packet+size-8,8);
        if(c011_audio_mode(peer->case_name)) {
            while(!peer->audio_seen || !peer->font_seen) {
                unsigned char report[1024];first_packet_byte(socket,report);size_t n=1;
                assert(report[0]==PKT_AUDIO || report[0]==PKT_FONT);
                size_t fixed=report[0]==PKT_AUDIO?4:2;
                assert(exact(socket,report+1,fixed));n+=fixed;
                unsigned count=report[0]==PKT_AUDIO?4:2;
                for(unsigned i=0;i<count;++i) {
                    assert(field(socket,(char *)report+n,sizeof(report)-n));
                    if(report[0]==PKT_AUDIO)snprintf(peer->audio_metadata[i],80,"%s",report+n);
                    n+=strlen((char *)report+n)+1;
                }
                if(report[0]==PKT_AUDIO) {
                    peer->audio_seen=true;peer->audio_sfx=(unsigned)report[1]<<8|report[2];
                    peer->audio_music=(unsigned)report[3]<<8|report[4];
                } else peer->font_seen=true;
                wire_log(peer,"actual-report",report,n);
            }
        }
        SDL_Delay(250);push_event(SDL_EVENT_QUIT,0);
        unsigned char byte;
        while(recv(socket,&byte,1,0)>0) { /* font/audio replies precede owner destruction */ }
        peer->closed=true;close(socket);return 0;
    }
    if (peer->mode == PEER_RETRY && !attempt)
        write_setting(peer->root, "Hero.opt", "X:rogue_like_commands\nY:censor_swearing\n");
    if (peer->mode == PEER_RETRY && !attempt) {
        static const unsigned char rejected[] = {PKT_QUIT, 'W','r','o','n','g',' ','p','a','s','s','w','o','r','d',0};
        assert(send(socket, rejected, sizeof(rejected), 0) == (ssize_t)sizeof(rejected));
        close(socket);
        SDL_Delay(200);
        push_event(SDL_EVENT_KEY_DOWN, SDLK_R);
        ++attempt;
        goto retry;
    }
    SDL_Delay(60);push_event(SDL_EVENT_KEY_DOWN,SDLK_SPACE);
    peer_read_prepared_play(peer,socket,(unsigned)attempt);
    const unsigned char play_reply[]={PKT_REPLY,PKT_PLAY,PKT_SUCCESS,PKT_END};
    send_all(socket,play_reply,sizeof(play_reply));
    SDL_Delay(150);
    push_event(SDL_EVENT_WINDOW_RESIZED, 0);
    push_event(SDL_EVENT_WINDOW_FOCUS_LOST, 0);
    push_event(SDL_EVENT_WINDOW_FOCUS_GAINED, 0);
    push_event(SDL_EVENT_KEY_DOWN, SDLK_SPACE);
    SDL_Delay(150);
    push_event(SDL_EVENT_QUIT, 0);
    unsigned char byte;
    peer->closed = recv(socket, &byte, 1, 0) == 0;
    close(socket);
    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    if(!strcmp(argv[3],"c011-ins")) {
        startup_ins_checks(argv[1]); puts("SV-C011 startup INS production checks passed"); return 0;
    }
    char settings_root[4096];
    assert(snprintf(settings_root, sizeof(settings_root), "%s/sv", argv[1]) < (int)sizeof(settings_root));
    assert(!mkdir(settings_root, 0700) || errno == EEXIST);
    /* Existing credential gestures are scheduled after VIDEO init; suppress
     * unrelated optional pack/device work in this focused transport fixture. */
    write_setting(argv[1], "tomenet.cfg", "svSchemaVersion 1\nsound 0\n");
    c011_audio_fixture(argv[1],argv[2],argv[3]);
    write_setting(argv[1], "global.opt", "Y:rogue_like_commands\nY:censor_swearing\n");
    write_setting(argv[1], "Hero.opt", "X:rogue_like_commands\nX:censor_swearing\n");
    Peer peer = {.listener = socket(AF_INET, SOCK_STREAM, 0), .mode = PEER_SUCCESS, .root = argv[1], .library=argv[2], .case_name=argv[3]};
    assert(peer.listener >= 0);
    if (!strcmp(argv[3], "disconnect")) peer.mode = PEER_DISCONNECT;
    else if (!strcmp(argv[3], "quit")) peer.mode = PEER_QUIT;
    else if (!strcmp(argv[3], "retry")) peer.mode = PEER_RETRY;
    else if (!strcmp(argv[3], "c010-transfer")) peer.mode = PEER_C010;
    else if (!strcmp(argv[3], "c011-preparation")) peer.mode = PEER_C011;
    else if (!strcmp(argv[3], "c011-initial-repair")) peer.mode = PEER_C011_REPAIR;
    else if (!strcmp(argv[3], "c011-transfer-failure")) peer.mode = PEER_C011_FILE_FAILURE;
    else if (!strcmp(argv[3], "c011-startup-quit")) peer.mode = PEER_C011_QUIT;
    else if (!strcmp(argv[3], "c011-cancel")) peer.mode = PEER_C011_CANCEL;
    else if (!strcmp(argv[3], "c011-retry")) peer.mode = PEER_C011_RETRY;
    else if (!strcmp(argv[3], "c011-controls")) peer.mode = PEER_C011_CONTROLS;
    else if (!strcmp(argv[3], "c011-birth")) peer.mode = PEER_C011_BIRTH;
    else if (!strcmp(argv[3], "c011-audio-enabled")) peer.mode = PEER_C011_AUDIO_ENABLED;
    else if (!strcmp(argv[3], "c011-audio-missing-pack")) peer.mode = PEER_C011_AUDIO_MISSING;
    else if (!strcmp(argv[3], "c011-audio-no-device")) peer.mode = PEER_C011_AUDIO_NO_DEVICE;
    else assert(!strcmp(argv[3], "success"));
    if(peer.mode>=PEER_C011) {
        identity_checks();
        c011_prefix_boundary_checks();
        redraw_checks();
        committed_birth_check();
        startup_ins_checks(argv[1]);
        const char *shared=argv[1]; /* Explicit options.root is production U root. */
        char user[4096];assert(snprintf(user,sizeof(user),"%s/user",shared)<(int)sizeof(user));
        assert(!mkdir(user,0700) || errno==EEXIST);
        ins_fixture(shared,"Accepted.ins","Auto-Inscriptions file for TomeNET v4.9.4.0.0.0\nPreparation rule\nkept\n  F,a,b,-\n");
        write_setting(argv[1],"global.opt","Y:rogue_like_commands\nY:censor_swearing\n#:C011 global intent\n!:g\n!:h\n");
        write_setting(argv[1],"Hero.opt","X:rogue_like_commands\nX:censor_swearing\nX:thin_down_flush\nY:disable_flush\n#:C011 character intent\n!:c\n!:d\n");
        write_setting(argv[1],"chathist-Test.tmp","C011 first history\nC011 second history\n");
    }
    if(peer.mode==PEER_C011_CONTROLS) {
        write_setting(argv[1],"Hero.opt","X:rogue_like_commands\nX:censor_swearing\nY:thin_down_flush\nX:disable_flush\n#:C011 character intent\n!:c\n!:d\n");
#ifdef SV_C011_TEST_WRAPPERS
        controls_probe.enabled=true;
#endif
    }
#ifdef SV_C011_TEST_WRAPPERS
    if(peer.mode==PEER_C011_BIRTH)native_birth_driver.enabled=true;
#endif
    if(peer.mode==PEER_C011_BIRTH)
        write_setting(argv[1],"Born.opt","X:rogue_like_commands\nX:censor_swearing\nX:thin_down_flush\nY:disable_flush\n#:C011 character intent\n!:c\n!:d\n");
    int reuse = 1;
    assert(!setsockopt(peer.listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    assert(!bind(peer.listener, (struct sockaddr *)&address, sizeof(address)));
    assert(!listen(peer.listener, 1));
    socklen_t address_size = sizeof(address);
    assert(!getsockname(peer.listener, (struct sockaddr *)&address, &address_size));
    if(peer.mode==PEER_C010 || peer.mode>=PEER_C011) {
        const char *evidence=getenv(peer.mode>=PEER_C011?"SV_C011_NATIVE_EVIDENCE":"SV_C010_NATIVE_EVIDENCE");assert(evidence);
        char name[4096];assert(snprintf(name,sizeof(name),"%s/file-transcript.log",evidence)<(int)sizeof(name));
        peer.transcript=fopen(name,"w");assert(peer.transcript);
    }
    SDL_Thread *worker = SDL_CreateThread(peer_run, "sv-b020-peer", &peer);
    assert(worker);
    SDL_Thread *credentials = (peer.mode == PEER_RETRY || peer.mode==PEER_C010 || peer.mode>=PEER_C011) ? NULL :
        SDL_CreateThread(credential_run, "sv-b020-credentials", NULL);
    assert(peer.mode == PEER_RETRY || peer.mode==PEER_C010 || peer.mode>=PEER_C011 || credentials);

    SvEndpointOutcome outcome = {0};
    SvEndpointOptions options = {.root = argv[1], .library = argv[2],
        .server = "127.0.0.1", .width = 1024, .height = 768, .windowed = 1,
        .port = ntohs(address.sin_port),
        .real_name = "PLAYER", .character = (peer.mode == PEER_QUIT || peer.mode==PEER_C010 || peer.mode==PEER_C011_BIRTH) ? NULL : "hero",
        .outcome = &outcome};
    SvSessionPreparation *prepared=NULL;
    if(peer.mode>=PEER_C011) options.preparation=&prepared;
    if (peer.mode == PEER_RETRY || peer.mode==PEER_C010 || peer.mode>=PEER_C011) {
        options.account = "Test";
        options.password = "pw";
    }
    int result = sv_endpoint_run(options);
    if(peer.mode==PEER_C011_QUIT || peer.mode==PEER_C011_CANCEL) {
        assert(!prepared && result==(peer.mode==PEER_C011_QUIT?1:0));
        if(peer.mode==PEER_C011_QUIT)assert(strstr(outcome.reason,"Preparation rejected"));
    } else if(peer.mode>=PEER_C011) {
        assert(result==0 && prepared);
        SvPreparationView view=sv_session_preparation_view(prepared);
        fprintf(peer.transcript,"view startup=%d identity=%d profile=%d complete=%d unsupported=%u deferred=%zu/%zu ingress=%llu width=%u/%u graphics=%u/%u\n",
            view.startup_complete,view.identity_ready,view.profile_ready,view.complete,view.unsupported_packet,
            view.deferred_frames,view.deferred_bytes,(unsigned long long)view.ingress_sequence,
            view.prefix_config.char_transfer_bytes,peer.glyph_width,view.prefix_config.graphics,peer.graphics);
        fflush(peer.transcript);
        assert(view.startup_complete && view.identity_ready && view.profile_ready);
        assert(view.runtime_ready && view.transfer_failed==(peer.mode==PEER_C011_FILE_FAILURE));
        assert(view.complete==(peer.mode!=PEER_C011_FILE_FAILURE));
        if(peer.mode==PEER_C011) {
            assert(view.transfers.accepted_init==1 && view.transfers.completed_end==1 && view.transfers.reloads==1);
            assert(!view.transfers.active && !view.transfers.failures && view.transfers.completion.transfer_success);
            assert(view.transfers.completion.runtime.status==SV_LUA_OK);
        }
        if(peer.mode==PEER_C011_REPAIR) {
            assert(view.begin.status!=SV_LUA_OK && view.transfers.reloads==1 &&
                !view.transfers.failures && view.transfers.completion.runtime.status==SV_LUA_OK);
        }
        if(peer.mode==PEER_C011_FILE_FAILURE) {
            assert(view.transfers.failures==1 && view.transfers.reloads==1 &&
                !view.transfers.completion.transfer_success && view.transfers.completion.runtime.status==SV_LUA_OK);
        }
        assert(!strcmp(view.presentation.identity.name,"Accepted") && view.presentation.identity.lives==3);
        assert(view.presentation.flush_sequence==(peer.mode==PEER_C011_CONTROLS?12:2));
#ifdef SV_C011_TEST_WRAPPERS
        if(peer.mode==PEER_C011_CONTROLS) {
            fprintf(peer.transcript,"native controls presents=%u pauses=%u stale=%u ack=%u flush=%u paced=%u unpaced=%u\n",
                controls_probe.present_count,controls_probe.pause_presents,controls_probe.stale_inputs,
                controls_probe.fresh_acks,controls_probe.flushes,controls_probe.paced,controls_probe.unpaced);
            for(unsigned i=0;i<controls_probe.flushes;++i)fprintf(peer.transcript,
                "flush%u frame=%llu due=%llu presented=%llu deadline=%llu\n",i+1,
                (unsigned long long)controls_probe.flush[i].frame_ms,(unsigned long long)controls_probe.flush[i].due_ms,
                (unsigned long long)controls_probe.flush[i].present_ns,(unsigned long long)controls_probe.flush[i].deadline_ns);
            fflush(peer.transcript);
            assert(controls_probe.pause_presents==2 && controls_probe.stale_inputs==2 && controls_probe.fresh_acks==2 && controls_probe.text_stopped);
            assert(!view.presentation.paused && view.presentation.request.pending && view.presentation.request.id==7);
            assert(view.presentation.server_flags[0]==3);
            assert(controls_probe.flushes==12 && controls_probe.paced==10 && controls_probe.unpaced==2);
        }
#endif
        const SvStartupIns *ins=sv_session_preparation_inscriptions(prepared);
        assert(ins && ins->count==1 && !strcmp(ins->rules[0].match,"Preparation rule"));
        const SvPreferences *character=sv_session_preparation_preferences(prepared,false);
        const SvPreferences *global=sv_session_preparation_preferences(prepared,true);
        assert(character && global);
        bool global_message=false,character_message=false;
        for(unsigned source=0;source<2;source++) {
            const SvPreferences *prefs=source?character:global;
            for(size_t i=0;i<sv_preferences_effect_count(prefs);i++) {
                const SvPreferenceEffect *effect=sv_preferences_effect(prefs,i);
                if(effect->kind!=SV_PREF_MESSAGE)continue;
                const char *expected=source?"C011 character intent":"C011 global intent";
                if(effect->size!=strlen(expected) || memcmp(effect->bytes,expected,effect->size))continue;
                assert(sv_session_preparation_pending(prepared,!source,i));
                assert(sv_session_preparation_ack(prepared,!source,i)==SV_OK);
                assert(!sv_session_preparation_pending(prepared,!source,i));
                if(source)character_message=true;else global_message=true;
                for(size_t j=i+1;j<sv_preferences_effect_count(prefs);j++)
                    assert(sv_session_preparation_pending(prepared,!source,j));
            }
        }
        assert(global_message && character_message);
        size_t queue_size;
        const unsigned char *queue=sv_preferences_queue(global,&queue_size);
        const unsigned char global_queue[]={'h',29,'g',29},character_queue[]={'d',29,'c',29};
        assert(queue_size==sizeof(global_queue) && !memcmp(queue,global_queue,queue_size));
        queue=sv_preferences_queue(character,&queue_size);
        assert(queue_size==sizeof(character_queue) && !memcmp(queue,character_queue,queue_size));
        if(!c011_audio_mode(argv[3])) {
        assert(view.audio.status==SV_AUDIO_DISABLED && !view.audio.sfx && !view.audio.music);
        assert(view.audio.requested_rate==44100 && view.audio.requested_tracks==32 &&
            view.audio.requested_buffer==1024 &&
            !strcmp(view.audio.diagnostic,"Audio disabled by requested settings"));
        }
        const SvPreparationResources *resources=sv_session_preparation_resources(prepared);
        c011_audio_assert(argv[1],argv[2],argv[3],&view,resources);
        const SvMapFont *map=sv_session_preparation_map_font(prepared);
        SvGraphics *graphics=sv_session_preparation_graphics(prepared);
        assert(resources && map==&resources->map_font && graphics);
        char text_source[4096];
        assert(snprintf(text_source,sizeof(text_source),"%s/xtra/font/CascadiaMono-Regular.ttf",argv[2])<(int)sizeof(text_source));
        assert(!strcmp(resources->requested_text_font,"CascadiaMono-Regular.ttf") &&
            !strcmp(resources->effective_text_font,text_source));
        assert(!strcmp(resources->requested_map_font,"16x24x.pcf") && !strcmp(map->effective,"16x24x.pcf"));
        assert(!strcmp(resources->requested_tiles,"16x24sv") && !strcmp(resources->effective_tiles,"16x24sv"));
        assert(resources->requested_graphics==1 && resources->effective_graphics==1 &&
            sv_graphics_mode(graphics)==1 && !strcmp(sv_graphics_name(graphics),resources->effective_tiles));
        assert(!strcmp(map->user,argv[1]) && !strcmp(map->library,argv[2]) &&
            !strcmp(map->source,"font-sv.prf") && map->source_result==SV_FILE_MISSING);
        char source[4096];assert(snprintf(source,sizeof(source),"%s/user/font-sv.prf",argv[2])<(int)sizeof(source));
        assert(map->resolved.owner==SV_FILE_BUNDLED && !strcmp(map->resolved.path,source));
        if(!c011_audio_mode(argv[3])) {
        assert(!strcmp(resources->requested_sound_pack,"sound") && !strcmp(resources->requested_music_pack,"music") &&
            resources->requested_sound_subset==1 && resources->requested_music_subset==1);
        }
        unsigned char visuals[16384],digest[16];
        size_t visual_size=sv_visual_encode(&map->definitions,view.version,true,visuals);
        MD5_CTX visual_hash;MD5Init(&visual_hash);MD5Update(&visual_hash,visuals,(unsigned)visual_size);
        MD5Final(digest,&visual_hash);assert(!memcmp(digest,peer.visual_digest,sizeof(digest)));
        const SvInputHistory *history=sv_session_preparation_history(prepared);
        assert(history && sv_history_count(history)==2);
        assert(!strcmp(sv_history_entry(history,0),"C011 first history") &&
            !strcmp(sv_history_entry(history,1),"C011 second history"));
        assert(sv_session_preparation_lua(prepared));
        if(peer.mode==PEER_C011_RETRY) {
            assert(outcome.prior_generation && outcome.prior_generation!=outcome.generation);
            assert(view.transfers.accepted_init==1 && view.transfers.completed_end==1);
            assert(view.transfers.completion.guide_rebuilt && view.transfers.completion.guide_status==SV_LUA_OK);
            assert(sv_lua_reload_guide_required(sv_session_preparation_lua(prepared)));
        }
        unsigned char retained[1024];
        if(peer.mode==PEER_C011) {
            assert(view.deferred_frames==peer.deferred_count && view.deferred_bytes==peer.deferred_size);
            assert(sv_session_preparation_was_applied(prepared,1)); /* post-END CHECK */
            assert(sv_session_preparation_was_applied(prepared,7)); /* MESSAGE */
            assert(sv_session_preparation_was_applied(prepared,4)); /* post-END FILE_END */
            assert(!sv_session_preparation_was_applied(prepared,0));
            assert(view.prefix_config.char_transfer_bytes==peer.glyph_width &&
                view.prefix_config.graphics==peer.graphics && view.ingress_sequence==8+peer.deferred_count);
            assert(sv_session_preparation_take_input(prepared,retained,sizeof(retained)).result==SV_BUSY);
            size_t offset=0;
            for(size_t i=0;i<peer.deferred_count;i++) {
                SvPreparationFrame frame={0};
                SvOutput probe=sv_session_preparation_take_deferred(prepared,&frame,NULL,0);
                assert(probe.result==SV_OUTPUT_TOO_SMALL && probe.size==peer.deferred_sizes[i]);
                SvOutput item=sv_session_preparation_take_deferred(prepared,&frame,retained,sizeof(retained));
                assert(item.result==SV_OK && item.size==peer.deferred_sizes[i] && frame.size==item.size);
                assert(frame.sequence==i+8 && frame.packet==peer.deferred[offset]);
                assert(!sv_session_preparation_was_applied(prepared,frame.sequence));
                assert(!memcmp(retained,peer.deferred+offset,item.size));offset+=item.size;
            }
            SvPreparationFrame frame;
            assert(sv_session_preparation_take_deferred(prepared,&frame,retained,sizeof(retained)).result==SV_WAITING);
            assert(!sv_session_preparation_view(prepared).deferred_frames &&
                !sv_session_preparation_view(prepared).deferred_bytes);
            assert(!sv_session_preparation_was_applied(prepared,8)); /* drained != applied */
        }
        size_t stale_consumed=99;
        const unsigned char stale_file[]={PKT_FILE,PKT_FILE_END,0,92};
        assert(sv_session_preparation_receive_file(prepared,view.generation+1,
            stale_file,sizeof(stale_file),&stale_consumed)==SV_STALE && !stale_consumed);
        assert(sv_session_preparation_pump(prepared,view.generation+1)==SV_STALE);
        SvOutput input=sv_session_preparation_take_input(prepared,retained,sizeof(retained));
        const unsigned char tail[]={PKT_INVEN,0xa1,0xb2,0xc3};
        assert(input.result==SV_OK && input.size==sizeof(tail) && !memcmp(retained,tail,sizeof(tail)));
        assert(!sv_session_preparation_view(prepared).retained_input);
        assert(sv_session_preparation_take_input(prepared,retained,sizeof(retained)).result==SV_WAITING);
        SvApp *app=sv_session_preparation_app(prepared);
        uint64_t generation=sv_app_view(app).generation;
        if(peer.mode==PEER_C011_CONTROLS) {
            const unsigned char abort_request=PKT_REQUEST_ABORT;
            assert(sv_app_receive(app,generation,&abort_request,1)==SV_OK && sv_app_step(app,1).processed==1);
        }
        unsigned char movement='h';SvCommand command;
        assert(sv_app_physical(app,generation,&movement,1)==SV_OK);
        assert(sv_app_take_command(app,generation,&command)==SV_OK && command.command=='h');
        unsigned char control=18,request[4096];
        /* Drain previous reporting before observing one production Ctrl-R. */
        while(sv_app_take_output(app,generation,request,sizeof(request)).size) {}
        assert(sv_app_physical(app,generation,&control,1)==SV_OK);
        sv_app_dispatch_input(app,8);
        SvOutput redraw=sv_app_take_output(app,generation,request,sizeof(request));
        assert(redraw.result==SV_OK && redraw.size==2 && request[0]==PKT_REDRAW && request[1]==0);
        assert(sv_app_view(app).redraw_revision==1);
        fprintf(peer.transcript,"C011 prepared generation=%llu app=%llu identity=%s rules=%zu runtime=%d complete=%d redraw=%zu\n",
            (unsigned long long)view.generation,(unsigned long long)generation,
            view.presentation.identity.name,ins->count,view.runtime_ready,view.complete,redraw.size);
        if(c011_audio_mode(argv[3])) {
            bool enabled=peer.mode==PEER_C011_AUDIO_ENABLED;
            assert(peer.audio_seen && peer.font_seen && peer.audio_sfx==enabled && peer.audio_music==enabled);
            const char *names[]={"Sound chosen","2","Music chosen","3"};
            for(unsigned i=0;i<4;++i)assert(!strcmp(peer.audio_metadata[i],enabled?names[i]:""));
        }
#ifdef SV_C011_TEST_WRAPPERS
        if(peer.mode==PEER_C011_BIRTH) {
            assert(native_birth_driver.name_started && native_birth_driver.name_submitted &&
                native_birth_driver.mode_confirmed && native_birth_driver.prepared_visible);
            fprintf(peer.transcript,"native birth owners=%x stats=%u mode_ready=%d confirmed=%d active=%d closed=%d selected=%s\n",
                native_birth_driver.rendered_owners,native_birth_driver.stat_step,outcome.birth_mode_ready,
                outcome.birth_mode_confirmed,outcome.birth_active,outcome.birth_closed,outcome.selected_character);
            fflush(peer.transcript);
            assert(native_birth_driver.stat_step==6 && outcome.birth_mode_ready && outcome.birth_mode_confirmed);
            assert(!strcmp(outcome.selected_character,"Born"));
        }
#endif
        sv_session_preparation_destroy(prepared);
        if(c011_audio_mode(argv[3]))assert(!(SDL_WasInit(SDL_INIT_AUDIO)&SDL_INIT_AUDIO));
    }
    if (credentials) SDL_WaitThread(credentials, NULL);
    SDL_WaitThread(worker, NULL);
    close(peer.listener);
    assert(peer.contact_exact && peer.verify_exact && peer.login_exact && peer.ping_exact);
    if (peer.mode == PEER_SUCCESS || peer.mode == PEER_RETRY) {
        assert(result == 0 && peer.choice_exact && peer.options_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_LIVE_HANDOFF);
        assert(!strcmp(outcome.selected_character, "Hero"));
    } else if (peer.mode == PEER_DISCONNECT) {
        assert(result == 1 && peer.choice_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_DISCONNECTED);
    } else if(peer.mode<PEER_C011) {
        assert(result == 0 && !peer.choice_exact && peer.closed);
        assert(outcome.phase == SV_PREGAME_OVERVIEW);
    }
    if(peer.mode==PEER_C010) {
        SvTransferSummary summary=outcome.transfers;
        assert(outcome.lua_begin.status==SV_LUA_OK && summary.generation==outcome.generation);
        assert(summary.accepted_init==1 && summary.completed_end==1 && summary.reloads==1 && !summary.active);
        assert(!summary.failures && !summary.runtime_failures && !summary.guide_failures && summary.has_completion);
        assert(summary.completion.generation==summary.generation && summary.completion.transfer_success &&
            summary.completion.reload_attempted && summary.completion.runtime.status==SV_LUA_OK &&
            summary.completion.guide_rebuilt && summary.completion.guide_status==SV_LUA_OK);
        fprintf(peer.transcript,"outcome generation=%llu phase=%d begin=%d init=%zu end=%zu reloads=%zu active=%zu failures=%zu runtime_failures=%zu guide_failures=%zu guide_rebuilt=%d\n",
            (unsigned long long)outcome.generation,(int)outcome.phase,(int)outcome.lua_begin.status,
            summary.accepted_init,summary.completed_end,summary.reloads,summary.active,
            summary.failures,summary.runtime_failures,summary.guide_failures,summary.completion.guide_rebuilt);
        assert(!fclose(peer.transcript));
        puts("SV-C010 native socket/FILE/Lua outcome passed");
    } else if(peer.mode>=PEER_C011) {
        assert(peer.options_exact && peer.closed);assert(!fclose(peer.transcript));
        puts("SV-C011 native preparation/identity/INS/redraw checks passed");
    } else puts("SV-B-020 managed-peer production flow passed");
    return 0;
}
