#define _XOPEN_SOURCE 700
#include "input/keymap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static SvCommand key(SvKeymap *map, unsigned char input, unsigned char command, unsigned char direction)
{
    SvCommand result;
    assert(sv_keymap_key(map,input,&result)==SV_OK);
    assert(result.command==command && result.direction==direction);
    return result;
}
static void write_file(const char *path, const char *text)
{
    FILE *file=fopen(path,"w");
    assert(file && fputs(text,file)>=0 && !fclose(file));
}
int main(void)
{
    SvKeymap map;
    SvCommand command;
    sv_keymap_init(&map,false);
    key(&map,'T','+',0); key(&map,',',',',5); key(&map,'.','.',0);
    for (unsigned i=1;i<=9;++i) key(&map,'0'+i,i==5?',':';',i);
    key(&map,10,13,0); key(&map,13,13,0); key(&map,'@','@',0);
    sv_keymap_init(&map,true);
    const unsigned char directions[]="bjnh.lyku";
    for (unsigned i=0;i<9;++i) {
        if (i==4) continue;
        key(&map,directions[i],';',i+1);
        key(&map,directions[i]-32,'.',i+1);
        key(&map,directions[i]&0x1f,'+',i+1);
    }
    /* Expected outcomes from baseline roguelike_commands; data, not a second
     * translator. Directional entries above also exercise the control keys. */
    static const unsigned char remaps[][2]={
        {6,21},{22,14},{3,'K'},{13,13},{4,'k'},{24,'U'},{23,'L'},
        {'P','b'},{1,'j'},{'#','S'},{'Z','u'},{'T','t'},{'t','f'},
        {'f','B'},{'x','l'},{'z','a'},{'a','z'},{'O','P'},{'S','x'},
        {5,'h'},{7,'H'},{26,7},{',','.'},{'@','@'}
    };
    for (size_t i=0;i<sizeof(remaps)/sizeof(remaps[0]);++i)
        key(&map,remaps[i][0],remaps[i][1],0);
    for (unsigned i=1;i<=9;++i) key(&map,'0'+i,i==5?',':';',i);
    key(&map,0xe8,';',4); /* Ordinary lookup masks the high bit. */
    key(&map,'.',',',5);
    assert(sv_keymap_key(&map,'\\',&command)==SV_WAITING);
    /* No input during an update leaves the unfinished bypass intact. */
    assert(sv_keymap_key(&map,0,&command)==SV_WAITING);
    key(&map,'h','h',5);
    assert(sv_keymap_key(&map,'^',&command)==SV_WAITING);
    key(&map,'H','+',4);
    assert(sv_keymap_key(&map,'\\',&command)==SV_WAITING);
    assert(sv_keymap_key(&map,'^',&command)==SV_WAITING);
    key(&map,'H',8,4);
    assert(sv_keymap_key(&map,'\\',&command)==SV_WAITING);
    key(&map,27,27,4); /* get_com cancel byte in bypass preserves direction. */
    assert(sv_keymap_key(&map,'^',&command)==SV_WAITING);
    key(&map,27,27,0); /* Control conversion retains Escape. */
    assert(sv_keymap_key(&map,'^',&command)==SV_WAITING);
    key(&map,'@',27,0); /* Zero mapped command becomes Escape. */
    key(&map,27,27,0); key(&map,13,13,0); key(&map,'-','-',0);
    assert(sv_keymap_key(&map,'^',&command)==SV_WAITING);
    sv_keymap_init(&map,true);
    key(&map,'h',';',4);

    char root[]="/tmp/sv-keymap-XXXXXX",user[256],file[256];
    assert(mkdtemp(root));
    snprintf(user,sizeof(user),"%s/user",root); assert(!mkdir(user,0700));
    snprintf(file,sizeof(file),"%s/user/test.prf",root);
    /* Three distinct S fields prove the declared first-field compatibility. */
    write_file(file,"S:104:59:4\nY:rogue_like_commands\nS:8:59:6\n");
    SvPreferences *preferences=sv_preferences_create(root,NULL,NULL);
    assert(preferences);
    SvPreferenceContext context=sv_preferences_default_context();
    SvPreferenceResult loaded=sv_preferences_load(preferences,SV_OPTION_SHARED,
        "test.prf",SV_PREF_EXPLICIT,&context);
    assert(loaded.status==SV_PREF_CLEAN);
    size_t cursor=0;
    sv_keymap_init(&map,false);
    sv_keymap_preferences(&map,preferences,&cursor);
    assert(cursor==4 && map.rogue);
    key(&map,'h','h',0); key(&map,8,8,8);
    sv_keymap_rebuild(&map); /* Manual load's post-load init erases S. */
    key(&map,'h',';',4); key(&map,8,'+',4);
    sv_keymap_preferences(&map,preferences,&cursor); /* No replay. */
    key(&map,'h',';',4);
    write_file(file,"X:rogue_like_commands\nS:104:59:4\nbogus\n");
    loaded=sv_preferences_load(preferences,SV_OPTION_SHARED,"test.prf",SV_PREF_EXPLICIT,&context);
    assert(loaded.status==SV_PREF_PARTIAL);
    sv_keymap_preferences(&map,preferences,&cursor);
    assert(!map.rogue);
    key(&map,'h','h',0);
    sv_keymap_rebuild(&map); key(&map,'T','+',0);
    SvPreferences *clone=sv_preferences_clone(preferences,false);
    assert(clone);
    size_t copied=0;
    sv_preferences_clear_effects(preferences);
    sv_keymap_init(&map,false); sv_keymap_preferences(&map,clone,&copied);
    assert(copied==cursor && !map.rogue); key(&map,8,8,8);
    sv_preferences_destroy(clone); sv_preferences_destroy(preferences);
    assert(!unlink(file) && !rmdir(user) && !rmdir(root));
    puts("SV keymap core passed");
    return 0;
}
