#include "input/chronicle.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(void) {
 assert(SDL_Init(0));
 SvChronicle *provider=sv_chronicle_start(); assert(provider);
 SvChronicleSnapshot *view=calloc(1,sizeof(*view)); assert(view);
 Uint64 start=SDL_GetTicks();
 do {
  if(sv_chronicle_poll(provider,view)) printf("revision=%u count=%zu loading=%d completed=%u failed=%u elapsed=%llu\n",view->revision,view->count,view->loading,view->completed_sources,view->failed_sources,(unsigned long long)(SDL_GetTicks()-start));
  if(view->revision && !view->loading)break;
  SDL_Delay(10);
 }while(SDL_GetTicks()-start<16000);
 bool ok=view->revision && !view->loading && view->completed_sources==9 && view->failed_sources==0 && view->count>0;
 for(size_t i=0;i<view->count && i<8;++i)printf("%u %s %06x %s\n",view->events[i].server,view->events[i].date,view->events[i].color,view->events[i].text);
 sv_chronicle_stop(provider); free(view);
 /* Cancellation leaves owner immediately and never joins a resolver. */
 provider=sv_chronicle_start(); assert(provider); start=SDL_GetTicks(); sv_chronicle_stop(provider);
 printf("cancel_elapsed_ms=%llu\n",(unsigned long long)(SDL_GetTicks()-start));
 SDL_Delay(200);SDL_Quit();return ok?0:1;
}
