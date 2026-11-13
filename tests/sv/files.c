#define _POSIX_C_SOURCE 200809L
#include "files/files.h"
#include "files/history.h"
#include "files/dna.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void put(const SvFilePath *p,const char *s)
{ FILE *f=sv_file_open(p,"wb"); assert(f); assert(fwrite(s,1,strlen(s),f)==strlen(s)); assert(!fclose(f)); }
static void expect(const SvFilePath *p,const char *s)
{ char b[1024]={0}; FILE *f=sv_file_open(p,"rb"); assert(f); size_t n=fread(b,1,sizeof(b)-1,f); assert(!ferror(f)); assert(!fclose(f)); assert(n==strlen(s) && !memcmp(b,s,n)); }
int main(void)
{
    char u[]="/tmp/sv-c003-u-XXXXXX",b[]="/tmp/sv-c003-b-XXXXXX";
    assert(mkdtemp(u) && mkdtemp(b));
    SvFilePath shared,bundle,path;
    assert(sv_file_path(&shared,u,"xtra/font/test",SV_FILE_SHARED)); put(&shared,"overlay");
    assert(sv_file_path(&bundle,b,"xtra/font/test",SV_FILE_SHARED)); put(&bundle,"bundle");
    assert(sv_file_resource(&path,u,b,"xtra/font/test")==SV_FILE_OK && path.owner==SV_FILE_SHARED); expect(&path,"overlay");
    struct stat info; assert(!sv_file_stat(&path,&info));
    SvFilePath moved; assert(sv_file_path(&moved,u,"xtra/font/moved",SV_FILE_SHARED));
    assert(!sv_file_rename(&path,&moved) && !sv_file_remove(&moved));
    assert(sv_file_resource(&path,u,b,"xtra/font/test")==SV_FILE_OK && path.owner==SV_FILE_BUNDLED); expect(&path,"bundle");
    assert(!sv_file_open(&path,"wb") && sv_file_remove(&path)<0);
    assert(!sv_file_shared_user(&path,u,"../secret"));
    assert(sv_file_shared_user(&shared,u,"export.txt")); put(&shared,"old");
    SvFilePublication op;
    assert(sv_file_publish_begin(&op,&shared,false)==SV_FILE_REPLACE_REQUIRED);
    assert(sv_file_publish_write(&op,"new",3)==SV_FILE_OK);
    assert(sv_file_publish_commit(&op,false)==SV_FILE_REPLACE_REQUIRED); expect(&shared,"old");
    char temp[SV_FILE_PATH]; strcpy(temp,op.temporary); sv_file_publish_cancel(&op);
    assert(access(temp,F_OK)<0); expect(&shared,"old");
    assert(sv_file_publish_begin(&op,&shared,false)==SV_FILE_REPLACE_REQUIRED);
    assert(sv_file_publish_write(&op,"new",3)==SV_FILE_OK); put(&shared,"external change");
    assert(sv_file_publish_commit(&op,true)==SV_FILE_CONFLICT); expect(&shared,"external change");
    assert(sv_file_publish_begin(&op,&shared,false)==SV_FILE_REPLACE_REQUIRED);
    assert(sv_file_publish_write(&op,"new",3)==SV_FILE_OK && sv_file_publish_commit(&op,true)==SV_FILE_OK); expect(&shared,"new");
    assert(sv_file_publish_begin(&op,&shared,true)==SV_FILE_OK);
    assert(sv_file_publish_write(&op,"generated",9)==SV_FILE_OK && sv_file_publish_commit(&op,false)==SV_FILE_OK);
    assert(strcmp(op.target.path,shared.path)); expect(&op.target,"generated"); expect(&shared,"new");
    /* Real write-provider failure preserves destination, cleans owned temp. */
    assert(sv_file_publish_begin(&op,&shared,false)==SV_FILE_REPLACE_REQUIRED);
    strcpy(temp,op.temporary); assert(freopen("/dev/full","wb",op.stream));
    (void)sv_file_publish_write(&op,"cannot commit",13);
    if (op.active) assert(sv_file_publish_commit(&op,true)==SV_FILE_IO);
    assert(access(temp,F_OK)<0); expect(&shared,"new");
    /* Occupied generated path can be a directory or symlink, not just a file. */
    SvFilePath occupied; assert(sv_file_shared_user(&occupied,u,"occupied"));
    assert(!mkdir(occupied.path,0700));
    assert(sv_file_publish_begin(&op,&occupied,true)==SV_FILE_OK);
    assert(sv_file_publish_write(&op,"safe",4)==SV_FILE_OK && sv_file_publish_commit(&op,false)==SV_FILE_OK);
    assert(strcmp(op.target.path,occupied.path)); expect(&op.target,"safe");
    SvFilePath badparent; assert(sv_file_path(&badparent,u,"broken",SV_FILE_SHARED)); put(&badparent,"file");
    assert(sv_file_resource(&path,u,b,"broken/asset")==SV_FILE_INVALID);
    SvInputHistory *h=sv_history_create(); assert(h);
    assert(sv_history_begin(h,u,"account")==SV_FILE_MISSING && sv_history_save(h)==SV_FILE_OK);
    SvFilePath history; assert(sv_file_path(&history,u,"sv/chathist-account.tmp",SV_FILE_PROFILE)); assert(access(history.path,F_OK)<0);
    assert(sv_history_accept(h,SV_HISTORY_SECRET,"secret")==SV_FILE_INVALID);
    char text[64];
    for (unsigned i=0;i<2002;++i) { snprintf(text,sizeof(text),"public-%u",i); assert(sv_history_accept(h,SV_HISTORY_PUBLIC_CHAT,text)==SV_FILE_OK); }
    assert(sv_history_count(h)==2000 && !strcmp(sv_history_entry(h,0),"public-2"));
    assert(sv_history_accept(h,SV_HISTORY_PUBLIC_CHAT,"public-2")==SV_FILE_OK);
    assert(!strcmp(sv_history_entry(h,0),"public-3") && !strcmp(sv_history_entry(h,1999),"public-2"));
    assert(sv_history_save(h)==SV_FILE_OK && sv_history_begin(h,u,"account")==SV_FILE_OK);
    assert(sv_history_count(h)==2000 && !strcmp(sv_history_entry(h,1999),"public-2"));
    put(&history,"external\n"); assert(sv_history_save(h)==SV_FILE_CONFLICT); expect(&history,"external\n");
    assert(sv_history_begin(h,u,"other")==SV_FILE_MISSING && !sv_history_count(h));
    put(&history,"one\r\ntwo\n"); assert(sv_history_begin(h,u,"account")==SV_FILE_OK); assert(sv_history_count(h)==2 && !strcmp(sv_history_entry(h,0),"one"));
    FILE *f=sv_file_open(&history,"wb"); assert(f); assert(fwrite("bad\0line\n",1,9,f)==9); assert(!fclose(f));
    assert(sv_history_begin(h,u,"account")==SV_FILE_INVALID && sv_history_save(h)==SV_FILE_IO);
    sv_history_destroy(h);
    char filename[256]; assert(sv_dna_filename(filename,"Hero XIV") && !strcmp(filename,"Hero.dna"));
    assert(sv_dna_filename(filename,"Hero IIX") && !strcmp(filename,"Hero IIX.dna"));
    SvDna dna={.mode=0x304,.class_id=1,.race=2,.trait=0,.stats={0,1,2,3,4,5}},loaded;
    assert(sv_dna_save(&dna,u,"Hero II")==SV_FILE_OK);
    assert(sv_dna_load(&loaded,u,b,"Hero III",NULL)==SV_FILE_OK && !memcmp(&dna,&loaded,sizeof(dna)));
    assert(sv_dna_load(&loaded,u,b,"Newname","Hero IV")==SV_FILE_OK);
    assert(sv_file_shared_user(&shared,u,"Old.dna"));
    put(&shared,"Character birth file for TomeNET v4.7.1a\n33\n1\n2\n0\n0\n1\n2\n3\n4\n5\n");
    assert(sv_dna_load(&loaded,u,b,"Old",NULL)==SV_FILE_OK && loaded.mode==0x300);
    assert(sv_dna_load(&dna,u,b,"Old",NULL)==SV_FILE_OK && dna.mode==loaded.mode);
    put(&shared,"Character birth file for TomeNET v4.9.4\n0\n9999\n");
    assert(sv_dna_load(&loaded,u,b,"Old",NULL)==SV_FILE_INVALID);
    assert(sv_dna_load(&loaded,u,b,"Missing",NULL)==SV_FILE_MISSING);
    puts("SV C003 file owners/publication/history/DNA core: PASS");
    /* mkdtemp roots are intentionally observable for this core invocation. */
    return 0;
}
