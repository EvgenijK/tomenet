#include "login/ui/login-shell.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
    assert(argc==2 || argc==3);
    assert(SDL_Init(SDL_INIT_VIDEO));assert(TTF_Init());
    SDL_Window *w=SDL_CreateWindow("MOTD diagnostic",1024,768,0);assert(w);
    SDL_Renderer *r=SDL_CreateRenderer(w,NULL);assert(r);
    assert(sv_login_shell_open(r,argv[1]));
    const unsigned char *texts[]={(const unsigned char *)"Managed peer MOTD",(const unsigned char *)"  ",(const unsigned char *)"\xffR                        TomeNET server - v4.9.4"};
    for(unsigned i=0;i<3;i++) {
        SvMotd motd=sv_motd_prepare(texts[i],strlen((const char *)texts[i])+1);
        SDL_ClearError();bool result=sv_login_shell_motd(r,&motd);
        fprintf(stderr,"probe case%u columns%zu result%d error=%s\n",i,motd.columns,result,SDL_GetError());
    }
    if(argc==3) {
        unsigned char bytes[23*120];FILE *file=fopen(argv[2],"rb");assert(file);
        assert(fread(bytes,1,sizeof(bytes),file)==sizeof(bytes) && !fclose(file));
        SvMotd motd=sv_motd_prepare(bytes,sizeof(bytes));
        SDL_ClearError();bool result=sv_login_shell_motd(r,&motd);
        fprintf(stderr,"probe exact-field columns%zu result%d error=%s\n",motd.columns,result,SDL_GetError());
    }
    SDL_DestroyRenderer(r);SDL_DestroyWindow(w);TTF_Quit();SDL_Quit();return 0;
}
