#define _POSIX_C_SOURCE 200809L
#include "transfer/transfer.h"
#include "files/files.h"
#include "../../common/pack.h"
#include "../../common/md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

enum { NAME_CAPACITY=80, TRANSFER_LIMIT=128 };
typedef enum { RECEIVING, CHECKING, SENDING } TransferKind;
typedef struct TransferFile {
    struct TransferFile *next;
    FILE *stream;
    uint16_t id;
    TransferKind kind;
    bool ready, done;
    char name[NAME_CAPACITY];
} TransferFile;
struct SvTransfer {
    char library[SV_FILE_PATH];
    uint64_t generation;
    bool active, modern, no_updates;
    uint16_t next_id;
    SvLuaReload *lua;
    SvTransferDiagnostic diagnostic;
    void *diagnostic_context;
    TransferFile *files;
    unsigned char output[SV_TRANSFER_OUTPUT_CAPACITY];
    size_t output_size;
    SvTransferSummary summary;
};
static unsigned get16(const unsigned char *p)
{ return ((unsigned)p[0]<<8)|p[1]; }
static uint32_t get32(const unsigned char *p)
{ return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }
static void put16(unsigned char *p,unsigned n)
{ p[0]=(unsigned char)(n>>8);p[1]=(unsigned char)n; }
static void put32(unsigned char *p,uint32_t n)
{ p[0]=(unsigned char)(n>>24);p[1]=(unsigned char)(n>>16);p[2]=(unsigned char)(n>>8);p[3]=(unsigned char)n; }
static bool newer(const int v[6])
{
    static const int gate[6]={4,6,1,1,0,1};
    for(unsigned i=0;i<6;++i) if(v[i]!=gate[i]) return v[i]>gate[i];
    return false;
}
static bool current(const SvTransfer *owner,uint64_t generation)
{ return owner && owner->active && generation && owner->generation==generation; }
static bool room(const SvTransfer *owner,size_t count)
{ return count<=sizeof(owner->output)-owner->output_size; }
static void header(SvTransfer *owner,unsigned command,unsigned id)
{
    unsigned char *p=owner->output+owner->output_size;
    p[0]=PKT_FILE;p[1]=(unsigned char)command;put16(p+2,id);owner->output_size+=4;
}
static TransferFile *find(SvTransfer *owner,unsigned id)
{
    for(TransferFile *file=owner->files;file;file=file->next)
        if(file->id==id) return file;
    return NULL;
}
static size_t count(const SvTransfer *owner)
{
    size_t total=0;
    for(const TransferFile *file=owner->files;file;file=file->next) ++total;
    return total;
}
static void remove_file(SvTransfer *owner,TransferFile *file)
{
    TransferFile **at=&owner->files;
    while(*at && *at!=file) at=&(*at)->next;
    if(!*at) return;
    *at=file->next;
    if(file->stream) fclose(file->stream);
    free(file);
    owner->summary.active=count(owner);
}
static TransferFile *allocate(SvTransfer *owner,unsigned id,const char *name,TransferKind kind)
{
    if(count(owner)>=TRANSFER_LIMIT) return NULL;
    TransferFile *file=calloc(1,sizeof(*file));
    if(!file) return NULL;
    file->id=(uint16_t)id;file->kind=kind;
    snprintf(file->name,sizeof(file->name),"%s",name);
    file->next=owner->files;owner->files=file;
    owner->summary.active=count(owner);
    return file;
}
/* Linux baseline maps all operations to ANGBAND_DIR + filename. Deliberately
 * does not call generic relative-path validation: CHECK/source and INIT differ. */
static bool path(const SvTransfer *owner,const char *name,SvFilePath *out)
{
    int result=snprintf(out->path,sizeof(out->path),"%s%s%s",owner->library,
        owner->library[strlen(owner->library)-1]=='/'?"":"/",name);
    out->owner=SV_FILE_BUNDLED;
    return result>=0 && (size_t)result<sizeof(out->path);
}
/* The historical algorithm updates s2 once per 4096-byte read block. */
static uint32_t old_update(uint32_t total,const unsigned char *bytes,size_t length)
{
    uint32_t s1=total&65535u,s2=total>>16;
    for(size_t i=0;i<length;++i) s1+=bytes[i];
    if(length) { s2=(s1+s1)%65521u;s1%=65521u; }
    return (s2<<16)|s1;
}
static bool checksum(const SvTransfer *owner,const char *name,unsigned char digest[16],uint32_t *old)
{
    SvFilePath target;
    bool resolved=path(owner,name,&target);
    if(!resolved) errno=ENAMETOOLONG;
    FILE *stream=resolved?sv_file_open(&target,"rb"):NULL;
    memset(digest,0,16);*old=0;
    /* Missing is a normal checksum sentinel requesting an update, not a
     * failed startup. Other open failures retain the same zero wire SUM. */
    if(!stream) return errno==ENOENT;
    MD5_CTX context;
    MD5Init(&context);
    *old=1;
    unsigned char block[4096];
    size_t length;
    while((length=fread(block,1,sizeof(block),stream))>0) {
        *old=old_update(*old,block,length);
        MD5Update(&context,block,(unsigned)length);
    }
    MD5Final(digest,&context);
    bool success=!ferror(stream);
    fclose(stream);
    return success;
}
static void failure(SvTransfer *owner,const char *message)
{
    ++owner->summary.failures;
    if(owner->diagnostic) owner->diagnostic(owner->diagnostic_context,message);
}
void sv_transfer_set_diagnostic(SvTransfer *owner,SvTransferDiagnostic diagnostic,void *context)
{
    if(owner) { owner->diagnostic=diagnostic;owner->diagnostic_context=context; }
}

SvTransfer *sv_transfer_create(const char *library)
{
    if(!library || !*library || strlen(library)>=SV_FILE_PATH) return NULL;
    SvTransfer *owner=calloc(1,sizeof(*owner));
    if(owner) snprintf(owner->library,sizeof(owner->library),"%s",library);
    return owner;
}
void sv_transfer_close(SvTransfer *owner,uint64_t generation)
{
    if(!current(owner,generation)) return;
    while(owner->files) remove_file(owner,owner->files);
    owner->active=false;owner->lua=NULL;owner->output_size=0;
}
void sv_transfer_destroy(SvTransfer *owner)
{
    if(!owner) return;
    sv_transfer_close(owner,owner->generation);
    free(owner);
}
SvResult sv_transfer_begin(SvTransfer *owner,uint64_t generation,const int version[6],
                           SvLuaReload *lua,bool no_updates)
{
    if(!owner || !generation || !version) return SV_INVALID;
    sv_transfer_close(owner,owner->generation);
    owner->active=true;owner->generation=generation;owner->modern=newer(version);
    owner->lua=lua;owner->no_updates=no_updates;
    owner->summary=(SvTransferSummary){.generation=generation};
    return SV_OK;
}
size_t sv_transfer_output_size(const SvTransfer *owner)
{ return owner?owner->output_size:0; }
SvOutput sv_transfer_take_output(SvTransfer *owner,void *bytes,size_t capacity)
{
    if(!owner || (!bytes && capacity)) return (SvOutput){SV_INVALID,0};
    if(!owner->output_size) return (SvOutput){SV_WAITING,0};
    if(capacity<owner->output_size) return (SvOutput){SV_OUTPUT_TOO_SMALL,owner->output_size};
    if(!bytes) return (SvOutput){SV_INVALID,0};
    size_t size=owner->output_size;
    memcpy(bytes,owner->output,size);owner->output_size=0;
    return (SvOutput){SV_OK,size};
}
SvTransferSummary sv_transfer_summary(const SvTransfer *owner)
{ return owner?owner->summary:(SvTransferSummary){0}; }
static void end_lua(SvTransfer *owner,bool success)
{
    if(!owner->lua) return;
    SvLuaCompletion result;
    if(!sv_lua_reload_end(owner->lua,owner->generation,count(owner),success) ||
       !sv_lua_reload_take(owner->lua,&result)) { ++owner->summary.runtime_failures;return; }
    owner->summary.has_completion=true;owner->summary.completion=result;
    if(result.reload_attempted) {
        ++owner->summary.reloads;
        if(result.runtime.status!=SV_LUA_OK) ++owner->summary.runtime_failures;
    }
    if(result.guide_status!=SV_LUA_NOT_READY && result.guide_status!=SV_LUA_OK)
        ++owner->summary.guide_failures;
}
static bool publish(SvTransfer *owner,TransferFile *file)
{
    SvFilePath target;
    /* Server transfer publication has baseline wb semantics, unlike C003
     * immutable bundled reads and consent-based export publication. */
    FILE *destination=path(owner,file->name,&target)?fopen(target.path,"wb"):NULL;
    bool success=destination!=NULL;
    if(destination) {
        if(fseek(file->stream,0,SEEK_SET)) success=false;
        unsigned char block[4096];
        size_t length;
        while(success && (length=fread(block,1,sizeof(block),file->stream))>0)
            if(fwrite(block,1,length,destination)!=length) success=false;
        if(ferror(file->stream)) success=false;
        if(fclose(destination)) success=false;
    }
    return success;
}
SvResult sv_transfer_receive(SvTransfer *owner,uint64_t generation,
                             const void *bytes,size_t available,size_t *consumed)
{
    if(!consumed || (!bytes && available)) return SV_INVALID;
    *consumed=0;
    if(!current(owner,generation)) return SV_STALE;
    if(available<4) return SV_WAITING;
    const unsigned char *packet=bytes;
    if(packet[0]!=PKT_FILE) return SV_DECODE_ERROR;
    unsigned command=packet[1],id=get16(packet+2);
    if(!id) return SV_DECODE_ERROR;
    size_t length=4,payload=0;
    const char *name=NULL;
    if(command==PKT_FILE_INIT || command==PKT_FILE_CHECK) {
        size_t limit=available-4;
        if(limit>NAME_CAPACITY) limit=NAME_CAPACITY;
        const unsigned char *end=memchr(packet+4,0,limit);
        if(!end) return limit==NAME_CAPACITY?SV_DECODE_ERROR:SV_WAITING;
        length=(size_t)(end-packet)+1;name=(const char *)packet+4;
    } else if(command==PKT_FILE_DATA) {
        if(available<6) return SV_WAITING;
        payload=get16(packet+4);
        if(payload>1024) return SV_DECODE_ERROR;
        length=6+payload;
    } else if(command==PKT_FILE_SUM) length=owner->modern?20:8;
    if(available<length) return SV_WAITING;
    size_t reserve=0;
    switch(command) {
        case PKT_FILE_CHECK: reserve=owner->modern?20:8;break;
        case PKT_FILE_SUM: reserve=4+NAME_CAPACITY;break;
        case 0: case PKT_FILE_ACK: case PKT_FILE_ERR:break;
        default:reserve=4;break;
    }
    if(!room(owner,reserve)) return SV_BACKPRESSURE;
    TransferFile *file=find(owner,id);
    bool success=false;
    switch(command) {
    case PKT_FILE_CHECK: {
        unsigned char digest[16];uint32_t old;
        if(!checksum(owner,name,digest,&old)) failure(owner,"File CHECK: source missing, unavailable or read failed");
        header(owner,PKT_FILE_SUM,id);
        if(owner->modern) { memcpy(owner->output+owner->output_size,digest,16);owner->output_size+=16; }
        else { put32(owner->output+owner->output_size,old);owner->output_size+=4; }
        break;
    }
    case PKT_FILE_INIT:
        if(!owner->no_updates && name[0]!='/' && !strstr(name,"..")) {
            file=allocate(owner,id,name,RECEIVING);
            if(file) {
                file->stream=tmpfile();
                if(file->stream) success=true;
                else remove_file(owner,file);
            }
        }
        if(success) {
            ++owner->summary.accepted_init;
            if(owner->lua && !sv_lua_reload_file_init(owner->lua,generation,name,true))
                ++owner->summary.runtime_failures;
        } else failure(owner,"File INIT: updates disabled, invalid destination or temporary open failed");
        header(owner,success?PKT_FILE_ACK:PKT_FILE_ERR,id);
        break;
    case PKT_FILE_DATA:
        if(file && file->kind==RECEIVING && file->stream) {
            if(fwrite(packet+6,1,payload,file->stream)!=payload) {
                failure(owner,"File DATA: temporary write failed");remove_file(owner,file);
                /* Full DATA disk failure is consumed, diagnostic-only, no ACK. */
                break;
            }
            success=true;
        } else failure(owner,"File DATA: unknown or unavailable transfer");
        header(owner,success?PKT_FILE_ACK:PKT_FILE_ERR,id);
        break;
    case PKT_FILE_END:
        if(file && file->kind==RECEIVING && file->stream) {
            success=publish(owner,file);remove_file(owner,file);
        }
        if(!success) failure(owner,"File END: destination publication failed or transfer unavailable");
        ++owner->summary.completed_end;
        end_lua(owner,success);
        header(owner,success?PKT_FILE_ACK:PKT_FILE_ERR,id);
        break;
    case PKT_FILE_SUM:
        if(file && file->kind==CHECKING) {
            unsigned char digest[16];uint32_t old;
            bool checked=checksum(owner,file->name,digest,&old);
            if(!checked) failure(owner,"File upload: checksum source missing, unavailable or read failed");
            bool same=owner->modern?!memcmp(digest,packet+4,16):old==get32(packet+4);
            if(same) remove_file(owner,file);
            else {
                SvFilePath source;
                file->stream=path(owner,file->name,&source)?sv_file_open(&source,"rb"):NULL;
                if(!file->stream) { failure(owner,"File upload: source open failed");remove_file(owner,file); }
                else {
                    file->kind=SENDING;
                    header(owner,PKT_FILE_INIT,id);
                    size_t size=strlen(file->name)+1;
                    memcpy(owner->output+owner->output_size,file->name,size);owner->output_size+=size;
                }
            }
        }
        break;
    case PKT_FILE_ACK:
        if(file && file->kind==SENDING) {
            file->ready=true;
            if(file->done) remove_file(owner,file);
        }
        break;
    case PKT_FILE_ERR:
        if(file) { failure(owner,"File transfer: peer reported ERR");remove_file(owner,file); }
        break;
    case 0:break;
    default:
        failure(owner,"File transfer: unknown subcommand");header(owner,PKT_FILE_ERR,id);break;
    }
    *consumed=length;
    return SV_OK;
}
SvResult sv_transfer_upload(SvTransfer *owner,uint64_t generation,const char *filename)
{
    if(!current(owner,generation)) return SV_STALE;
    if(!filename) return SV_OK;
    size_t length=strlen(filename);
    if(length>30) return SV_INVALID;
    if(!room(owner,5+length)) return SV_BACKPRESSURE;
    uint16_t id=owner->next_id;
    unsigned attempts=0;
    do { if(!++id) ++id; } while(find(owner,id) && ++attempts<65535u);
    if(attempts==65535u) return SV_BUSY;
    TransferFile *file=allocate(owner,id,filename,CHECKING);
    if(!file) return SV_NO_MEMORY;
    owner->next_id=id;
    header(owner,PKT_FILE_CHECK,id);
    memcpy(owner->output+owner->output_size,filename,length+1);owner->output_size+=length+1;
    return SV_OK;
}
SvResult sv_transfer_pump(SvTransfer *owner,uint64_t generation)
{
    if(!current(owner,generation)) return SV_STALE;
    /* Each call has a fixed bound on file work and output. READY transitions
     * match baseline; an EOF pass retains READY for next tick's END. */
    for(TransferFile *file=owner->files;file;file=file->next) {
        if(file->kind!=SENDING || !file->ready) continue;
        size_t chunk=owner->modern?1024:256;
        if(!room(owner,file->done?4:6+chunk)) return SV_BACKPRESSURE;
        if(file->done) {
            header(owner,PKT_FILE_END,file->id);file->ready=false;
        } else {
            unsigned char buffer[1024];
            size_t size=fread(buffer,1,chunk,file->stream);
            if(!size) {
                if(ferror(file->stream)) failure(owner,"File upload: source read failed");
                file->done=true;
            } else {
                header(owner,PKT_FILE_DATA,file->id);
                put16(owner->output+owner->output_size,(unsigned)size);owner->output_size+=2;
                memcpy(owner->output+owner->output_size,buffer,size);owner->output_size+=size;
                file->ready=false;
            }
        }
    }
    return SV_OK;
}
