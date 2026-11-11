/* Offline inspection of a retained leading packet stream, using production
 * decoders only. Never searches for guessed packet bytes beyond a boundary. */
#include "protocol/startup-prefix.h"
#include "protocol/character-identity.h"
#include "app.h"
#include "../../src/common/pack.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
int main(int argc,char **argv)
{
    assert(argc==2);FILE *file=fopen(argv[1],"rb");assert(file);
    unsigned char bytes[512*1024];size_t size=fread(bytes,1,sizeof(bytes),file);assert(!ferror(file));fclose(file);
    SvStartupPrefixConfig config={.version={4,9,3,0,2,0},.graphics=1,.char_transfer_bytes=2};
    SvApp *app=sv_app_create((SvAlertSink){0});assert(app && sv_app_open(app,config.version)==SV_OK);
    uint64_t generation=sv_app_view(app).generation;
    size_t at=0,count=0;
    while(at<size) {
        unsigned packet=bytes[at];size_t consumed=0;SvResult result;
        if(packet==PKT_CHAR_INFO) {
            SvCharacterIdentity identity;
            result=sv_character_identity_decode(config.version,bytes+at,size-at,&identity,&consumed);
            printf("IDENTITY offset=%zu result=%d size=%zu name=%s frames=%zu\n",at,result,consumed,
                result==SV_OK?identity.name:"",count);break;
        }
        if(sv_startup_prefix_supports(packet)) {
            result=sv_startup_prefix_decode(&config,bytes+at,size-at,&consumed);
        } else if(sv_protocol_supports_packet(packet)) {
            result=SV_WAITING;
            while(at+consumed<size) {
                SvResult received=sv_app_receive(app,generation,bytes+at+consumed,1);
                if(received!=SV_OK) { result=received;break; }
                consumed++;SvStep step=sv_app_step(app,1);
                if(step.processed) { result=SV_OK;break; }
                if(step.result!=SV_OK && step.result!=SV_WAITING) { result=step.result;break; }
            }
        } else { printf("UNSUPPORTED offset=%zu packet=%u frames=%zu\n",at,packet,count);break; }
        printf("frame offset=%zu packet=%u result=%d size=%zu\n",at,packet,result,consumed);
        if(result!=SV_OK || !consumed)break;
        at+=consumed;count++;
    }
    sv_app_destroy(app);return 0;
}
