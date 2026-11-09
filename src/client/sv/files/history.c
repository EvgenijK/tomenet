#define _POSIX_C_SOURCE 200809L
#include "files/history.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
struct SvInputHistory {
    SvFilePath path;
    size_t count;
    bool active, writable, existed;
    struct stat loaded;
    char entries[SV_HISTORY_CAPACITY][SV_HISTORY_BYTES];
};
SvInputHistory *sv_history_create(void) { return calloc(1,sizeof(SvInputHistory)); }
void sv_history_destroy(SvInputHistory *history) { free(history); }
size_t sv_history_count(const SvInputHistory *history) { return history?history->count:0; }
const char *sv_history_entry(const SvInputHistory *history, size_t n)
{ return history && n<history->count?history->entries[n]:NULL; }
SvFileResult sv_history_begin(SvInputHistory *h, const char *root, const char *account)
{
    char name[256];
    if (!h || !account || !*account || strchr(account,'/') || strchr(account,'\\') ||
        strchr(account,'\n') || strchr(account,'\r')) return SV_FILE_INVALID;
    int size=snprintf(name,sizeof(name),"sv/chathist-%s.tmp",account);
    if (size<0 || (size_t)size>=sizeof(name)) return SV_FILE_INVALID;
    memset(h,0,sizeof(*h));
    if (!sv_file_path(&h->path,root,name,SV_FILE_PROFILE)) return SV_FILE_INVALID;
    h->active=h->writable=true;
    FILE *file=sv_file_open(&h->path,"rb");
    if (!file) {
        if (errno==ENOENT) return SV_FILE_MISSING;
        h->writable=false; return SV_FILE_IO;
    }
    if (fstat(fileno(file),&h->loaded)) { fclose(file); h->writable=false; return SV_FILE_IO; }
    h->existed=true;
    SvFileResult result=SV_FILE_OK;
    while (h->count<SV_HISTORY_CAPACITY) {
        char line[SV_HISTORY_BYTES]; size_t used=0; int ch;
        bool invalid=false;
        while ((ch=fgetc(file))!=EOF && ch!='\n') {
            if (!ch || used==sizeof(line)-1) invalid=true;
            else if (!invalid) line[used++]=(char)ch;
        }
        if (ch==EOF && !used && !invalid) break;
        if (used && line[used-1]=='\r') --used;
        if (invalid) { result=SV_FILE_INVALID; break; }
        line[used]=0;
        memcpy(h->entries[h->count++],line,used+1);
        if (ch==EOF) break;
    }
    if (ferror(file)) result=SV_FILE_IO;
    if (fclose(file)) result=SV_FILE_IO;
    if (result!=SV_FILE_OK) h->writable=false; /* Do not replace an unread source. */
    return result;
}
SvFileResult sv_history_accept(SvInputHistory *h, SvHistoryInput kind, const char *input)
{
    if (!h || !h->active || kind!=SV_HISTORY_PUBLIC_CHAT || !input ||
        strlen(input)>=SV_HISTORY_BYTES || strchr(input,'\n') || strchr(input,'\r')) return SV_FILE_INVALID;
    size_t at=0;
    while (at<h->count && strcmp(h->entries[at],input)) ++at;
    if (at<h->count) {
        memmove(h->entries+at,h->entries+at+1,(h->count-at-1)*sizeof(h->entries[0])); --h->count;
    }
    if (h->count==SV_HISTORY_CAPACITY) {
        memmove(h->entries,h->entries+1,(h->count-1)*sizeof(h->entries[0])); --h->count;
    }
    strcpy(h->entries[h->count++],input);
    return SV_FILE_OK;
}
SvFileResult sv_history_save(SvInputHistory *h)
{
    if (!h || !h->active) return SV_FILE_INVALID;
    if (!h->writable) return SV_FILE_IO;
    if (!h->count) return SV_FILE_OK;
    struct stat now;
    bool exists=sv_file_stat(&h->path,&now)==0;
    if (exists!=h->existed || (exists &&
        (now.st_dev!=h->loaded.st_dev || now.st_ino!=h->loaded.st_ino || now.st_size!=h->loaded.st_size ||
         now.st_mtim.tv_sec!=h->loaded.st_mtim.tv_sec || now.st_mtim.tv_nsec!=h->loaded.st_mtim.tv_nsec ||
         now.st_ctim.tv_sec!=h->loaded.st_ctim.tv_sec || now.st_ctim.tv_nsec!=h->loaded.st_ctim.tv_nsec))) return SV_FILE_CONFLICT;
    if (!exists && errno!=ENOENT && errno!=ENOTDIR) return SV_FILE_IO;
    SvFilePublication op;
    SvFileResult result=sv_file_publish_begin(&op,&h->path,false);
    if (result!=SV_FILE_OK && result!=SV_FILE_REPLACE_REQUIRED) return result;
    if (op.existed!=h->existed || (op.existed &&
        (op.observed.st_dev!=h->loaded.st_dev || op.observed.st_ino!=h->loaded.st_ino ||
         op.observed.st_size!=h->loaded.st_size ||
         op.observed.st_mtim.tv_sec!=h->loaded.st_mtim.tv_sec || op.observed.st_mtim.tv_nsec!=h->loaded.st_mtim.tv_nsec ||
         op.observed.st_ctim.tv_sec!=h->loaded.st_ctim.tv_sec || op.observed.st_ctim.tv_nsec!=h->loaded.st_ctim.tv_nsec))) {
        sv_file_publish_cancel(&op); return SV_FILE_CONFLICT;
    }
    for (size_t i=0;i<h->count;++i) {
        if (!*h->entries[i]) continue;
        result=sv_file_publish_write(&op,h->entries[i],strlen(h->entries[i]));
        if (result==SV_FILE_OK) result=sv_file_publish_write(&op,"\n",1);
        if (result!=SV_FILE_OK) return result;
    }
    result=sv_file_publish_commit(&op,true); /* Baseline independent autosave. */
    if (result==SV_FILE_OK) h->existed=sv_file_stat(&h->path,&h->loaded)==0;
    return result;
}
