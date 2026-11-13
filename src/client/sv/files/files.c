#define _POSIX_C_SOURCE 200809L
#include "files/files.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool relative_name(const char *name)
{
    if (!name || !*name || *name=='/' || strchr(name,'\\')) return false;
    for (const char *p=name; *p;) {
        const char *end=strchr(p,'/');
        size_t size=end?(size_t)(end-p):strlen(p);
        if (!size || (size==1 && *p=='.') || (size==2 && p[0]=='.' && p[1]=='.')) return false;
        if (!end) break;
        p=end+1;
        if (!*p) return false;
    }
    return true;
}
bool sv_file_path(SvFilePath *out, const char *root, const char *relative, SvFileOwner owner)
{
    if (!out || !root || !*root || !relative_name(relative)) { errno=EINVAL; return false; }
    SvFilePath next={.owner=owner};
    int size=snprintf(next.path,sizeof(next.path),"%s%s%s",root,
        root[strlen(root)-1]=='/'?"":"/",relative);
    if (size<0 || (size_t)size>=sizeof(next.path)) { errno=ENAMETOOLONG; return false; }
    *out=next;
    return true;
}
bool sv_file_shared_user(SvFilePath *out, const char *root, const char *name)
{
    char relative[SV_FILE_PATH];
    if (!relative_name(name) || strchr(name,'/')) { errno=EINVAL; return false; }
    int n=snprintf(relative,sizeof(relative),"user/%s",name);
    return n>0 && (size_t)n<sizeof(relative) && sv_file_path(out,root,relative,SV_FILE_SHARED);
}
int sv_file_stat(const SvFilePath *path, struct stat *info)
{
    if (!path || !info) { errno=EINVAL; return -1; }
    return stat(path->path,info);
}
SvFileResult sv_file_resource(SvFilePath *out, const char *user, const char *bundle, const char *relative)
{
    struct stat info;
    if (!sv_file_path(out,user,relative,SV_FILE_SHARED)) return SV_FILE_INVALID;
    if (!sv_file_stat(out,&info)) return S_ISREG(info.st_mode)?SV_FILE_OK:SV_FILE_INVALID;
    if (errno==ENOTDIR) return SV_FILE_INVALID;
    if (errno!=ENOENT) return SV_FILE_IO;
    if (!sv_file_path(out,bundle,relative,SV_FILE_BUNDLED)) return SV_FILE_INVALID;
    if (!sv_file_stat(out,&info)) return S_ISREG(info.st_mode)?SV_FILE_OK:SV_FILE_INVALID;
    return errno==ENOENT?SV_FILE_MISSING:errno==ENOTDIR?SV_FILE_INVALID:SV_FILE_IO;
}
static bool parents(const char *name)
{
    char path[SV_FILE_PATH];
    if (strlen(name)>=sizeof(path)) { errno=ENAMETOOLONG; return false; }
    strcpy(path,name);
    char *last=strrchr(path,'/');
    if (!last) return true;
    *last=0;
    for (char *p=path+1;;++p) {
        if (*p && *p!='/') continue;
        char saved=*p; *p=0;
        struct stat info;
        if (mkdir(path,0700) && (errno!=EEXIST || stat(path,&info) || !S_ISDIR(info.st_mode))) return false;
        *p=saved;
        if (!saved) break;
    }
    return true;
}
FILE *sv_file_open(const SvFilePath *path, const char *mode)
{
    if (!path || !mode) { errno=EINVAL; return NULL; }
    bool write=strpbrk(mode,"wa+")!=NULL;
    if (write && path->owner==SV_FILE_BUNDLED) { errno=EROFS; return NULL; }
    if (write && !parents(path->path)) return NULL;
    return fopen(path->path,mode);
}
int sv_file_remove(const SvFilePath *path)
{
    if (!path || path->owner==SV_FILE_BUNDLED) { errno=EROFS; return -1; }
    return remove(path->path);
}
int sv_file_rename(const SvFilePath *from, const SvFilePath *to)
{
    if (!from || !to || from->owner!=to->owner || to->owner==SV_FILE_BUNDLED) { errno=EROFS; return -1; }
    return rename(from->path,to->path);
}
void sv_file_publish_cancel(SvFilePublication *op)
{
    if (!op || !op->active) return;
    if (op->stream) fclose(op->stream);
    if (*op->temporary) unlink(op->temporary);
    op->stream=NULL; op->active=false; op->temporary[0]=0;
}
SvFileResult sv_file_publish_begin(SvFilePublication *op, const SvFilePath *target, bool generated)
{
    if (!op || !target || target->owner==SV_FILE_BUNDLED) return SV_FILE_INVALID;
    *op=(SvFilePublication){.target=*target,.generated=generated};
    if (!lstat(target->path,&op->observed)) {
        if (!generated && !S_ISREG(op->observed.st_mode)) return SV_FILE_INVALID;
        op->existed=true;
    } else if (errno!=ENOENT) return SV_FILE_IO;
    if (!parents(target->path)) return SV_FILE_IO;
    int n=snprintf(op->temporary,sizeof(op->temporary),"%s.sv-XXXXXX",target->path);
    if (n<0 || (size_t)n>=sizeof(op->temporary)) return SV_FILE_INVALID;
    int fd=mkstemp(op->temporary);
    if (fd<0) { op->temporary[0]=0; return SV_FILE_IO; }
    op->active=true;
    op->stream=fdopen(fd,"wb");
    if (!op->stream) { close(fd); sv_file_publish_cancel(op); return SV_FILE_IO; }
    return op->existed && !generated?SV_FILE_REPLACE_REQUIRED:SV_FILE_OK;
}
SvFileResult sv_file_publish_write(SvFilePublication *op, const void *bytes, size_t size)
{
    if (!op || !op->active || !op->stream || (!bytes && size)) return SV_FILE_INVALID;
    if (size && fwrite(bytes,1,size,op->stream)!=size) { sv_file_publish_cancel(op); return SV_FILE_IO; }
    return SV_FILE_OK;
}
static bool same(const struct stat *a, const struct stat *b)
{
    return a->st_dev==b->st_dev && a->st_ino==b->st_ino && a->st_size==b->st_size &&
        a->st_mtim.tv_sec==b->st_mtim.tv_sec && a->st_mtim.tv_nsec==b->st_mtim.tv_nsec &&
        a->st_ctim.tv_sec==b->st_ctim.tv_sec && a->st_ctim.tv_nsec==b->st_ctim.tv_nsec;
}
SvFileResult sv_file_publish_commit(SvFilePublication *op, bool replace)
{
    if (!op || !op->active || !op->stream) return SV_FILE_INVALID;
    if (op->existed && !op->generated && !replace) return SV_FILE_REPLACE_REQUIRED;
    if (fflush(op->stream) || ferror(op->stream) || fsync(fileno(op->stream))) {
        sv_file_publish_cancel(op); return SV_FILE_IO;
    }
    int closed=fclose(op->stream); op->stream=NULL;
    if (closed) { sv_file_publish_cancel(op); return SV_FILE_IO; }
    int result=-1;
    if (op->generated) {
        char base[SV_FILE_PATH]; strcpy(base,op->target.path);
        for (unsigned suffix=0;suffix<100000;++suffix) {
            int n=suffix?snprintf(op->target.path,sizeof(op->target.path),"%s.%u",base,suffix):
                snprintf(op->target.path,sizeof(op->target.path),"%s",base);
            if (n<0 || (size_t)n>=sizeof(op->target.path)) break;
            result=link(op->temporary,op->target.path);
            if (!result || errno!=EEXIST) break;
        }
    } else if (!op->existed) {
        result=link(op->temporary,op->target.path);
        if (result && errno==EEXIST) { sv_file_publish_cancel(op); return SV_FILE_CONFLICT; }
    } else {
        struct stat now;
        if (lstat(op->target.path,&now) || !same(&now,&op->observed)) {
            sv_file_publish_cancel(op); return SV_FILE_CONFLICT;
        }
        result=rename(op->temporary,op->target.path);
    }
    sv_file_publish_cancel(op);
    return result?SV_FILE_IO:SV_FILE_OK;
}
