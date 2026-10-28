/* D shell, derived from the frozen login.html under lib/xtra/sv/login/reference. */
#include "ui/login-shell.h"
#include "ui/login-layout.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../../../../lib/xtra/sv/login/logo-data.h"

#define SHELL_PROPERTY "sv.login.shell"
#define TEXT_CACHE_SIZE 128
typedef struct {
    SDL_Texture *texture;
    char value[256];
    float size, width, height;
    SDL_Color color;
} TextCache;
typedef struct {
    TTF_Font *font, *heading;
    TextCache text[TEXT_CACHE_SIZE];
    unsigned next_text;
    SDL_Texture *background[2], *icons[6];
    char directory[4096];
    unsigned current, previous;
    Uint64 elapsed, last, frame, rotation;
    bool reduced;
} Shell;
static const char *const backgrounds[] = {
    "cirith_ungol_ascii_seed06.png",
    "cloud_planes_ascii_seed14.png",
    "erebor_ascii_seed19.png",
    "halls_of_mandos_ascii_seed9.png",
    "heart_of_earth_ascii_seed17.png",
    "helcaraxe_ascii_seed31.png",
    "illusory_castle_ascii_seed16.png",
    "land_of_rhun_ascii_seed36.png",
    "mirkwood_ascii_seed01.png",
    "moria_ascii_seed22.png",
    "mount_doom_ascii_seed24.png",
    "nether_realm_depth195_ascii_seed34.png",
    "old_forest_ascii_seed04.png",
    "orc_caves_ascii_3168x1584.png",
    "sacred_land_of_mountains_ascii_seed70.png",
    "sandworm_lair_ascii_seed31.png",
    "small_water_cave_ascii_seed44.png",
    "submerged_ruins_ascii_seed01.png",
    "training_tower_ascii_seed404.png",
};
static void SDLCALL close_shell(void *unused, void *value)
{
    (void)unused;
    Shell *s=value;
    for (int i=0;i<2;++i) SDL_DestroyTexture(s->background[i]);
    for (int i=0;i<6;++i) SDL_DestroyTexture(s->icons[i]);
    for(unsigned i=0;i<TEXT_CACHE_SIZE;++i) SDL_DestroyTexture(s->text[i].texture);
    if (s->font) TTF_CloseFont(s->font);
    if (s->heading) TTF_CloseFont(s->heading);
    SDL_free(s);
}
static SDL_Texture *load_image(SDL_Renderer *r,Shell *s,const char *name,bool blur)
{
    char path[8192];
    SDL_snprintf(path,sizeof(path),"%s/%s",s->directory,name);
    png_image png={0}; png.version=PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&png,path)) {
        SDL_SetError("B2 background unavailable: %s",path); return NULL;
    }
    png.format=PNG_FORMAT_RGBA;
    SDL_Surface *surface=SDL_CreateSurface((int)png.width,(int)png.height,SDL_PIXELFORMAT_RGBA32);
    if (!surface || !png_image_finish_read(&png,NULL,surface->pixels,surface->pitch,NULL)) {
        png_image_free(&png); SDL_DestroySurface(surface);
        SDL_SetError("B2 background decode failed: %s",path); return NULL;
    }
    png_image_free(&png);
    if (blur) {
        SDL_Surface *small=SDL_CreateSurface(640,320,SDL_PIXELFORMAT_RGBA32);
        if (!small || !SDL_BlitSurfaceScaled(surface,NULL,small,NULL,SDL_SCALEMODE_LINEAR)) {
            SDL_DestroySurface(small);SDL_DestroySurface(surface);return NULL;
        }
        SDL_DestroySurface(surface);surface=small;
    }
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);
    SDL_DestroySurface(surface);
    if (texture) SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_LINEAR);
    return texture;
}
static SDL_Texture *image(SDL_Renderer *r,Shell *s,unsigned index)
{ return load_image(r,s,backgrounds[index],true); }
bool sv_login_shell_reset(SDL_Renderer *r)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if (!s) return true;
    for(int i=0;i<2;++i) {SDL_DestroyTexture(s->background[i]);s->background[i]=NULL;}
    for(int i=0;i<6;++i) {SDL_DestroyTexture(s->icons[i]);s->icons[i]=NULL;}
    for(unsigned i=0;i<TEXT_CACHE_SIZE;++i) {
        SDL_DestroyTexture(s->text[i].texture);s->text[i].texture=NULL;
    }
    s->next_text=0;
    s->background[0]=image(r,s,s->current);
    s->rotation=s->elapsed;
    for(int i=0;i<6;++i) {
        char name[32];SDL_snprintf(name,sizeof(name),"realm-%d.png",i);
        s->icons[i]=load_image(r,s,name,false);
        if(!s->icons[i])return false;
    }
    return s->background[0]!=NULL;
}
bool sv_login_shell_open(SDL_Renderer *r,const char *library)
{
    Shell *s=SDL_calloc(1,sizeof(*s));
    if (!s) return false;
    char path[8192];
    if (SDL_snprintf(s->directory,sizeof(s->directory),"%s/xtra/sv/login",library)>=(int)sizeof(s->directory)) {
        close_shell(NULL,s); return SDL_SetError("B2 resource path too long");
    }
    SDL_snprintf(path,sizeof(path),"%s/LiberationMono-Regular.ttf",s->directory);
    s->font=TTF_OpenFont(path,14);
    SDL_snprintf(path,sizeof(path),"%s/LiberationSerif-Regular.ttf",s->directory);
    s->heading=TTF_OpenFont(path,19);
    s->reduced=SDL_getenv("SV_B2_REDUCED_MOTION") && strcmp(SDL_getenv("SV_B2_REDUCED_MOTION"),"0");
    const char *chosen=SDL_getenv("SV_B2_BACKGROUND");
    s->current=chosen?(unsigned)SDL_atoi(chosen)%19:(unsigned)(SDL_GetTicksNS()%19);
    s->last=SDL_GetTicks();
    if (!s->font || !s->heading) {close_shell(NULL,s);return false;}
    if (!SDL_SetPointerPropertyWithCleanup(SDL_GetRendererProperties(r),SHELL_PROPERTY,s,close_shell,NULL)) {
        close_shell(NULL,s);return false;
    }
    return sv_login_shell_reset(r);
}
static bool box(SDL_Renderer *r,SvLoginRect b,float scale,SDL_Color c,bool outline)
{
    SDL_FRect rect={b.x*scale,b.y*scale,b.w*scale,b.h*scale};
    return SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a) &&
        (outline?SDL_RenderRect(r,&rect):SDL_RenderFillRect(r,&rect));
}
static bool text(SDL_Renderer *r,Shell *s,const char *value,float x,float y,
                 float points,float scale,SDL_Color color)
{
    if (!*value) return true;
    char clean[256];size_t n=0;
    for (;value[n] && n+1<sizeof(clean);++n)
        clean[n]=(unsigned char)value[n]>=32 && (unsigned char)value[n]<=126?value[n]:'?';
    clean[n]=0;
    float size=points*scale;
    TextCache *cached=NULL;
    for(unsigned i=0;i<TEXT_CACHE_SIZE;++i) {
        TextCache *entry=&s->text[i];
        if(entry->texture && entry->size==size && !strcmp(entry->value,clean) &&
           entry->color.r==color.r && entry->color.g==color.g &&
           entry->color.b==color.b && entry->color.a==color.a) {cached=entry;break;}
    }
    if(!cached) {
        if (!TTF_SetFontSize(s->font,size)) return false;
        SDL_Surface *surface=TTF_RenderText_Blended(s->font,clean,0,color);
        if (!surface) return false;
        SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);
        if(!texture) {SDL_DestroySurface(surface);return false;}
        cached=&s->text[s->next_text++%TEXT_CACHE_SIZE];
        SDL_DestroyTexture(cached->texture);
        *cached=(TextCache){.texture=texture,.size=size,.width=(float)surface->w,
                            .height=(float)surface->h,.color=color};
        SDL_strlcpy(cached->value,clean,sizeof(cached->value));
        SDL_DestroySurface(surface);
    }
    SDL_FRect rect={x*scale,y*scale,cached->width,cached->height};
    return SDL_RenderTexture(r,cached->texture,NULL,&rect);
}
static SDL_Color flame(unsigned row,unsigned column,Uint64 frame,bool reduced)
{
    static const SDL_Color colors[]={{0,0,0,255},{255,141,0,255},{183,0,0,255},{255,255,0,255},{255,48,48,255}};
    static const unsigned still[]={2,3,2,2,4,2};
    unsigned hash=(unsigned)frame*1664525u+column*1013904223u+row*374761393u;
    unsigned color=still[row];
    if (!reduced) {
        hash^=hash>>13;
        color=row==0?(hash%2?0:hash%3?3:2):row==1?(hash%5>2?1:3):
            row==2 || row==3?(hash%7==6?3:hash%3>1?2:4):
            row==4?(hash%5==0?2:4):(hash%6?2:hash%2?1:4);
    }
    return colors[color];
}
static bool logo(SDL_Renderer *r,Shell *s,SvLoginRect rect,float scale)
{
    float cw=rect.w/80, ch=rect.h/6;
    for (unsigned row=0;row<6;++row) for (unsigned col=0;col<80;++col) {
        char c=logo_lines[row][col];
        if (c==' ' || c=='|') continue;
        SDL_Color color=flame(row,col,s->frame,s->reduced);
        SvLoginRect cell={rect.x+col*cw,rect.y+row*ch,cw,ch};
        if (c=='#') {
            cell.x=SDL_floorf(cell.x*scale)/scale;cell.y=SDL_floorf(cell.y*scale)/scale;
            cell.w+=1/scale;cell.h+=1/scale;
            if (!box(r,cell,scale,color,false)) return false;
        }
        else if (c=='^') {
            if (!text(r,s,"^",cell.x,cell.y,ch*.83f,scale,color)) return false;
        } else {
            bool left=(col>0 && logo_lines[row][col-1]=='#') || (col<79 && logo_lines[row][col+1]==' ');
            SDL_FPoint a={cell.x*scale,cell.y*scale},b={(cell.x+cw)*scale,cell.y*scale},
                       d={cell.x*scale,(cell.y+ch)*scale},e={(cell.x+cw)*scale,(cell.y+ch)*scale};
            SDL_FColor fc={color.r/255.f,color.g/255.f,color.b/255.f,1};
            SDL_Vertex v[3]={0};
            v[0].position=c=='\\'?(left?a:a):(left?a:b);
            v[1].position=c=='\\'?(left?d:b):(left?b:e);
            v[2].position=c=='\\'?e:d;
            for(int i=0;i<3;++i)v[i].color=fc;
            if(!SDL_RenderGeometry(r,NULL,v,3,NULL,0))return false;
        }
    }
    return true;
}
/* Shared account decoration: account consumer owns placement; the shell owns
 * elapsed animation and text/font resources. Narrow layouts omit this call. */
bool sv_login_shell_torches(SDL_Renderer *r,float left,float right,float top,float scale)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if(!s)return false;
    static const char *const glyphs[]={"^","*","#","|","|","|","|","~"};
    for(unsigned side=0;side<2;++side)for(unsigned row=0;row<8;++row) {
        SDL_Color color=row<3?flame(row,side,s->frame,s->reduced):
            row==7?(SDL_Color){85,89,80,255}:(SDL_Color){140,101,15,255};
        if(!text(r,s,glyphs[row],side?right:left,top+row*24,22,scale,color))return false;
    }
    return true;
}
bool sv_login_shell_text(SDL_Renderer *r,const char *value,float x,float y,
                         float size,float scale,SDL_Color color)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    return s && text(r,s,value,x,y,size,scale,color);
}

bool sv_login_shell_heading(SDL_Renderer *r,const char *value,float x,float y,
                            float size,float scale,SDL_Color color)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if(!s || !TTF_SetFontSize(s->heading,size*scale))return false;
    SDL_Surface *surface=TTF_RenderText_Blended(s->heading,value,0,color);
    if(!surface)return false;
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);
    SDL_FRect rect={x*scale,y*scale,(float)surface->w,(float)surface->h};
    SDL_DestroySurface(surface);
    bool ok=texture && SDL_RenderTexture(r,texture,NULL,&rect);
    SDL_DestroyTexture(texture);
    return ok;
}

bool sv_login_shell_render(SDL_Renderer *r,const SvEndpoint *endpoint,const SvEndpointInput *input)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if (!s) return SDL_SetError("B2 shell resources have not been initialized");
    Uint64 now=SDL_GetTicks(),delta=now-s->last;s->last=now;
    SDL_Window *window=SDL_GetRenderWindow(r);
    if (SDL_GetWindowFlags(window)&(SDL_WINDOW_HIDDEN|SDL_WINDOW_MINIMIZED))return true;
    if (!s->reduced) s->elapsed+=delta;
    s->frame=s->elapsed/100;
    if (!s->reduced && s->elapsed-s->rotation>=30000) {
        SDL_Texture *next=image(r,s,(s->current+1)%19);
        if (!next) return false;
        SDL_DestroyTexture(s->background[1]);s->background[1]=s->background[0];
        s->background[0]=next;s->previous=s->current;s->current=(s->current+1)%19;s->rotation=s->elapsed;
    }
    int w,h;if (!SDL_GetCurrentRenderOutputSize(r,&w,&h))return false;
    float scale=SDL_GetWindowDisplayScale(window);if(scale<=0)scale=1;
    SvLoginLayout l=sv_login_layout(w/scale,h/scale,endpoint->server_count,endpoint->selected);
    if(!SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND) ||
       !SDL_SetRenderDrawColor(r,2,4,3,255) || !SDL_RenderClear(r))return false;
    float motion=s->reduced?0:(1-SDL_cosf((float)(s->elapsed%18000)*3.14159265f/9000))/2;
    for(int layer=1;layer>=0;--layer) if(s->background[layer]) {
        float tw,th;if(!SDL_GetTextureSize(s->background[layer],&tw,&th))return false;
        float fit=SDL_max(w/tw,h/th)*(1.1024f+.015f*motion);
        SDL_FRect dest={(w-tw*fit)/2-w*.002f*motion,(h-th*fit)/2-h*.0035f*motion,tw*fit,th*fit};
        Uint8 alpha=layer==0 && s->background[1]?(Uint8)SDL_min(255,(s->elapsed-s->rotation)*255/2000):255;
        if(!SDL_SetTextureBlendMode(s->background[layer],SDL_BLENDMODE_BLEND) ||
           !SDL_SetTextureAlphaMod(s->background[layer],alpha) ||
           !SDL_RenderTexture(r,s->background[layer],NULL,&dest))return false;
    }
    if(s->background[1] && s->elapsed-s->rotation>=2000) {
        SDL_DestroyTexture(s->background[1]);s->background[1]=NULL;
    }
    SDL_Color normal={238,234,221,255}, muted={133,141,131,255},gold={201,170,77,255};
    if(!box(r,(SvLoginRect){0,0,w/scale,h/scale},scale,(SDL_Color){0,0,0,140},false) ||
       !logo(r,s,l.logo,scale) ||
       !box(r,l.panel,scale,(SDL_Color){4,8,5,232},false) ||
       !box(r,l.panel,scale,(SDL_Color){130,111,62,133},true))return false;
    if(!l.compact) {
        if(!box(r,l.chronicle,scale,(SDL_Color){4,8,5,51},false) ||
           !text(r,s,"World Chronicle",l.chronicle.x+8,l.chronicle.y+12,14,scale,gold) ||
           !text(r,s,"News unavailable.",l.chronicle.x+8,l.chronicle.y+48,12,scale,muted))return false;
    }
    static const char *labels[]={"TomeNET website","Discord chat","IRC channel","Official Guide","Tangar's Ru Guide","Github"};
    for(int i=0;i<6;++i) {
        float size=l.compact?40:48,x=l.links.x+(l.compact?i*50:0),y=l.links.y+(l.compact?0:i*61);
        SDL_FRect icon={x*scale,y*scale,size*scale,size*scale};
        if(!SDL_RenderTexture(r,s->icons[i],NULL,&icon))return false;
        if(!l.compact && !text(r,s,labels[i],x+59,y+15,10,scale,muted))return false;
    }
    if(endpoint->phase == SV_ENDPOINT_LIST || endpoint->phase == SV_ENDPOINT_MANUAL) {
    if(!text(r,s,"Up/Down select  |  a-z select directly  |  Enter connect  |  Q direct",l.list.x,l.panel.y+24,10,scale,muted) ||
       !text(r,s,"AVAILABLE SERVERS",l.list.x,l.list.y-27,10,scale,gold) ||
       !box(r,(SvLoginRect){l.list.x,l.list.y-10,l.list.w,1},scale,(SDL_Color){105,95,65,140},false))return false;
    SDL_Rect clip={(int)(l.list.x*scale),(int)(l.list.y*scale),(int)(l.list.w*scale),(int)(l.list.h*scale)};
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    bool ok=true;
    if(!endpoint->server_count)ok=text(r,s,"No server list available.",l.list.x+12,l.list.y+14,14,scale,muted);
    for(size_t i=l.first;ok && i<endpoint->server_count && i<l.first+l.rows;++i) {
        SvLoginRect row=sv_login_row(l,i);const SvServer *server=&endpoint->servers[i];
        bool selected=i==endpoint->selected;
        bool hover=input && input->hover==i;
        char info[128],name[128],badge[4];
        SDL_snprintf(badge,sizeof(badge),"%c",(int)('a'+i));
        int host_chars=(int)((row.w-130)/8.4f)-6;
        if(host_chars<1)host_chars=1;
        SDL_snprintf(name,sizeof(name),"%.*s:%u",host_chars,server->host,server->port);
        if(server->ping_ms>=0)SDL_snprintf(info,sizeof(info),"%.42s  %d ms",server->label,server->ping_ms);
        else SDL_snprintf(info,sizeof(info),"%.42s  ping %s",server->label,server->ping_ms==-1?"pending":"unavailable");
        ok=box(r,row,scale,(selected || hover)?(SDL_Color){78,61,18,102}:(SDL_Color){4,7,5,184},false) &&
           box(r,row,scale,(selected || hover)?(SDL_Color){201,170,77,220}:(SDL_Color){105,95,65,140},true) &&
           box(r,(SvLoginRect){row.x+14,row.y+(row.h-30)/2,34,30},scale,(SDL_Color){105,95,65,140},true) &&
           text(r,s,badge,row.x+28,row.y+row.h/2-5,10,scale,gold) &&
           text(r,s,name,row.x+62,row.y+16,14,scale,selected?gold:normal) &&
           text(r,s,info,row.x+62,row.y+40,11,scale,muted) &&
           box(r,(SvLoginRect){row.x+row.w-47,row.y+(row.h-34)/2,34,34},scale,gold,true) &&
           box(r,(SvLoginRect){row.x+row.w-38,row.y+row.h/2-11,16,23},scale,gold,true) &&
           text(r,s,">",row.x+row.w-35,row.y+row.h/2-8,16,scale,gold);
    }
    if(!SDL_SetRenderClipRect(r,NULL) || !ok)return false;
    if(!box(r,l.manual,scale,(SDL_Color){17,18,8,184},false) ||
       !box(r,l.manual,scale,input && (input->hover_manual || input->list_focus_manual)?gold:(SDL_Color){130,111,62,133},true) ||
       !text(r,s,"Q  Direct connection",l.manual.x+12,l.manual.y+8,12,scale,gold))return false;
    }
    /* Launcher policy belongs to C060; visibly disabled realm controls. */
    if(!text(r,s,"Links unavailable",l.compact?l.links.x+94:l.links.x,
             l.compact?l.links.y+41:l.links.y+370,l.compact?8:10,scale,muted))return false;
    return true;
}
