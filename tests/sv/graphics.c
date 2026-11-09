#define _XOPEN_SOURCE 700
#include "settings/graphics.h"
#include "protocol/login.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static void directory(const char *root,const char *relative)
{char path[1024];assert(snprintf(path,sizeof(path),"%s/%s",root,relative)<(int)sizeof(path));assert(!mkdir(path,0700));}
static void file(const char *root,const char *relative,const char *contents)
{char path[1024];assert(snprintf(path,sizeof(path),"%s/%s",root,relative)<(int)sizeof(path));FILE *f=fopen(path,"w");assert(f);assert(fputs(contents,f)>=0);assert(!fclose(f));}
static void sheet(const char *root,const char *name,SDL_Color color)
{
    char path[1024];assert(snprintf(path,sizeof(path),"%s/xtra/graphics/%s",root,name)<(int)sizeof(path));
    SDL_Surface *s=SDL_CreateSurface(4,2,SDL_PIXELFORMAT_RGBA32);assert(s);
    assert(SDL_FillSurfaceRect(s,NULL,SDL_MapSurfaceRGBA(s,color.r,color.g,color.b,255)));
    assert(SDL_SaveBMP(s,path));SDL_DestroySurface(s);
}
static void wire(const SvGraphics *g,const SvPreferences *p)
{
    const int version[6]={4,9,4,0,0,0};const unsigned char address[6]={0};
    unsigned char bytes[32768];SvVisualDefinitions definitions;sv_map_font_definitions(&definitions,p);
    SvLogin *login=sv_login_create(version,address);assert(login);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_OK);
    assert(sv_login_set_rendering(login,&definitions,sv_graphics_mode(g),sv_graphics_name(g),"map.pcf")==SV_OK);
    unsigned char overview[29]={PKT_SERVERDETAILS};overview[17]=PKT_LOGIN;
    assert(sv_login_receive(login,overview,sizeof(overview))==SV_OK);
    assert(sv_login_create_character(login,"Graphics",false)==SV_OK);
    assert(sv_login_take_output(login,bytes,sizeof(bytes)).result==SV_OK);
    const unsigned char accepted=E_NEED_INFO;assert(sv_login_receive(login,&accepted,1)==SV_OK);
    int16_t stats[6]={0};assert(sv_login_start_play(login,0,0,0,0,stats,"map.pcf")==SV_OK);
    SvOutput out=sv_login_take_output(login,bytes,sizeof(bytes));
    assert(out.result==SV_OK && bytes[0]==PKT_PLAY && bytes[13]==0 && bytes[14]==2);
    assert(!strcmp((char *)bytes+15,sv_graphics_name(g)));
    const unsigned char reply[]={PKT_REPLY,PKT_PLAY,PKT_SUCCESS};assert(sv_login_receive(login,reply,sizeof(reply))==SV_OK);
    for(unsigned length=78;length<=80;++length) {
        char name[81];memset(name,'q',length);name[length]=0;
        assert(sv_login_update_rendering(login,&definitions,2,name,"map.pcf")==SV_OK);
        out=sv_login_take_output(login,bytes,sizeof(bytes));assert(out.result==SV_OK);
        size_t setup=(127+512+1280+1280)*5u;
        assert(bytes[setup+1]==PKT_FONT && bytes[setup+2]==0 && bytes[setup+3]==2);
        assert(strlen((char *)bytes+setup+4)==(length<80?length:79));
        assert(strlen(name)==length);
    }
    sv_login_destroy(login);
}
int main(void)
{
    assert(SDL_Init(SDL_INIT_VIDEO));
    char root[]="/tmp/sv-graphics-XXXXXX";assert(mkdtemp(root));
    directory(root,"xtra");directory(root,"xtra/graphics");directory(root,"user");
    sheet(root,"2x2test.bmp",(SDL_Color){230,20,30,255});
    sheet(root,"2x2test#0_green.bmp",(SDL_Color){20,230,30,255});
    file(root,"user/graphics-2x2test.prf","F:1:3:0\nK:1:4:1\nI:1:0:0:4:2\nm:0:#000000\nm:1:#fc00fb\n");
    file(root,"user/graphics-2x2test#0_green.prf","R:1:5:0\nm:0:#111111\nI:2:0:0:2:2\nBAD\nK:2:6:0\n");
    SvPreferences *p=sv_preferences_create(root,root,NULL);assert(p);
    SvSettingsConfig config={0};config.graphics=2;config.graphic_subsets[0]=1;
    strcpy(config.graphic_tiles,"2x2test");strcpy(config.graphics_filter,"nearest");config.graphics_force_outline=-1;
    SvPreferenceContext ctx=sv_preferences_default_context();ctx.generation=7;char error[256];
    SvGraphics *g=sv_graphics_prepare(root,root,&config,p,&ctx,4,4,error,sizeof(error));
    if(!g)fprintf(stderr,"%s\n",error);assert(g);
    assert(sv_graphics_mode(g)==2 && !strcmp(sv_graphics_name(g),"2x2test"));
    assert(sv_preferences_glyph_subtileset(p,256)==0);
    SvVisualDefinitions definitions;sv_map_font_definitions(&definitions,p);
    assert(definitions.features[1].character==256 && definitions.kinds[2].character==256);
    const SvPreparedResource *tile=sv_graphics_tile(g,256,4,4);assert(tile);
    SDL_Surface *surface=sv_resource_composite(tile,(SDL_Color){255,255,255,255},(SDL_Color){0,0,0,255});assert(surface);
    unsigned char *pixel=surface->pixels;assert(pixel[1]>pixel[0]);SDL_DestroySurface(surface);
    const SvPreparedResource *raw=sv_graphics_raw(g,1,4,4);assert(raw && raw->authored->w==8 && raw->authored->h==4);
    SDL_Window *window=NULL;SDL_Renderer *renderer=NULL;assert(SDL_CreateWindowAndRenderer("graphics",64,64,0,&window,&renderer));
    assert(sv_graphics_draw(g,renderer,256,&(SDL_FRect){0,0,4,4},(SDL_Color){255,255,255,255},(SDL_Color){0,0,0,255})==1);
    assert(sv_graphics_draw_raw(g,renderer,1,4,4,4,4)==1);
    wire(g,p);
    sv_graphics_reset_renderer(g);
    assert(sv_graphics_draw(g,renderer,256,&(SDL_FRect){0,0,4,4},(SDL_Color){90,120,150,255},(SDL_Color){0,0,0,255})==1);
    sv_graphics_cache_policy(g,true,true);assert(!sv_graphics_persistent_cache(g));
    assert(sv_graphics_tile(g,256,4,4));sv_graphics_cache_policy(g,false,false);
    SvPreferences *same=sv_preferences_create(root,root,NULL);assert(same);
    SvPreferenceContext other=ctx;other.generation=8;
    SvGraphics *same_graphics=sv_graphics_prepare(root,root,&config,same,&other,4,4,error,sizeof(error));
    assert(same_graphics && sv_graphics_generation(same_graphics)==sv_graphics_generation(g));
    sv_graphics_destroy(same_graphics);sv_preferences_destroy(same);
    /* Filter/outline edits rebuild immutable sources without reloading PRFs or effects. */
    size_t effects=sv_preferences_effect_count(p),records=sv_preferences_record_count(p);
    file(root,"user/graphics-2x2test.prf","F:1:3:99999\n");
    SvGraphics *filtered=sv_graphics_reprepare(g,SV_RESOURCE_LINEAR,2,4,4,error,sizeof(error));assert(filtered);
    assert(sv_graphics_generation(filtered)!=sv_graphics_generation(g));
    assert(sv_preferences_effect_count(p)==effects && sv_preferences_record_count(p)==records);
    const SvPreparedResource *filtered_tile=sv_graphics_tile(filtered,256,4,4);
    assert(filtered_tile && filtered_tile->requested_filter==SV_RESOURCE_LINEAR);
    assert(sv_graphics_raw(filtered,1,4,4));
    SvGraphics *restored=sv_graphics_reprepare(filtered,SV_RESOURCE_NEAREST,-1,4,4,error,sizeof(error));assert(restored);
    assert(sv_graphics_generation(restored)==sv_graphics_generation(g));
    assert(!sv_graphics_reprepare(g,SV_RESOURCE_LINEAR,2,0,4,error,sizeof(error)) && *error);
    assert(sv_graphics_tile(g,256,4,4));
    sv_graphics_destroy(restored);sv_graphics_destroy(filtered);
    file(root,"user/graphics-2x2test.prf","F:1:3:0\nK:1:4:1\nI:1:0:0:4:2\nm:0:#000000\nm:1:#fc00fb\n");
    uint64_t generation=sv_graphics_generation(g);size_t count=sv_preferences_mapping_count(p);
    strcpy(config.graphic_tiles,"missing");
    assert(!sv_graphics_prepare(root,root,&config,p,&ctx,4,4,error,sizeof(error)) && *error);
    assert(sv_graphics_generation(g)==generation && sv_preferences_mapping_count(p)==count);
    assert(sv_graphics_tile(g,256,6,6));
    SvPreferences *fresh=sv_preferences_create(root,root,NULL);assert(fresh);
    strcpy(config.graphic_tiles,"2x2test");config.graphic_subsets[0]=0;
    SvGraphics *next=sv_graphics_prepare(root,root,&config,fresh,&ctx,4,4,error,sizeof(error));assert(next);
    assert(sv_preferences_glyph_subtileset(fresh,256)==-1 && !sv_graphics_raw(next,2,4,4));
    sv_graphics_destroy(next);sv_preferences_destroy(fresh);
    fresh=sv_preferences_create(root,root,NULL);config.graphics=0;
    next=sv_graphics_prepare(root,root,&config,fresh,&ctx,4,4,error,sizeof(error));assert(next);
    assert(!sv_graphics_mode(next) && !sv_graphics_tile(next,256,4,4));
    sv_graphics_destroy(next);sv_preferences_destroy(fresh);
    /* Shipped source exercises actual sheet/PRF geometry and masks. */
    fresh=sv_preferences_create(root,"lib",NULL);config.graphics=1;strcpy(config.graphic_tiles,"16x24sv");
    next=sv_graphics_prepare(root,"lib",&config,fresh,&ctx,16,24,error,sizeof(error));
    if(!next)fprintf(stderr,"shipped: %s\n",error);assert(next);
    assert(sv_graphics_tile(next,306,16,24));
    sv_graphics_destroy(next);sv_preferences_destroy(fresh);
    /* Invalid user overlay must not silently select the valid bundled sheet. */
    directory(root,"xtra/graphics/16x24sv.bmp");
    fresh=sv_preferences_create(root,"lib",NULL);assert(fresh);
    assert(!sv_graphics_prepare(root,"lib",&config,fresh,&ctx,16,24,error,sizeof(error)));
    assert(sv_preferences_mapping_count(fresh)==0);sv_preferences_destroy(fresh);
    sheet(root,"2x2bare.bmp",(SDL_Color){50,60,70,255});
    fresh=sv_preferences_create(root,root,NULL);strcpy(config.graphic_tiles,"2x2bare");
    next=sv_graphics_prepare(root,root,&config,fresh,&ctx,4,4,error,sizeof(error));
    assert(next && *error && sv_graphics_tile(next,256,4,4));
    sv_graphics_destroy(next);sv_preferences_destroy(fresh);
    /* Startup fallback uses the real font resource hook and does not change request. */
    SvMapFont map;assert(sv_map_font_init(&map,root,"lib","16x24x.pcf"));
    strcpy(config.graphic_tiles,"missing");next=NULL;
    SvGraphicsBootstrap bootstrap={.map=&map,.config=&config,.active=&next,.width=4,.height=4};
    fresh=sv_preferences_create(root,"lib",NULL);assert(fresh);
    assert(sv_graphics_bootstrap(fresh,&ctx,&bootstrap).status==SV_PREF_PARTIAL);
    assert(next && !sv_graphics_mode(next) && *bootstrap.error && config.graphics==1);
    sv_graphics_destroy(next);sv_preferences_destroy(fresh);
    sv_graphics_reset_renderer(g);
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);
    sv_graphics_destroy(g);sv_preferences_destroy(p);SDL_Quit();
    puts("graphics: real base/subset prepare/draw, shared glyph identity, raw extents, atomic failure, off, shipped and wire boundaries PASS");
    return 0;
}
