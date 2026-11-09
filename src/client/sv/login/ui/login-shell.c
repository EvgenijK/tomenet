#include "login/ui/scale.h"
#include "ui/font.h"
/* D shell, derived from tomenet_interface/login.html in the external UX prototype.
 * Source commit: 1bb2d5d45b9001bf2a6f2084f78184c29a73fca7. See docs/evidence/sv-login/PROVENANCE.md. */
#include "login/ui/login-shell.h"
#include "login/ui/login-layout.h"
#include "login/motd/motd-colors.h"
#include "login/login-view.h"
#include "login/chronicle.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../../../../../lib/xtra/sv/login/logo-data.h"

#define SHELL_PROPERTY "sv.login.shell"
#define TEXT_CACHE_SIZE 128
typedef struct {
    SDL_Texture *texture;
    char value[256];
    float size, width, height;
    SDL_Color color;
    bool bold;
} TextCache;
typedef struct {
    TTF_Font *font, *heading;
    SvFont *selected_text;
    TextCache text[TEXT_CACHE_SIZE];
    unsigned next_text;
    SDL_Texture *background[2], *icons[6][2], *doors[2], *motd_texture, *glow;
    int background_lower[2],background_count[2];
    SvMotd motd;
    int motd_width,motd_height,motd_raster_width,motd_raster_height;
    float motd_dpi;
    uint32_t motd_tick;
    bool motd_valid;
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
    for (int i=0;i<6;++i)for(int state=0;state<2;++state) SDL_DestroyTexture(s->icons[i][state]);
    for (int i=0;i<2;++i) SDL_DestroyTexture(s->doors[i]);
    for(unsigned i=0;i<TEXT_CACHE_SIZE;++i) SDL_DestroyTexture(s->text[i].texture);
    SDL_DestroyTexture(s->motd_texture);
    SDL_DestroyTexture(s->glow);
    sv_font_close(s->selected_text);
    if (s->font) TTF_CloseFont(s->font);
    if (s->heading) TTF_CloseFont(s->heading);
    SDL_free(s);
}
/* Three box convolutions approximate CSS Gaussian blur at the decoded
 * resolution. O(pixels) preparation needs one temporary surface, only when
 * loading artwork or changing the blur kernel, never on an ordinary frame. */
static void background_box(SDL_Surface *source,SDL_Surface *target,int radius,bool vertical)
{
    int lines=vertical?source->w:source->h,length=vertical?source->h:source->w;
    int step=vertical?source->pitch:4,line_step=vertical?4:source->pitch;
    int target_step=vertical?target->pitch:4,target_line=vertical?4:target->pitch;
    int count=radius*2+1;
    for(int line=0;line<lines;++line) {
        Uint8 *in=(Uint8 *)source->pixels+line*line_step;
        Uint8 *out=(Uint8 *)target->pixels+line*target_line;
        int sums[4]={0};
        for(int k=-radius;k<=radius;++k) {
            Uint8 *pixel=in+SDL_clamp(k,0,length-1)*step;
            for(int c=0;c<4;++c)sums[c]+=pixel[c];
        }
        for(int pos=0;pos<length;++pos) {
            for(int c=0;c<4;++c)out[pos*target_step+c]=(Uint8)((sums[c]+count/2)/count);
            Uint8 *remove=in+SDL_clamp(pos-radius,0,length-1)*step;
            Uint8 *add=in+SDL_clamp(pos+radius+1,0,length-1)*step;
            for(int c=0;c<4;++c)sums[c]+=add[c]-remove[c];
        }
    }
}
static void background_kernel(SDL_Renderer *r,int width,int height,int *lower,int *low_count)
{
    int w=width,h=height;
    SDL_GetCurrentRenderOutputSize(r,&w,&h);
    float scale=sv_login_ui_scale(SDL_GetRenderWindow(r));
    if(scale<=0)scale=1;
    float fit=SDL_max((float)w/width,(float)h/height)*1.1024f;
    /* CSS 2 logical px, scaled by the 1.04 layer transform, expressed in
     * source pixels before cover sampling. Flight adds at most 1.4%. */
    float sigma=2.08f*scale/fit;
    *lower=(int)SDL_floorf(SDL_sqrtf(4*sigma*sigma+1));
    if(!(*lower&1))--*lower;
    *lower=SDL_max(1,*lower);
    *low_count=(int)SDL_roundf((12*sigma*sigma-3*(*lower)*(*lower)-12*(*lower)-9)/(-4*(*lower)-4));
}
static bool filter_background(SDL_Surface *surface,int lower,int low_count)
{
    SDL_Surface *scratch=SDL_CreateSurface(surface->w,surface->h,SDL_PIXELFORMAT_RGBA32);
    if(!scratch)return false;
    /* CSS contrast(.78) precedes blur(2px), centered on the midpoint. */
    for(int y=0;y<surface->h;++y) {
        Uint8 *row=(Uint8 *)surface->pixels+y*surface->pitch;
        for(int x=0;x<surface->w;++x)
            for(int c=0;c<3;++c)row[x*4+c]=(Uint8)SDL_roundf(row[x*4+c]*.78f+255*.11f);
    }
    for(int pass=0;pass<3;++pass) {
        int radius=((pass<low_count?lower:lower+2)-1)/2;
        background_box(surface,scratch,radius,false);
        background_box(scratch,surface,radius,true);
    }
    SDL_DestroySurface(scratch);
    return true;
}
static SDL_Texture *load_filtered_image(SDL_Renderer *r,Shell *s,const char *name,int *lower,int *count)
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
    if(lower) {
        background_kernel(r,surface->w,surface->h,lower,count);
        if(!filter_background(surface,*lower,*count)) {SDL_DestroySurface(surface);return NULL;}
    }
    SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);
    SDL_DestroySurface(surface);
    if (texture) SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_LINEAR);
    return texture;
}
static SDL_Texture *load_image(SDL_Renderer *r,Shell *s,const char *name)
{ return load_filtered_image(r,s,name,NULL,NULL); }
static SDL_Texture *image(SDL_Renderer *r,Shell *s,unsigned index,int slot)
{ return load_filtered_image(r,s,backgrounds[index],&s->background_lower[slot],&s->background_count[slot]); }
bool sv_login_shell_reset(SDL_Renderer *r)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if (!s) return true;
    for(int i=0;i<2;++i) {SDL_DestroyTexture(s->background[i]);s->background[i]=NULL;}
    for(int i=0;i<6;++i)for(int state=0;state<2;++state) {SDL_DestroyTexture(s->icons[i][state]);s->icons[i][state]=NULL;}
    for(unsigned i=0;i<TEXT_CACHE_SIZE;++i) {
        SDL_DestroyTexture(s->text[i].texture);s->text[i].texture=NULL;
    }
    for(int i=0;i<2;++i) {SDL_DestroyTexture(s->doors[i]);s->doors[i]=NULL;}
    s->next_text=0;
    SDL_DestroyTexture(s->glow);s->glow=NULL;
    SDL_DestroyTexture(s->motd_texture);s->motd_texture=NULL;s->motd_valid=false;
    s->background[0]=image(r,s,s->current,0);
    s->rotation=s->elapsed;
    for(int i=0;i<6;++i)for(int state=0;state<2;++state) {
        char name[32];SDL_snprintf(name,sizeof(name),state?"realm-%d-hover.png":"realm-%d.png",i);
        s->icons[i][state]=load_image(r,s,name);
        if(!s->icons[i][state])return false;
    }
    for(int i=0;i<2;++i) {
        s->doors[i]=load_image(r,s,i?"server-door-open.png":"server-door.png");
        if(!s->doors[i])return false;
    }
    return s->background[0]!=NULL;
}
bool sv_login_shell_open_options(SDL_Renderer *r,const char *library,
                                SvLoginShellOptions options)
{
    Shell *s=SDL_calloc(1,sizeof(*s));
    if (!s) return false;
    char path[8192];
    if (SDL_snprintf(s->directory,sizeof(s->directory),"%s/xtra/sv/login",library)>=(int)sizeof(s->directory)) {
        close_shell(NULL,s); return SDL_SetError("B2 resource path too long");
    }
    SDL_snprintf(path,sizeof(path),"%s/LiberationMono-Regular.ttf",s->directory);
    s->font=TTF_OpenFont(path,14);
    if (options.text_font_path) {
        s->selected_text=sv_font_open_file(options.text_font_path);
        if (!s->selected_text) { close_shell(NULL,s); return false; }
    }
    SDL_snprintf(path,sizeof(path),"%s/LiberationSerif-Regular.ttf",s->directory);
    s->heading=TTF_OpenFont(path,19);
    s->reduced=options.reduced_motion;
    s->current=options.background_selected?options.background%19:(unsigned)(SDL_GetTicksNS()%19);
    s->last=SDL_GetTicks();
    if (!s->font || !s->heading) {close_shell(NULL,s);return false;}
    if (!SDL_SetPointerPropertyWithCleanup(SDL_GetRendererProperties(r),SHELL_PROPERTY,s,close_shell,NULL)) {
        close_shell(NULL,s);return false;
    }
    return sv_login_shell_reset(r);
}
bool sv_login_shell_open(SDL_Renderer *r,const char *library)
{
    return sv_login_shell_open_options(r,library,(SvLoginShellOptions){0});
}
static bool box(SDL_Renderer *r,SvLoginRect b,float scale,SDL_Color c,bool outline)
{
    SDL_FRect rect={b.x*scale,b.y*scale,b.w*scale,b.h*scale};
    return SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a) &&
        (outline?SDL_RenderRect(r,&rect):SDL_RenderFillRect(r,&rect));
}
/* Pixel-wide alpha fringes smooth rounded CSS edges on both software and
 * GPU renderers. Geometry remains in logical units at the current DPI. */
static bool roundbox(SDL_Renderer *r,SvLoginRect b,float radius,float scale,SDL_Color color,bool outline)
{
    enum { N=68, RINGS=4 };
    SDL_Vertex vertices[N*RINGS+1];int indices[N*6*3];
    float aa=.5f/scale;
    float inset[4]={-aa,aa,1-aa,1+aa};
    int rings=outline?4:2,nindices=0;
    radius=SDL_min(radius,SDL_min(b.w,b.h)/2);
    for(int ring=0;ring<rings;++ring) {
        float d=inset[ring],rad=SDL_max(0,radius-d);
        float alpha=ring==0 || (outline && ring==3)?0:color.a/255.f;
        for(int corner=0;corner<4;++corner)for(int step=0;step<17;++step) {
            int i=corner*17+step;
            float angle=(-90+corner*90+step*90.f/16)*3.14159265f/180;
            float cx=b.x+(corner<2?b.w-radius:radius),cy=b.y+(corner==0 || corner==3?radius:b.h-radius);
            vertices[ring*N+i]=(SDL_Vertex){.position={(cx+rad*SDL_cosf(angle))*scale,
                (cy+rad*SDL_sinf(angle))*scale},.color={color.r/255.f,color.g/255.f,color.b/255.f,alpha}};
        }
        if(ring)for(int i=0;i<N;++i) {
            int a=(ring-1)*N+i,bb=(ring-1)*N+(i+1)%N,c=ring*N+i,d2=ring*N+(i+1)%N;
            int pair[]={a,bb,c,bb,d2,c};
            for(int k=0;k<6;++k)indices[nindices++]=pair[k];
        }
    }
    if(!outline) {
        vertices[rings*N]=(SDL_Vertex){.position={(b.x+b.w/2)*scale,(b.y+b.h/2)*scale},
            .color={color.r/255.f,color.g/255.f,color.b/255.f,color.a/255.f}};
        for(int i=0;i<N;++i) {indices[nindices++]=rings*N;indices[nindices++]=N+i;indices[nindices++]=N+(i+1)%N;}
    }
    return SDL_RenderGeometry(r,NULL,vertices,rings*N+(!outline),indices,nindices);
}
bool sv_login_shell_roundrect(SDL_Renderer *r,SDL_FRect rect,float radius,
                              float scale,SDL_Color color,bool outline)
{
    return roundbox(r,(SvLoginRect){rect.x,rect.y,rect.w,rect.h},radius,scale,color,outline);
}
/* D ::before: vertical black veil and a subtle gold radial glow. The
 * tiny radial mask is created once; the renderer interpolates it at output DPI. */
static bool background_veil(SDL_Renderer *r,Shell *s,int width,int height)
{
    if(!s->glow) {
        SDL_Surface *surface=SDL_CreateSurface(128,128,SDL_PIXELFORMAT_RGBA32);
        if(!surface)return false;
        for(int y=0;y<128;++y)for(int x=0;x<128;++x) {
            float dx=(x+.5f)/128-.5f,dy=(y+.5f)/128-.38f;
            float distance=SDL_sqrtf(dx*dx/.25f+dy*dy/.3844f);
            Uint8 *pixel=(Uint8 *)surface->pixels+y*surface->pitch+x*4;
            pixel[0]=146;pixel[1]=112;pixel[2]=52;
            pixel[3]=(Uint8)(20.4f*SDL_max(0,1-distance/.7354f));
        }
        s->glow=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);
        if(!s->glow || !SDL_SetTextureBlendMode(s->glow,SDL_BLENDMODE_BLEND) ||
           !SDL_SetTextureScaleMode(s->glow,SDL_SCALEMODE_LINEAR))return false;
    }
    SDL_Vertex vertices[]={
        {.position={0,0},.color={0,0,0,.44f}},
        {.position={(float)width,0},.color={0,0,0,.44f}},
        {.position={(float)width,(float)height},.color={0,0,0,.56f}},
        {.position={0,(float)height},.color={0,0,0,.56f}}};
    const int indices[]={0,1,2,0,2,3};
    return SDL_RenderGeometry(r,NULL,vertices,4,indices,6) && SDL_RenderTexture(r,s->glow,NULL,NULL);
}
static bool text_style(SDL_Renderer *r,Shell *s,const char *value,float x,float y,
                 float points,float scale,SDL_Color color,bool bold)
{
    if (!*value) return true;
    char clean[256];size_t n=0;
    for (;value[n] && n+1<sizeof(clean);++n)
        clean[n]=(unsigned char)value[n]>=32?value[n]:' ';
    clean[n]=0;
    float size=points*scale;
    TextCache *cached=NULL;
    for(unsigned i=0;i<TEXT_CACHE_SIZE;++i) {
        TextCache *entry=&s->text[i];
        if(entry->texture && entry->size==size && entry->bold==bold && !strcmp(entry->value,clean) &&
           entry->color.r==color.r && entry->color.g==color.g &&
           entry->color.b==color.b && entry->color.a==color.a) {cached=entry;break;}
    }
    if(!cached) {
        SDL_Surface *surface;
        if (s->selected_text) surface=sv_font_render_surface(s->selected_text,clean,points,scale,color,bold);
        else {
            TTF_SetFontStyle(s->font,bold?TTF_STYLE_BOLD:TTF_STYLE_NORMAL);
            if (!TTF_SetFontSize(s->font,size)) return false;
            surface=TTF_RenderText_Blended(s->font,clean,0,color);
            TTF_SetFontStyle(s->font,TTF_STYLE_NORMAL);
        }
        if (!surface) return false;
        SDL_Texture *texture=SDL_CreateTextureFromSurface(r,surface);
        if(!texture) {SDL_DestroySurface(surface);return false;}
        cached=&s->text[s->next_text++%TEXT_CACHE_SIZE];
        SDL_DestroyTexture(cached->texture);
        *cached=(TextCache){.texture=texture,.size=size,.width=(float)surface->w,
                            .height=(float)surface->h,.color=color,.bold=bold};
        SDL_strlcpy(cached->value,clean,sizeof(cached->value));
        SDL_DestroySurface(surface);
    }
    SDL_FRect rect={x*scale,y*scale,cached->width,cached->height};
    return SDL_RenderTexture(r,cached->texture,NULL,&rect);
}
static bool text(SDL_Renderer *r,Shell *s,const char *value,float x,float y,
                 float points,float scale,SDL_Color color)
{ return text_style(r,s,value,x,y,points,scale,color,false); }
/* Use exactly the output font size/style used by text_style. A nominal
 * monospace advance is not reliable after hinting or synthetic bold at DPI. */
static bool text_width(Shell *s,const char *value,float points,float scale,bool bold,float *width)
{
    int w,h;
    bool ok;
    if (s->selected_text) ok=sv_font_measure(s->selected_text,value,points,scale,&w,&h,bold);
    else {
        TTF_SetFontStyle(s->font,bold?TTF_STYLE_BOLD:TTF_STYLE_NORMAL);
        ok=TTF_SetFontSize(s->font,points*scale) && TTF_GetStringSize(s->font,value,0,&w,&h);
        TTF_SetFontStyle(s->font,TTF_STYLE_NORMAL);
    }
    if(ok)*width=w/scale;
    return ok;
}
bool sv_login_shell_measure(SDL_Renderer *r,const char *value,float size,
                            float scale,bool heading,float *width)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if(!s || !width || scale<=0)return false;
    if(!heading)return text_width(s,value,size,scale,false,width);
    int w,h;
    bool ok=TTF_SetFontSize(s->heading,size*scale) && TTF_GetStringSize(s->heading,value,0,&w,&h);
    if(ok)*width=w/scale;
    return ok;
}
/* Preserve the shared logical hint advances while rasterizing at output DPI.
 * Only this fixed-width paragraph compensates for FreeType pixel hinting. */
static bool hint_text(SDL_Renderer *r,Shell *s,const char *value,float x,float y,
                      float width,float scale,SDL_Color color)
{
    float actual,sx,sy;
    if(!text_width(s,value,10,scale,false,&actual) || actual<=0 ||
       !SDL_GetRenderScale(r,&sx,&sy))return false;
    float factor=width/actual;
    if(!SDL_SetRenderScale(r,sx*factor,sy))return false;
    bool ok=text(r,s,value,x/factor,y,10,scale,color);
    bool restored=SDL_SetRenderScale(r,sx,sy);
    return ok && restored;
}
static bool server_shortcuts(SDL_Renderer *r,Shell *s,SvLoginLayout l,float scale,size_t count,bool direct_hover)
{
    char last[2]={(char)('a'+(count?SDL_min(count,26)-1:0)),0};
    const char *pieces[]={"↑"," ","↓"," select · ","a","–",last," select directly · ","Enter"," connect · press ","Q"," for direct connection"};
    const bool keys[]={true,false,true,false,true,false,true,false,true,false,true,false};
    float widths[12],total=sv_login_hint_width(0,12);
    for(unsigned i=0;i<SDL_arraysize(pieces);++i)widths[i]=sv_login_hint_piece_width(i);
    /* Narrow supported layouts wrap the same hint into two centered lines. */
    bool wrap=total>l.panel.w-42;
    float first=0;for(int i=0;i<(wrap?8:12);++i)first+=widths[i];
    float x=l.panel.x+(l.panel.w-first)/2,y=l.panel.y+(l.row_height<=72?14:26);
    for(unsigned i=0;i<SDL_arraysize(pieces);++i) {
        if(wrap && i==8) {float rest=total-first;x=l.panel.x+(l.panel.w-rest)/2;y+=20;}
        SDL_Color keycolor=i==10 && direct_hover?(SDL_Color){255,226,118,255}:(SDL_Color){217,189,97,255};
        if(keys[i] && (!roundbox(r,(SvLoginRect){x,y-2,widths[i],15},3,scale,(SDL_Color){8,11,8,194},false) ||
                         !roundbox(r,(SvLoginRect){x,y-2,widths[i],15},3,scale,(SDL_Color){77,73,55,255},true)))return false;
        if(!hint_text(r,s,pieces[i],x+(keys[i]?6:0),y,widths[i]-(keys[i]?12:0),scale,
                     keys[i]?keycolor:(SDL_Color){135,139,131,255}))return false;
        x+=widths[i];
    }
    return true;
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
            unsigned hash=(unsigned)s->frame*1664525u+col*1013904223u;
            hash^=hash>>13;
            char glyph[2]={s->reduced?'^':"')(."[hash%4],0};
            if (!text(r,s,glyph,cell.x,cell.y,ch*.83f,scale,color)) return false;
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
        char animated[2]={0};
        const char *glyph=glyphs[row];
        if(!s->reduced && row<2) {
            unsigned hash=(unsigned)s->frame*1664525u+(side*7+row)*1013904223u;
            hash^=hash>>13;
            animated[0]=row==0?"')(."[hash%4]:"0o*"[hash%3];
            glyph=animated;
        }
        SDL_Color color=row<3?(s->reduced?(SDL_Color){183,0,0,255}:
            flame(row==2?5:2,side+row*7,s->frame,false)):
            row==7?(SDL_Color){85,89,80,255}:(SDL_Color){140,101,15,255};
        float x=side?right:left,y=top+row*17.2f;
        if(row>=3 && row<7 && !text_style(r,s,glyph,x+1,y,20,scale,
                                         (SDL_Color){53,38,10,255},true))return false;
        if(!text_style(r,s,glyph,x,y,20,scale,color,true))return false;
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

/* Wrap public UTF-8 at word boundaries. Glyph preparation stays at output DPI;
 * no source/news pixels are cached or scaled as a fixed-size UI surface. */
static bool news_text(SDL_Renderer *r,Shell *s,const char *value,float x,float y,float width,
                      float scale,SDL_Color color,bool draw,float *height)
{
    size_t columns=(size_t)SDL_max(1,SDL_floorf(width/7.2f));
    float used=0;
    while(*value) {
        size_t end=0,space=0,characters=0;
        while(value[end] && value[end]!='\n' && characters<columns && end<sizeof(((TextCache *)0)->value)-5) {
            if(value[end]==' ')space=end;
            ++end;
            while(((unsigned char)value[end]&0xc0)==0x80)++end;
            ++characters;
        }
        if(value[end] && value[end]!='\n' && space)end=space;
        char line[256];SDL_memcpy(line,value,end);line[end]=0;
        if(draw && !text(r,s,line,x,y+used,12,scale,color))return false;
        used+=18.6f;value+=end;
        if(*value=='\n')++value;
        while(*value==' ')++value;
    }
    *height=used;return true;
}
static bool chronicle(SDL_Renderer *r,Shell *s,SvLoginRect area,float scale,const SvEndpointView *input)
{
    const SvChronicleSnapshot *snapshot=input?input->chronicle:NULL;
    float width=area.w-28,heights[SV_CHRONICLE_EVENTS],total=0;
    SDL_Color muted={133,141,131,255};
    size_t count=snapshot?snapshot->count:0;
    for(size_t i=0;i<count;++i) {
        float height;
        if(!news_text(r,s,snapshot->events[i].text,0,0,width,scale,muted,false,&height))return false;
        heights[i]=height+43;total+=heights[i];
    }
    SDL_Rect clip={(int)(area.x*scale),(int)(area.y*scale),(int)(area.w*scale),(int)(area.h*scale)};
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    bool ok=true;
    float y=area.y+19-SDL_clamp(input?input->chronicle_scroll:0,0,1)*SDL_max(0,total-area.h+31);
    static const char *const regions[]={"EU","NA","APAC"};
    for(size_t i=0;ok && i<count;++i) {
        const SvChronicleEvent *event=&snapshot->events[i];
        if(y+heights[i]>area.y && y<area.y+area.h) {
            float ignored;unsigned rgb=event->color;
            SDL_Color color={(Uint8)(((rgb>>16)&255)*.78f+255*.22f),
                             (Uint8)(((rgb>>8)&255)*.78f+255*.22f),
                             (Uint8)((rgb&255)*.78f+255*.22f),255};
            ok=(i==0 || box(r,(SvLoginRect){area.x+12,y-11,area.w-24,1},scale,(SDL_Color){78,83,69,82},false)) &&
               box(r,(SvLoginRect){area.x+14,y,31,17},scale,(SDL_Color){64,82,68,255},true) &&
               text(r,s,regions[event->server<3?event->server:0],area.x+18,y+3,10,scale,(SDL_Color){169,213,177,255}) &&
               text(r,s,event->date_label,area.x+52,y+3,10,scale,(SDL_Color){174,180,172,255}) &&
               news_text(r,s,event->text,area.x+14,y+24,width,scale,color,true,&ignored);
        }
        y+=heights[i];
    }
    if(!count) {
        const char *message=snapshot && snapshot->loading?"Loading World Chronicle...":
            snapshot && !snapshot->failed_sources?"No recent events.":"World Chronicle unavailable.";
        float ignored;
        ok=news_text(r,s,message,area.x+16,area.y+32,width,scale,muted,true,&ignored);
    } else if(snapshot->failed_sources || snapshot->loading) {
        float ignored;
        ok=ok && news_text(r,s,snapshot->loading?"Updating other sources...":"Some sources unavailable.",
                 area.x+14,y+10,width,scale,muted,true,&ignored);
    }
    return SDL_SetRenderClipRect(r,NULL) && ok;
}

bool sv_login_shell_render(SDL_Renderer *r,const SvEndpoint *endpoint,const SvEndpointView *input)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if (!s) return SDL_SetError("B2 shell resources have not been initialized");
    Uint64 now=SDL_GetTicks(),delta=now-s->last;s->last=now;
    SDL_Window *window=SDL_GetRenderWindow(r);
    if (SDL_GetWindowFlags(window)&(SDL_WINDOW_HIDDEN|SDL_WINDOW_MINIMIZED))return true;
    if (!s->reduced) s->elapsed+=delta;
    s->frame=s->elapsed/100;
    for(int layer=0;layer<2;++layer)if(s->background[layer]) {
        float tw,th;
        if(!SDL_GetTextureSize(s->background[layer],&tw,&th))return false;
        int lower,count;background_kernel(r,(int)tw,(int)th,&lower,&count);
        if(lower!=s->background_lower[layer] || count!=s->background_count[layer]) {
            SDL_Texture *next=image(r,s,layer?s->previous:s->current,layer);
            if(!next)return false;
            SDL_DestroyTexture(s->background[layer]);s->background[layer]=next;
        }
    }
    if (!s->reduced && s->elapsed-s->rotation>=30000) {
        int lower=s->background_lower[0],count=s->background_count[0];
        SDL_Texture *next=image(r,s,(s->current+1)%19,0);
        if (!next) return false;
        SDL_DestroyTexture(s->background[1]);s->background[1]=s->background[0];
        s->background_lower[1]=lower;s->background_count[1]=count;
        s->background[0]=next;s->previous=s->current;s->current=(s->current+1)%19;s->rotation=s->elapsed;
    }
    int w,h;if (!SDL_GetCurrentRenderOutputSize(r,&w,&h))return false;
    float scale=sv_login_ui_scale(window);if(scale<=0)scale=1;
    SvLoginLayout l=sv_login_server_layout(w/scale,h/scale,endpoint);
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
    SDL_Color muted={133,141,131,255};
    bool name_entry=input && input->login_view && (input->login_view->ordinary_name_entry || input->birth_view);
    if(name_entry)l=sv_character_shell_layout(w/scale,h/scale,1);
    if(input && input->birth_view)l.panel=sv_birth_layout(w/scale,h/scale,input->birth_view->owner).panel;
    SDL_Color surface=input && (input->credentials || (input->login_view && input->login_view->overview && !name_entry))?(SDL_Color){14,18,14,245}:
                                                  (SDL_Color){12,16,13,245};
    if(!background_veil(r,s,w,h) ||
       !logo(r,s,l.logo,scale) ||
       !roundbox(r,l.panel,3,scale,surface,false) ||
       !roundbox(r,l.panel,3,scale,(SDL_Color){130,111,62,133},true))return false;
    if(!l.compact && !name_entry) {
        if(!box(r,l.chronicle,scale,(SDL_Color){4,8,5,51},false) ||
           !chronicle(r,s,l.chronicle,scale,input))return false;
    }
    static const char *labels[]={"TomeNET website","Discord chat","IRC channel","Official Guide","Tangar's Ru Guide","Github"};
    for(int i=0;!name_entry && i<6;++i) {
        bool hover=input && input->hover_link==(size_t)i && endpoint->phase!=SV_ENDPOINT_MANUAL;
        SvLoginRect bounds=sv_login_link_rect(l,(size_t)i);
        if(hover)bounds.x+=2;
        SDL_FRect icon={bounds.x*scale,bounds.y*scale,bounds.w*scale,bounds.h*scale};
        if(!SDL_RenderTexture(r,s->icons[i][hover?1:0],NULL,&icon))return false;
        if(!l.compact && !text(r,s,labels[i],bounds.x+59,bounds.y+15,10,scale,
                              hover?(SDL_Color){230,220,191,255}:(SDL_Color){170,169,151,255}))return false;
    }
    if(endpoint->phase == SV_ENDPOINT_LIST || endpoint->phase == SV_ENDPOINT_MANUAL) {
    if(!server_shortcuts(r,s,l,scale,endpoint->server_count,input && (input->hover_manual || input->list_focus_manual)))return false;
    /* Rows reach both horizontal list edges. Round outward and retain the
     * one-output-pixel antialias fringe instead of clipping their borders. */
    int clip_left=(int)SDL_floorf(l.list.x*scale)-1;
    int clip_right=(int)SDL_ceilf((l.list.x+l.list.w)*scale)+1;
    SDL_Rect clip={clip_left,(int)(l.list.y*scale),clip_right-clip_left,(int)(l.list.h*scale)};
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    bool ok=true;
    if(!endpoint->server_count)ok=text(r,s,"No server list available.",l.list.x+12,l.list.y+14,14,scale,muted);
    for(size_t i=l.first;ok && i<endpoint->server_count && i<l.first+l.rows;++i) {
        SvLoginRect row=sv_login_row(l,i),door=sv_login_server_door(row);
        const SvServer *server=&endpoint->servers[i];
        bool selected=i==endpoint->selected;
        bool hover=input && input->hover==i;
        bool door_hover=hover && input->hover_enter;
        if(l.group_headers & (UINT32_C(1)<<i)) {
            bool official=sv_login_server_official(server);
            ok=text(r,s,official?"OFFICIAL SERVERS":"UNOFFICIAL SERVERS",row.x,row.y-30,11,scale,
                        official?(SDL_Color){200,169,75,255}:muted) &&
               box(r,(SvLoginRect){row.x,row.y-10,row.w,1},scale,(SDL_Color){49,43,27,255},false);
            if(!ok)break;
        }
        char address[128],health[48],badge[4],detail_text[128],count_text[48];
        SDL_snprintf(badge,sizeof(badge),"%c",(int)('a'+i));
        if(server->ping_ms>=0)SDL_snprintf(health,sizeof(health),"%d ms",server->ping_ms);
        else SDL_strlcpy(health,server->ping_ms==-1?"ping pending":"ping unavailable",sizeof(health));
        if(server->metadata_available)
            SDL_snprintf(count_text,sizeof(count_text),"%u %s",server->player_count,
                         server->player_count==1?"player":"players");
        else SDL_strlcpy(count_text,"players unavailable",sizeof(count_text));
        float address_width,health_width,players_width,region_width;
        if(!text_width(s,health,12,scale,false,&health_width) ||
           !text_width(s,count_text,10,scale,false,&players_width)) {ok=false;break;}
        float health_right=row.x+row.w-65;
        float health_left=health_right-SDL_max(health_width,players_width);
        float address_budget=health_left-(row.x+62)-12;
        int host_chars=(int)SDL_strlen(server->host);
        do {
            SDL_snprintf(address,sizeof(address),"%.*s:%u",host_chars,server->host,server->port);
            if(!text_width(s,address,14,scale,true,&address_width)) {ok=false;break;}
        } while(address_width>address_budget && --host_chars>0);
        if(!ok)break;
        const char *detail=strcmp(server->label,server->host)?server->label:"Version unavailable";
        if(*server->version)SDL_snprintf(detail_text,sizeof(detail_text),"%s %s",
            *server->game?server->game:"TomeNET",server->version);
        else SDL_strlcpy(detail_text,detail,sizeof(detail_text));
        float detail_width;
        for(;;) {
            if(!text_width(s,detail_text,11,scale,false,&detail_width)) {ok=false;break;}
            if(detail_width<=address_budget || !*detail_text)break;
            size_t n=SDL_strlen(detail_text)-1;
            while(n && ((unsigned char)detail_text[n]&0xc0)==0x80)--n;
            detail_text[n]=0;
        }
        if(!ok)break;
        char region_text[160];
        const char *region=!strcmp(server->host,"europe.tomenet.eu")?"(Europe)":
            !strcmp(server->host,"na.tomenet.eu")?"(North America)":
            !strcmp(server->host,"apac.tomenet.eu")?"(Asia-Pacific)":"";
        if(*server->notes) {
            SDL_snprintf(region_text,sizeof(region_text),"(%s)",server->notes);
            region=region_text;
        }
        bool show_players=server->metadata_available && server->player_count && *server->players;
        float main_y=row.y+(row.h-(show_players?64:48))/2;
        ok=roundbox(r,row,8,scale,selected?(SDL_Color){78,61,18,102}:
                         hover?(SDL_Color){12,16,13,255}:(SDL_Color){7,11,8,194},false) &&
           roundbox(r,row,8,scale,selected?(SDL_Color){230,192,79,255}:
                         hover?(SDL_Color){114,99,58,255}:(SDL_Color){49,53,45,255},true);
        ok=ok && box(r,(SvLoginRect){row.x+14,main_y+4,36,30},scale,(SDL_Color){58,66,56,255},true) &&
           text_style(r,s,badge,row.x+29,main_y+13,10,scale,(SDL_Color){217,189,97,255},true) &&
           text_style(r,s,address,row.x+62,main_y+3,14,scale,(SDL_Color){229,226,213,255},true) &&
           text(r,s,detail_text,row.x+62,main_y+25,11,scale,(SDL_Color){111,117,110,255}) &&
           text(r,s,health,health_right-health_width,main_y+3,12,scale,
                    server->ping_ms>=0?(SDL_Color){74,221,90,255}:muted) &&
           text(r,s,count_text,health_right-players_width,main_y+25,10,scale,(SDL_Color){119,124,117,255});
        if(ok && show_players) {
            char names[sizeof(server->players)+10];float names_width;
            SDL_snprintf(names,sizeof(names),"Players: %s",server->players);
            for(;;) {
                if(!text_width(s,names,10,scale,false,&names_width)) {ok=false;break;}
                if(names_width<=row.w-127 || !*names)break;
                size_t n=SDL_strlen(names)-1;
                while(n && ((unsigned char)names[n]&0xc0)==0x80)--n;
                names[n]=0;
            }
            if(ok)ok=text(r,s,names,row.x+62,main_y+47,10,scale,(SDL_Color){104,174,186,255});
        }
        float note_x=row.x+62+address_width+8;
        if(ok && *region) {
            ok=text_width(s,region,12,scale,false,&region_width);
            if(ok && note_x+region_width<=health_left-12)
                ok=text(r,s,region,note_x,main_y+4,12,scale,muted);
        }
        ok=ok && roundbox(r,door,5,scale,door_hover?(SDL_Color){41,34,13,255}:(SDL_Color){11,14,10,235},false) &&
           roundbox(r,door,5,scale,door_hover?(SDL_Color){211,174,63,255}:
                            selected?(SDL_Color){169,138,50,255}:(SDL_Color){92,81,46,255},true);
        SDL_FRect icon={(door.x+3.5f)*scale,(door.y+3.5f)*scale,27*scale,27*scale};
        ok=ok && SDL_RenderTexture(r,s->doors[door_hover?1:0],NULL,&icon);
    }
    if(!SDL_SetRenderClipRect(r,NULL) || !ok)return false;

    }
    return true;
}

/* C has its own whole-window composition. Prepare glyphs at the final output
 * metrics: the complete fixed-cell scene is presented 1:1 without steady-state
 * texture scaling. Markers never occupy columns; empty rows retain height. */
bool sv_login_shell_motd(SDL_Renderer *r,const SvMotd *motd)
{
    Shell *s=SDL_GetPointerProperty(SDL_GetRendererProperties(r),SHELL_PROPERTY,NULL);
    if(!s || !SDL_SetRenderDrawColor(r,0,0,0,255) || !SDL_RenderClear(r))return false;
    if(!motd->columns)return true;
    /* Canonical default client_color_map, independent of shell D palette. */
    static const SDL_Color colors[]={
        {0,0,0,255},{255,255,255,255},{157,157,157,255},{255,141,0,255},
        {183,0,0,255},{0,157,68,255},{0,0,255,255},{141,102,0,255},
        {102,102,102,255},{205,205,205,255},{175,0,255,255},{255,255,0,255},
        {255,48,48,255},{0,255,0,255},{0,255,255,255},{199,157,85,255}};
    int w,h;
    if(!SDL_GetCurrentRenderOutputSize(r,&w,&h))return false;
    float dpi=sv_login_ui_scale(SDL_GetRenderWindow(r));if(dpi<=0)dpi=1;
    bool animated=false;
    for(size_t row=0;row<SV_MOTD_ROWS;++row)for(size_t col=0;col<motd->length[row];++col) {
        unsigned role=motd->colors[row][col];
        if(role>=16 && (!s->reduced || role==56 || role==57 || role==58 || role==59 || role==61))animated=true;
    }
    uint32_t tick=animated?(uint32_t)(SDL_GetTicks()/100):0;
    if(s->motd_valid && s->motd_width==w && s->motd_height==h && s->motd_dpi==dpi &&
       s->motd_tick==tick && !memcmp(&s->motd,motd,sizeof(*motd))) {
        SDL_FRect dest={(w-s->motd_raster_width)/2.f,(h-s->motd_raster_height)/2.f,
                        (float)s->motd_raster_width,(float)s->motd_raster_height};
        return SDL_RenderTexture(r,s->motd_texture,NULL,&dest);
    }
    float available_w=w-48*dpi,available_h=h-48*dpi;
    if(available_w<=0 || available_h<=0)return SDL_SetError("Invalid MOTD geometry");
    /* Search final font metrics rather than assuming integer glyph sizes are
     * a continuous multiple of a base raster. Both axes constrain the fit. */
    float low=.1f,high=available_h;int cell=0,height=0;
    for(unsigned step=0;step<24;++step) {
        float points=(low+high)/2;int cw,ch;
        if(!TTF_SetFontSize(s->font,points) || !TTF_GetStringSize(s->font,"M",1,&cw,&ch))return false;
        if(cw>0 && ch>0 && cw*motd->columns<=available_w && ch*SV_MOTD_ROWS<=available_h)low=points;
        else high=points;
    }
    if(!TTF_SetFontSize(s->font,low) || !TTF_GetStringSize(s->font,"M",1,&cell,&height))return false;
    SDL_Surface *scene=SDL_CreateSurface((int)motd->columns*cell,SV_MOTD_ROWS*height,SDL_PIXELFORMAT_RGBA32);
    if(!scene)return false;
    bool ok=SDL_ClearSurface(scene,0,0,0,1);
    for(size_t row=0;ok && row<SV_MOTD_ROWS;++row)for(size_t start=0;ok && start<motd->length[row];) {
        size_t end=start+1;
        while(motd->glyphs[row][start]!=2 && motd->colors[row][start]<16 && end<motd->length[row] &&
              motd->glyphs[row][end]!=2 && motd->colors[row][end]==motd->colors[row][start])++end;
        SDL_Color color=colors[sv_motd_color(motd->colors[row][start],tick,(uint32_t)(row*120+start+1),s->reduced)];
        if(motd->glyphs[row][start]==2) {
            /* FONT_MAP_SOLID_X11 is a filled cell, not a Unicode control. */
            SDL_Rect solid={(int)start*cell,(int)row*height,cell,height};
            Uint32 pixel=SDL_MapSurfaceRGBA(scene,color.r,color.g,color.b,255);
            ok=SDL_FillSurfaceRect(scene,&solid,pixel);start=end;continue;
        }
        /* Numeric byte profile for non-ASCII text; ASCII remains unchanged.
         * The vein byte has its explicit diamond identity. Unassigned control
         * bytes retain one visible replacement cell, never UTF-8 structure. */
        char text[SV_MOTD_STRIDE*3+1];size_t at=0;
        for(size_t col=start;col<end;++col) {
            unsigned char c=motd->glyphs[row][col];
            if(c==1) {text[at++]=(char)0xe2;text[at++]=(char)0x97;text[at++]=(char)0x86;}
            else if(c<32 || c==127 || (c>=128 && c<160))text[at++]='?';
            else if(c<128)text[at++]=(char)c;
            else {text[at++]=(char)(0xc0|(c>>6));text[at++]=(char)(0x80|(c&63));}
        }
        text[at]=0;
        SDL_Surface *run=TTF_RenderText_Blended(s->font,text,at,color);
        SDL_Rect dest={(int)start*cell,(int)row*height,0,0};
        ok=run && SDL_BlitSurface(run,NULL,scene,&dest);SDL_DestroySurface(run);
        start=end;
    }
    SDL_Texture *texture=ok?SDL_CreateTextureFromSurface(r,scene):NULL;
    if(texture) {
        SDL_FRect dest={(w-scene->w)/2.f,(h-scene->h)/2.f,(float)scene->w,(float)scene->h};
        ok=SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST) && SDL_RenderTexture(r,texture,NULL,&dest);
    } else ok=false;
    if(ok) {
        SDL_DestroyTexture(s->motd_texture);s->motd_texture=texture;texture=NULL;
        s->motd=*motd;s->motd_width=w;s->motd_height=h;s->motd_dpi=dpi;s->motd_tick=tick;
        s->motd_raster_width=scene->w;s->motd_raster_height=scene->h;s->motd_valid=true;
    }
    SDL_DestroyTexture(texture);SDL_DestroySurface(scene);return ok;
}
