/* One-off actual-server C011 probe. All client behavior is linked production;
 * this file supplies synthetic credentials, native SDL gestures and observation. */
#include "login/native-run.h"
#include "login/preparation.h"
#include "ui/native-controls.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static void push_text(const char *text)
{
    SDL_Event event={.type=SDL_EVENT_TEXT_INPUT};event.text.text=text;
    assert(SDL_PushEvent(&event));
}
#include "birth-wrapper.inc"

static bool prepared_seen, quit_sent;
static unsigned captured_owners;
static FILE *incoming,*outgoing;
static void wire(FILE **stream,const char *name,const void *bytes,size_t size)
{
    if(!size)return;
    if(!*stream) {
        char path[4096];assert(snprintf(path,sizeof(path),"%s/%s",getenv("SV_C011_CAPTURE_DIR"),name)<(int)sizeof(path));
        *stream=fopen(path,"wb");assert(*stream);
    }
    assert(fwrite(bytes,1,size,*stream)==size);fflush(*stream);
}
const SvContactSetup *__real_sv_contact_socket_setup(const SvContactSocket *);
const SvContactSetup *__wrap_sv_contact_socket_setup(const SvContactSocket *socket)
{
    const SvContactSetup *setup=__real_sv_contact_socket_setup(socket);
    static FILE *motd;
    if(setup && setup->motd_size && !motd)wire(&motd,"actual-motd.bin",setup->motd,setup->motd_size);
    return setup;
}
SvOutput __real_sv_contact_socket_write(SvContactSocket *,const void *,size_t);
SvOutput __wrap_sv_contact_socket_write(SvContactSocket *socket,const void *bytes,size_t size)
{
    SvOutput result=__real_sv_contact_socket_write(socket,bytes,size);
    if(result.result==SV_OK)wire(&outgoing,"post-contact-out.bin",bytes,result.size);
    return result;
}
SvOutput __real_sv_contact_socket_read(SvContactSocket *,void *,size_t);
SvOutput __wrap_sv_contact_socket_read(SvContactSocket *socket,void *bytes,size_t capacity)
{
    SvOutput result=__real_sv_contact_socket_read(socket,bytes,capacity);
    if(result.result==SV_OK)wire(&incoming,"post-contact-in.bin",bytes,result.size);
    return result;
}
bool __real_SDL_RenderPresent(SDL_Renderer *);
static void capture(SDL_Renderer *renderer,const char *name)
{
    const char *folder=getenv("SV_C011_CAPTURE_DIR");if(!folder)return;
    char path[4096];assert(snprintf(path,sizeof(path),"%s/%s.bmp",folder,name)<(int)sizeof(path));
    SDL_Surface *surface=SDL_RenderReadPixels(renderer,NULL);assert(surface);
    assert(SDL_SaveBMP(surface,path));
    fprintf(stderr,"capture %s %dx%d\n",path,surface->w,surface->h);SDL_DestroySurface(surface);
}
static void capture_birth(SDL_Renderer *renderer,SvBirthOwner owner)
{
    if(!(captured_owners&(1u<<owner)) &&
       (owner==SV_BIRTH_NONE || owner==SV_BIRTH_SEX || owner==SV_BIRTH_STATS || owner==SV_BIRTH_MODE || owner==SV_BIRTH_READY)) {
        capture(renderer,owner==SV_BIRTH_NONE?"motd":owner==SV_BIRTH_SEX?"birth-sex":owner==SV_BIRTH_STATS?"birth-stats":owner==SV_BIRTH_READY?"prepared":"birth-mode");
        captured_owners|=1u<<owner;
    }
}
bool __wrap_SDL_RenderPresent(SDL_Renderer *renderer)
{
    if(prepared_seen && native_birth_driver.handoff_rendered && !quit_sent) {
        quit_sent=true;
        SDL_Event event={.type=SDL_EVENT_QUIT};assert(SDL_PushEvent(&event));
    }
    return __real_SDL_RenderPresent(renderer);
}
SvStep __real_sv_native_controls_step(SvNativeControls *,SvApp *,size_t);
SvStep __wrap_sv_native_controls_step(SvNativeControls *controls,SvApp *app,size_t budget)
{
    SvStep step=__real_sv_native_controls_step(controls,app,budget);
    if(sv_app_view(app).identity.available)prepared_seen=true;
    return step;
}
int main(int argc,char **argv)
{
    assert(argc==3);
    native_birth_driver.enabled=true;
    native_birth_capture=capture_birth;
    SvEndpointOutcome outcome={0};SvSessionPreparation *prepared=NULL;
    SvEndpointOptions options={.root=argv[1],.library=argv[2],.server="127.0.0.1",.port=23648,
        .account="C011Probe",.password="Fixture",.real_name="PLAYER",.width=1024,.height=768,
        .windowed=1,.fps=100,.frames=450,.skip_motd=getenv("SV_REAL_SKIP_MOTD")?atoi(getenv("SV_REAL_SKIP_MOTD"))!=0:false,
        .outcome=&outcome,.preparation=&prepared};
    int result=sv_endpoint_run(options);
    fprintf(stderr,"endpoint result=%d phase=%d reason=%s birth=%d mode=%d stats=%u owners=%x prepared=%d\n",
        result,outcome.phase,outcome.reason,outcome.birth_entered,outcome.birth_mode_ready,
        native_birth_driver.stat_step,native_birth_driver.rendered_owners,prepared!=NULL);
    fprintf(stderr,"native MOTD acknowledged=%d handoff rendered=%d\n",
        native_birth_driver.motd_acknowledged,native_birth_driver.handoff_rendered);
    if(prepared) {
        SvPreparationView view=sv_session_preparation_view(prepared);
        fprintf(stderr,"preparation complete=%d startup=%d identity=%d runtime=%d unsupported=%u deferred=%zu reason=%s name=%s\n",
            view.complete,view.startup_complete,view.identity_ready,view.runtime_ready,
            view.unsupported_packet,view.deferred_frames,view.failure_reason,view.presentation.identity.name);
        unsigned char bytes[SV_PREPARATION_DEFERRED_BYTES];SvPreparationFrame frame;
        static FILE *deferred,*tail;
        SvOutput item;
        while((item=sv_session_preparation_take_deferred(prepared,&frame,bytes,sizeof(bytes))).result==SV_OK)
            wire(&deferred,"deferred.bin",bytes,item.size);
        item=sv_session_preparation_take_input(prepared,bytes,sizeof(bytes));
        if(item.result==SV_OK)wire(&tail,"retained-tail.bin",bytes,item.size);
        if(!view.identity_ready)for(unsigned i=0;i<8;i++) {
            item=sv_contact_socket_read(sv_session_preparation_connection(prepared),bytes,sizeof(bytes));
            if(item.result!=SV_OK)break;
            wire(&tail,"retained-tail.bin",bytes,item.size);
        }
        sv_session_preparation_destroy(prepared);
        return view.complete && view.identity_ready?0:2;
    }
    return 3;
}
