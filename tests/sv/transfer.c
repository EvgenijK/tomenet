#define _POSIX_C_SOURCE 200809L
#include "protocol/login.h"
#include "transfer/transfer.h"
#include "transfer/startup.h"
#include "lua/reload.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

static char root[4096];
static unsigned warnings, diagnostics;
static SvLoginTransfer attachment;
static void (*destroy_attachment)(SvLoginTransfer *);
static void login_end(SvLogin *login)
{
    sv_login_destroy(login);
    destroy_attachment(&attachment);
}
static void diagnostic(void *unused, const SvLuaResult *result)
{
    (void)unused;
    if (strstr(result->diagnostic,"restarting the game is recommended")) ++warnings;
    if (result->status!=SV_LUA_OK) ++diagnostics;
}
static void path(char out[4096],const char *name)
{
    int n=snprintf(out,4096,"%s/%s",root,name);
    assert(n>0 && n<4096);
}
static void write_file(const char *name,const void *bytes,size_t size)
{
    char namebuf[4096]; path(namebuf,name);
    FILE *f=fopen(namebuf,"wb"); assert(f);
    assert(fwrite(bytes,1,size,f)==size && !fclose(f));
}
static void expect_file(const char *name,const void *bytes,size_t size)
{
    unsigned char actual[2048]; char namebuf[4096]; path(namebuf,name);
    FILE *f=fopen(namebuf,"rb"); assert(f && size<sizeof(actual));
    assert(fread(actual,1,sizeof(actual),f)==size && !memcmp(bytes,actual,size));
    assert(!ferror(f) && !fclose(f));
}
static size_t header(unsigned char *out,unsigned command,unsigned id)
{
    out[0]=PKT_FILE; out[1]=(unsigned char)command;
    out[2]=(unsigned char)(id>>8); out[3]=(unsigned char)id; return 4;
}
static size_t filename(unsigned char *out,unsigned command,unsigned id,const char *name)
{
    size_t at=header(out,command,id), size=strlen(name)+1;
    memcpy(out+at,name,size); return at+size;
}
static size_t data(unsigned char *out,unsigned id,const void *bytes,size_t size)
{
    assert(size<=1024); header(out,PKT_FILE_DATA,id);
    out[4]=(unsigned char)(size>>8); out[5]=(unsigned char)size;
    memcpy(out+6,bytes,size); return size+6;
}
static void receive(SvLogin *login,const void *bytes,size_t size)
{
    SvResult result=sv_login_receive(login,bytes,size);
    assert(result==SV_OK);
}
static void expect_output(SvLogin *login,const void *expected,size_t size)
{
    unsigned char actual[65536];
    SvOutput output=sv_login_take_output(login,actual,sizeof(actual));
    assert(output.result==SV_OK && output.size==size && !memcmp(expected,actual,size));
    assert(sv_login_take_output(login,actual,sizeof(actual)).result==SV_WAITING);
}
static void ack(SvLogin *login,unsigned command,unsigned id)
{
    unsigned char packet[4]; header(packet,command,id); expect_output(login,packet,4);
}
static SvLogin *login_begin(const int version[6],uint64_t generation,
                            SvLuaReload *lua,bool no_updates)
{
    const unsigned char iaddr[6]={0}; unsigned char discard[65536];
    SvLogin *login=sv_login_create(version,iaddr); assert(login);
    assert(sv_login_take_output(login,discard,sizeof(discard)).result==SV_OK);
    SvTransferStartupConfig config={.library=root,.lua=lua,.no_updates=no_updates};
    SvLoginTransferFactory factory=sv_transfer_startup_factory(&config);
    assert(factory.create(factory.context,generation,version,&attachment));
    destroy_attachment=factory.destroy;
    assert(sv_login_attach_transfer(login,attachment)==SV_OK);
    /* Actual decoded server-details precede transfer input; no fabricated
     * features call bypasses the production startup attachment. */
    const unsigned char details[17]={PKT_SERVERDETAILS,0,0,0,0,0,0,0,0x84,
        0,0,0,0,0,0,0,0x1f};
    for (size_t i=0;i<sizeof(details)-1;++i) receive(login,details+i,1);
    assert(sv_login_lua_begin_result(login).status==SV_LUA_NOT_READY);
    receive(login,details+sizeof(details)-1,1);
    assert(sv_login_lua_begin_result(login).status==SV_LUA_OK);
    assert(sv_lua_ready(sv_lua_reload_runtime(lua)));
    const char *features[]={"RPG_SERVER","FUN_SERVER","PARTY_SERVER","ARCADE_SERVER","TEST_SERVER"};
    for (size_t i=0;i<sizeof(features)/sizeof(features[0]);++i) {
        long value=0;
        assert(sv_lua_number((SvLuaRuntime *)sv_lua_reload_runtime(lua),features[i],&value)==SV_LUA_OK);
        assert(value==1);
    }
    long temporary=0;
    assert(sv_lua_number((SvLuaRuntime *)sv_lua_reload_runtime(lua),"TEMP7",&temporary)==SV_LUA_OK);
    assert(temporary==128);
    return login;
}
static void checksum_cases(SvLuaReload *lua)
{
    static const int versions[][6]={{4,6,1,1,0,0},{4,6,1,1,0,1},{4,6,1,1,0,2}};
    static const unsigned char abc_md5[16]={0x90,0x01,0x50,0x98,0x3c,0xd2,0x4f,0xb0,
        0xd6,0x96,0x3f,0x7d,0x28,0xe1,0x7f,0x72};
    static const unsigned char empty_md5[16]={0xd4,0x1d,0x8c,0xd9,0x8f,0x00,0xb2,0x04,
        0xe9,0x80,0x09,0x98,0xec,0xf8,0x42,0x7e};
    write_file("abc","abc",3); write_file("empty","",0);
    for (unsigned v=0;v<3;++v) {
        SvLogin *login=login_begin(versions[v],10+v,lua,false);
        const char *names[]={"abc","empty","missing"};
        for (unsigned n=0;n<3;++n) {
            unsigned char request[256], expected[20]={0};
            size_t size=filename(request,PKT_FILE_CHECK,0x1234,names[n]);
            for (size_t i=0;i<size;++i) receive(login,request+i,1);
            header(expected,PKT_FILE_SUM,0x1234);
            if (v==2) {
                if (n<2) memcpy(expected+4,n ? empty_md5:abc_md5,16);
            } else if (n==0) {
                expected[4]=2; expected[5]=0x4e; expected[6]=1; expected[7]=0x27;
            } else if (n==1) expected[7]=1;
            expect_output(login,expected,v==2 ? 20:8);
        }
        /* CHECK has its own path rules: it must not inherit INIT's '..' filter. */
        write_file("name..txt","abc",3);
        unsigned char request[256], expected[20]={0};
        size_t size=filename(request,PKT_FILE_CHECK,1,"name..txt");
        receive(login,request,size); header(expected,PKT_FILE_SUM,1);
        if (v==2) memcpy(expected+4,abc_md5,16);
        else { expected[4]=2; expected[5]=0x4e; expected[6]=1; expected[7]=0x27; }
        expect_output(login,expected,v==2 ? 20:8);
        login_end(login);
    }
    puts("CHECK: exact old/MD5 bytes, boundary, missing/empty and operation paths PASS");
}
static void incoming_cases(SvLuaReload *lua)
{
    const int version[6]={4,9,4,0,0,0};
    SvLogin *login=login_begin(version,20,lua,false);
    unsigned char packet[4096], bytes[1024]; memset(bytes,0xa7,sizeof(bytes));
    size_t size=filename(packet,PKT_FILE_INIT,1,"payload");
    receive(login,packet,size-1);
    assert(!sv_login_transfer_summary(login).accepted_init);
    receive(login,packet+size-1,1); ack(login,PKT_FILE_ACK,1);
    size=data(packet,1,bytes,sizeof(bytes)); assert(size==1030);
    receive(login,packet,size-1);
    assert(!sv_login_transfer_summary(login).completed_end);
    receive(login,packet+size-1,1); ack(login,PKT_FILE_ACK,1);
    header(packet,PKT_FILE_END,1); receive(login,packet,4); ack(login,PKT_FILE_ACK,1);
    expect_file("payload",bytes,sizeof(bytes));
    SvTransferSummary summary=sv_login_transfer_summary(login);
    assert(summary.accepted_init==1 && summary.completed_end==1 && summary.reloads==1);
    assert(summary.has_completion && summary.completion.transfer_success &&
        summary.completion.runtime.status==SV_LUA_OK);
    receive(login,NULL,0); assert(sv_login_tick(login)==SV_OK);
    assert(sv_login_transfer_summary(login).reloads==1);

    /* Two chained INIT/END occurrences; only the last transfer reloads. */
    size=filename(packet,PKT_FILE_INIT,2,"audio.lua.note");
    size+=filename(packet+size,PKT_FILE_INIT,3,"guide.lua.note");
    receive(login,packet,size);
    const unsigned char init_acks[]={PKT_FILE,PKT_FILE_ACK,0,2,PKT_FILE,PKT_FILE_ACK,0,3};
    expect_output(login,init_acks,sizeof(init_acks));
    header(packet,PKT_FILE_END,2); header(packet+4,PKT_FILE_END,3);
    unsigned before=warnings; receive(login,packet,8);
    const unsigned char end_acks[]={PKT_FILE,PKT_FILE_ACK,0,2,PKT_FILE,PKT_FILE_ACK,0,3};
    expect_output(login,end_acks,sizeof(end_acks));
    summary=sv_login_transfer_summary(login);
    assert(summary.completed_end==3 && summary.reloads==2 && warnings==before+1);
    assert(summary.completion.audio_warning && summary.completion.guide_rebuilt);

    /* Failed publication still reloads current scripts, preserving transfer ERR. */
    char directory[4096]; path(directory,"directory"); assert(!mkdir(directory,0700));
    size=filename(packet,PKT_FILE_INIT,4,"directory"); receive(login,packet,size);
    ack(login,PKT_FILE_ACK,4); header(packet,PKT_FILE_END,4); receive(login,packet,4);
    ack(login,PKT_FILE_ERR,4); summary=sv_login_transfer_summary(login);
    assert(summary.failures && summary.completion.reload_attempted &&
        !summary.completion.transfer_success && summary.completion.runtime.status==SV_LUA_OK);
    /* A later successful END does not erase the aggregate transfer failure. */
    size=filename(packet,PKT_FILE_INIT,5,"after-failure"); receive(login,packet,size);
    ack(login,PKT_FILE_ACK,5); header(packet,PKT_FILE_END,5); receive(login,packet,4);
    ack(login,PKT_FILE_ACK,5); assert(sv_login_transfer_summary(login).failures==summary.failures);

    /* A successful file transfer can independently fail Lua execution or Guide
     * metadata evaluation. Both still ACK their actual file publication. */
    const char *names[]={"scpt/audio.lua","scpt/guide.lua"};
    const char *scripts[]={"not valid Lua !!!\n","guide_races='invalid count'\n"};
    for (unsigned i=0;i<2;++i) {
        unsigned id=10+i;
        size=filename(packet,PKT_FILE_INIT,id,names[i]); receive(login,packet,size);
        ack(login,PKT_FILE_ACK,id);
        size=data(packet,id,scripts[i],strlen(scripts[i])); receive(login,packet,size);
        ack(login,PKT_FILE_ACK,id); header(packet,PKT_FILE_END,id);
        before=warnings; receive(login,packet,4); ack(login,PKT_FILE_ACK,id);
        summary=sv_login_transfer_summary(login);
        assert(summary.completion.transfer_success && warnings==before+1);
        if (!i) assert(summary.runtime_failures && summary.completion.runtime.status!=SV_LUA_OK);
        else assert(summary.guide_failures && summary.completion.runtime.status==SV_LUA_OK &&
            summary.completion.guide_status!=SV_LUA_OK);
        char destination[4096]; path(destination,names[i]); assert(!unlink(destination));
    }
    assert(diagnostics);

    login_end(login);
    puts("incoming: split/chained 1030-byte frame, publication/Lua failures, sticky lifecycle PASS");
}
static void sticky_and_reject(const char *library)
{
    const int version[6]={4,9,4,0,0,0}; unsigned char packet[256];
    SvLuaReload *lua=sv_lua_reload_create(root,library,diagnostic,NULL); assert(lua);
    assert(sv_lua_reload_init(lua).status==SV_LUA_OK);
    SvLogin *login=login_begin(version,21,lua,false);
    size_t size=filename(packet,PKT_FILE_INIT,6,"audio.lua.pending");
    size+=filename(packet+size,PKT_FILE_INIT,7,"guide.lua.pending");
    receive(login,packet,size);
    const unsigned char accepted[]={PKT_FILE,PKT_FILE_ACK,0,6,PKT_FILE,PKT_FILE_ACK,0,7};
    expect_output(login,accepted,sizeof(accepted)); login_end(login);
    login=login_begin(version,22,lua,false);
    size=filename(packet,PKT_FILE_INIT,8,"new-session"); receive(login,packet,size);
    ack(login,PKT_FILE_ACK,8); header(packet,PKT_FILE_END,8);
    unsigned before=warnings; receive(login,packet,4); ack(login,PKT_FILE_ACK,8);
    SvTransferSummary summary=sv_login_transfer_summary(login);
    assert(!summary.failures && summary.reloads==1 && warnings==before+1 &&
        summary.completion.audio_warning && summary.completion.guide_rebuilt);
    login_end(login); sv_lua_reload_destroy(lua);

    /* A fresh application owner proves rejected INIT cannot set sticky flags. */
    lua=sv_lua_reload_create(root,library,diagnostic,NULL); assert(lua);
    assert(sv_lua_reload_init(lua).status==SV_LUA_OK);
    login=login_begin(version,23,lua,true);
    size=filename(packet,PKT_FILE_INIT,9,"ordinary-non-lua"); receive(login,packet,size);
    ack(login,PKT_FILE_ERR,9);
    size=filename(packet,PKT_FILE_INIT,10,"audio.lua.guide.lua"); receive(login,packet,size);
    ack(login,PKT_FILE_ERR,10); assert(!sv_login_transfer_summary(login).accepted_init);
    header(packet,PKT_FILE_END,10); before=warnings; receive(login,packet,4);
    ack(login,PKT_FILE_ERR,10); summary=sv_login_transfer_summary(login);
    assert(summary.completion.reload_attempted && !summary.completion.audio_warning &&
        !summary.completion.guide_rebuilt && warnings==before);
    login_end(login); sv_lua_reload_destroy(lua);
    puts("lifecycle: accepted INIT-disconnect sticky flags, -u rejects every INIT without flags PASS");
}
static void output_retry(SvLuaReload *lua)
{
    const int version[6]={4,9,4,0,0,0};
    SvLogin *login=login_begin(version,30,lua,false);
    /* Fill the actual login output queue, then deliver complete file events.
     * A retained reply may wait, but irreversible work must never replay. */
    size_t filled=0;
    while (sv_login_keepalive(login)==SV_OK) { ++filled; assert(filled<=65536); }
    unsigned char packet[128]; size_t size=filename(packet,PKT_FILE_INIT,9,"backpressure");
    SvResult result=sv_login_receive(login,packet,size);
    assert(result==SV_OK || result==SV_BACKPRESSURE);
    unsigned char output[65536];
    SvOutput out=sv_login_take_output(login,output,sizeof(output)); assert(out.result==SV_OK);
    assert(out.size>=filled);
    for (size_t i=0;i<filled;++i) assert(output[i]==PKT_KEEPALIVE);
    assert(sv_login_tick(login)==SV_OK);
    if (out.size==filled) ack(login,PKT_FILE_ACK,9);
    else { const unsigned char expected[]={PKT_FILE,PKT_FILE_ACK,0,9};
        assert(out.size==filled+4 && !memcmp(output+filled,expected,4)); }
    assert(sv_login_transfer_summary(login).accepted_init==1);
    header(packet,PKT_FILE_END,9); receive(login,packet,4); ack(login,PKT_FILE_ACK,9);
    assert(sv_login_tick(login)==SV_OK && sv_login_transfer_summary(login).reloads==1);
    login_end(login);
    puts("output: production buffer drain/resume without repeat file/Lua effects PASS");
}
static void upload_cases(SvLuaReload *lua)
{
    const int versions[][6]={{4,6,1,1,0,1},{4,6,1,1,0,2}};
    unsigned char bytes[1024]; for (size_t i=0;i<sizeof(bytes);++i) bytes[i]=(unsigned char)i;
    for (unsigned v=0;v<2;++v) {
        SvLogin *login=login_begin(versions[v],40+v,lua,false);
        unsigned char request[2048], expected[2048], output[65536];
        assert(sv_login_upload(login,NULL)==SV_OK);
        assert(sv_login_take_output(login,output,sizeof(output)).result==SV_WAITING);
        assert(sv_login_upload(login,"1234567890123456789012345678901")==SV_INVALID);
        size_t chunk=v ? 1024:256; write_file("upload",bytes,chunk);
        assert(sv_login_upload(login,"upload")==SV_OK);
        assert(sv_login_tick(login)==SV_OK);
        SvOutput out=sv_login_take_output(login,output,sizeof(output));
        assert(out.result==SV_OK && out.size==11 && output[0]==PKT_FILE &&
            output[1]==PKT_FILE_CHECK && !strcmp((char *)output+4,"upload"));
        unsigned id=((unsigned)output[2]<<8)|output[3]; assert(id);
        header(request,PKT_FILE_SUM,id); memset(request+4,0,v ? 16:4);
        receive(login,request,v ? 20:8);
        size_t size=filename(expected,PKT_FILE_INIT,id,"upload"); expect_output(login,expected,size);
        assert(sv_login_tick(login)==SV_OK);
        assert(sv_login_take_output(login,output,sizeof(output)).result==SV_WAITING);
        header(request,PKT_FILE_ACK,id); receive(login,request,4);
        assert(sv_login_tick(login)==SV_OK);
        size=data(expected,id,bytes,chunk); expect_output(login,expected,size);
        receive(login,request,4);
        /* EOF and END progress are tick-driven: no fabricated DATA ACK. */
        assert(sv_login_tick(login)==SV_OK); assert(sv_login_tick(login)==SV_OK);
        header(expected,PKT_FILE_END,id); expect_output(login,expected,4);
        receive(login,request,4); assert(!sv_login_transfer_summary(login).active);

        assert(sv_login_upload(login,"empty")==SV_OK);
        assert(sv_login_tick(login)==SV_OK);
        out=sv_login_take_output(login,output,sizeof(output)); assert(out.result==SV_OK);
        id=((unsigned)output[2]<<8)|output[3]; header(request,PKT_FILE_SUM,id);
        memset(request+4,0,v ? 16:4); receive(login,request,v ? 20:8);
        size=filename(expected,PKT_FILE_INIT,id,"empty"); expect_output(login,expected,size);
        header(request,PKT_FILE_ACK,id); receive(login,request,4);
        assert(sv_login_tick(login)==SV_OK); assert(sv_login_tick(login)==SV_OK);
        header(expected,PKT_FILE_END,id); expect_output(login,expected,4);
        header(request,PKT_FILE_ERR,id); receive(login,request,4);
        assert(!sv_login_transfer_summary(login).active);

        /* Equal checksum completes the check-only transfer without INIT/DATA. */
        assert(sv_login_upload(login,"abc")==SV_OK && sv_login_tick(login)==SV_OK);
        out=sv_login_take_output(login,output,sizeof(output)); assert(out.result==SV_OK);
        id=((unsigned)output[2]<<8)|output[3]; header(request,PKT_FILE_SUM,id);
        static const unsigned char digest[16]={0x90,0x01,0x50,0x98,0x3c,0xd2,0x4f,0xb0,
            0xd6,0x96,0x3f,0x7d,0x28,0xe1,0x7f,0x72};
        if (v) memcpy(request+4,digest,16);
        else { request[4]=2;request[5]=0x4e;request[6]=1;request[7]=0x27; }
        receive(login,request,v ? 20:8);
        assert(!sv_login_transfer_summary(login).active);
        assert(sv_login_take_output(login,output,sizeof(output)).result==SV_WAITING);
        login_end(login);
    }
    puts("upload: 256/1024 boundary, cancel, ACK pacing, empty/exact EOF tick and ERR cleanup PASS");
}
static void upload_backpressure(void)
{
    const int version[6]={4,6,1,1,0,2};
    const uint64_t generation=60;
    unsigned char source[2048], packet[2048], expected[2048];
    unsigned char output[SV_TRANSFER_OUTPUT_CAPACITY];
    memset(source,0x35,1024); memset(source+1024,0xca,1024);
    write_file("upload-backpressure",source,sizeof(source));
    SvTransfer *transfer=sv_transfer_create(root); assert(transfer);
    assert(sv_transfer_begin(transfer,generation,version,NULL,false)==SV_OK);
    assert(sv_transfer_upload(transfer,generation,"upload-backpressure")==SV_OK);
    SvOutput out=sv_transfer_take_output(transfer,output,sizeof(output));
    assert(out.result==SV_OK && out.size==24 && output[0]==PKT_FILE &&
        output[1]==PKT_FILE_CHECK && !strcmp((char *)output+4,"upload-backpressure"));
    unsigned id=((unsigned)output[2]<<8)|output[3]; assert(id);
    header(packet,PKT_FILE_SUM,id); memset(packet+4,0,16);
    size_t consumed=0;
    assert(sv_transfer_receive(transfer,generation,packet,20,&consumed)==SV_OK && consumed==20);
    size_t size=filename(expected,PKT_FILE_INIT,id,"upload-backpressure");
    out=sv_transfer_take_output(transfer,output,sizeof(output));
    assert(out.result==SV_OK && out.size==size && !memcmp(output,expected,size));
    header(packet,PKT_FILE_ACK,id);
    assert(sv_transfer_receive(transfer,generation,packet,4,&consumed)==SV_OK && consumed==4);

    /* Fill this READY sender's own output queue, rather than login output,
     * until a complete modern DATA frame cannot be reserved before fread. */
    size=filename(packet,PKT_FILE_CHECK,0x7000,"sender-fill-missing");
    size_t replies=0;
    while (SV_TRANSFER_OUTPUT_CAPACITY-sv_transfer_output_size(transfer)>=SV_TRANSFER_FRAME_CAPACITY) {
        assert(sv_transfer_receive(transfer,generation,packet,size,&consumed)==SV_OK && consumed==size);
        ++replies; assert(replies<=SV_TRANSFER_OUTPUT_CAPACITY/20);
    }
    size_t filled=sv_transfer_output_size(transfer);
    assert(filled==replies*20 && SV_TRANSFER_OUTPUT_CAPACITY-filled<SV_TRANSFER_FRAME_CAPACITY);
    SvTransferSummary before=sv_transfer_summary(transfer);
    assert(before.generation==generation && before.active==1 && !before.failures);
    for (unsigned retry=0;retry<2;++retry) {
        assert(sv_transfer_pump(transfer,generation)==SV_BACKPRESSURE);
        SvTransferSummary after=sv_transfer_summary(transfer);
        assert(sv_transfer_output_size(transfer)==filled && after.generation==before.generation &&
            after.active==before.active && after.failures==before.failures &&
            after.runtime_failures==before.runtime_failures && after.guide_failures==before.guide_failures &&
            after.accepted_init==before.accepted_init && after.completed_end==before.completed_end &&
            after.reloads==before.reloads && after.has_completion==before.has_completion);
    }
    out=sv_transfer_take_output(transfer,output,filled-1);
    assert(out.result==SV_OUTPUT_TOO_SMALL && out.size==filled && sv_transfer_output_size(transfer)==filled);
    out=sv_transfer_take_output(transfer,output,sizeof(output));
    assert(out.result==SV_OK && out.size==filled && !sv_transfer_output_size(transfer));
    unsigned char sum[20]={0}; header(sum,PKT_FILE_SUM,0x7000);
    for (size_t i=0;i<replies;++i) assert(!memcmp(output+i*sizeof(sum),sum,sizeof(sum)));

    for (unsigned chunk=0;chunk<2;++chunk) {
        assert(sv_transfer_pump(transfer,generation)==SV_OK);
        size=data(expected,id,source+chunk*1024,1024);
        out=sv_transfer_take_output(transfer,output,sizeof(output));
        assert(out.result==SV_OK && out.size==SV_TRANSFER_FRAME_CAPACITY && out.size==size &&
            !memcmp(output,expected,size));
        assert(sv_transfer_pump(transfer,generation)==SV_OK && !sv_transfer_output_size(transfer));
        header(packet,PKT_FILE_ACK,id);
        assert(sv_transfer_receive(transfer,generation,packet,4,&consumed)==SV_OK && consumed==4);
    }
    /* The EOF pass retains READY; END follows on a tick without another ACK. */
    assert(sv_transfer_pump(transfer,generation)==SV_OK && !sv_transfer_output_size(transfer));
    assert(sv_transfer_pump(transfer,generation)==SV_OK);
    header(expected,PKT_FILE_END,id);
    out=sv_transfer_take_output(transfer,output,sizeof(output));
    assert(out.result==SV_OK && out.size==4 && !memcmp(output,expected,4));
    assert(sv_transfer_receive(transfer,generation,packet,4,&consumed)==SV_OK && consumed==4);
    SvTransferSummary final=sv_transfer_summary(transfer);
    assert(!final.active && !final.failures && !sv_transfer_output_size(transfer));
    printf("sender backpressure: SUM replies=%zu queued=%zu remaining=%zu BP retries=2; "
        "DATA=1030(0x35),1030(0xca) ACK-paced EOF/END active=0 failures=0 PASS\n",
        replies,filled,(size_t)SV_TRANSFER_OUTPUT_CAPACITY-filled);
    sv_transfer_destroy(transfer);
}
int main(int argc,char **argv)
{
    assert(argc==2);
    char temporary[]="/tmp/sv-transfer-core-XXXXXX";
    char *created=mkdtemp(temporary); assert(created); snprintf(root,sizeof(root),"%s",created);
    char scripts[4096]; path(scripts,"scpt"); assert(!mkdir(scripts,0700));
    SvLuaReload *lua=sv_lua_reload_create(root,argv[1],diagnostic,NULL); assert(lua);
    assert(sv_lua_reload_init(lua).status==SV_LUA_OK);
    checksum_cases(lua); incoming_cases(lua); sticky_and_reject(argv[1]);
    output_retry(lua); upload_cases(lua); upload_backpressure();
    /* Explicit generation APIs reject stale work before any filesystem effect. */
    const int version[6]={4,9,4,0,0,0};
    SvTransfer *transfer=sv_transfer_create(root); assert(transfer);
    assert(sv_transfer_begin(transfer,50,version,NULL,false)==SV_OK);
    unsigned char packet[128]; size_t size=filename(packet,PKT_FILE_INIT,10,"stale"), consumed=99;
    assert(sv_transfer_receive(transfer,49,packet,size,&consumed)==SV_STALE && !consumed);
    assert(sv_transfer_upload(transfer,49,"abc")==SV_STALE);
    assert(sv_transfer_pump(transfer,49)==SV_STALE);
    assert(!sv_transfer_summary(transfer).accepted_init && !sv_transfer_output_size(transfer));
    /* Complete invalid frames are distinguished from split input; an unknown
     * DATA ID consumes its entire body before the following CHECK. */
    header(packet,PKT_FILE_END,0); consumed=99;
    assert(sv_transfer_receive(transfer,50,packet,4,&consumed)==SV_DECODE_ERROR && !consumed);
    unsigned char frame[1030], output[65536], payload[1024]; memset(payload,0x32,sizeof(payload));
    size=data(frame,999,"xyz",3);
    assert(sv_transfer_receive(transfer,50,frame,size,&consumed)==SV_OK && consumed==size);
    SvOutput out=sv_transfer_take_output(transfer,output,sizeof(output));
    const unsigned char unknown_err[]={PKT_FILE,PKT_FILE_ERR,3,0xe7};
    assert(out.result==SV_OK && out.size==4 && !memcmp(output,unknown_err,4));
    header(frame,PKT_FILE_DATA,1); frame[4]=4; frame[5]=1;
    assert(sv_transfer_receive(transfer,50,frame,6,&consumed)==SV_DECODE_ERROR && !consumed);

    /* A real Linux temporary-file write failure, not an injected test backend.
     * Restore the process limits before assertions or subsequent work. */
    size=filename(frame,PKT_FILE_INIT,11,"disk-failure");
    assert(sv_transfer_receive(transfer,50,frame,size,&consumed)==SV_OK);
    assert(sv_transfer_take_output(transfer,output,sizeof(output)).result==SV_OK);
    struct rlimit prior_limit, zero_limit;
    assert(!getrlimit(RLIMIT_FSIZE,&prior_limit)); zero_limit=prior_limit; zero_limit.rlim_cur=0;
    void (*prior_signal)(int)=signal(SIGXFSZ,SIG_IGN); assert(prior_signal!=SIG_ERR);
    assert(!setrlimit(RLIMIT_FSIZE,&zero_limit));
    bool write_failed=false;
    size=data(frame,11,payload,sizeof(payload));
    for (unsigned i=0;i<16;++i) {
        SvResult result=sv_transfer_receive(transfer,50,frame,size,&consumed);
        out=sv_transfer_take_output(transfer,output,sizeof(output));
        if (result!=SV_OK || consumed!=size) break;
        if (out.result==SV_WAITING) { write_failed=true;break; }
    }
    int restored=setrlimit(RLIMIT_FSIZE,&prior_limit);
    void (*restored_signal)(int)=signal(SIGXFSZ,prior_signal);
    assert(!restored && restored_signal!=SIG_ERR && write_failed);
    assert(!sv_transfer_summary(transfer).active && sv_transfer_summary(transfer).failures);
    sv_transfer_destroy(transfer); sv_lua_reload_destroy(lua);
    printf("PASS: production C010 login/transfer/Lua core; warnings=%u diagnostics=%u root=%s\n",
        warnings,diagnostics,root);
    return 0;
}
