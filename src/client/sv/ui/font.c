#include "files/files.h"
#include "ui/font.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BDF_H
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

struct SvFont {
    TTF_Font *ttf;
    FT_Library ft;
    FT_Face pcf;
    char resource[4096];
    float size;
    char profile[4096];
    uint32_t unicode[256];
    bool mapped[256];
    bool encoded_default;
    uint32_t default_id;
};

static uint32_t little32(const unsigned char *p)
{
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}

/* FreeType exposes PCF character lookup but not the encoding-table default.
 * Read only that metadata, leaving ranges, glyph slots and rasterization to FT.
 * The PCF directory is little endian; encoding values use its table byte order. */
static void pcf_default(SvFont *font)
{
    unsigned char header[8],entry[16],encoding[14];
    FILE *stream=fopen(font->resource,"rb");
    if (!stream) return;
    if (fread(header,1,sizeof(header),stream)!=sizeof(header) || little32(header)!=0x70636601) goto done;
    uint32_t tables=little32(header+4);
    if (tables>1024) goto done;
    for (uint32_t i=0;i<tables;++i) {
        if (fread(entry,1,sizeof(entry),stream)!=sizeof(entry)) break;
        if (little32(entry)!=32) continue; /* PCF_BDF_ENCODINGS */
        if (little32(entry+8)<sizeof(encoding) ||
            fseek(stream,(long)little32(entry+12),SEEK_SET) ||
            fread(encoding,1,sizeof(encoding),stream)!=sizeof(encoding)) break;
        bool big=(little32(encoding)&4)!=0;
        uint32_t id=big?((uint32_t)encoding[12]<<8|encoding[13]):
            ((uint32_t)encoding[13]<<8|encoding[12]);
        if (id!=0xffff && FT_Get_Char_Index(font->pcf,id)) {
            font->default_id=id; font->encoded_default=true;
        }
        break;
    }
done:
    fclose(stream);
}

static bool ascii_ttf(TTF_Font *font)
{
    for (unsigned c = 32; c < 127; ++c)
        if (!TTF_FontHasGlyph(font, c)) return false;
    return true;
}

static bool open_pcf(SvFont *font)
{
    if (!font->ft && FT_Init_FreeType(&font->ft)) return false;
    if (FT_New_Face(font->ft, font->resource, 0, &font->pcf)) return false;
    bool valid = font->pcf->num_charmaps == 1 && font->pcf->num_fixed_sizes > 0 &&
        !FT_Set_Charmap(font->pcf, font->pcf->charmaps[0]) &&
        !FT_Select_Size(font->pcf, 0);
    if (!valid) {
        FT_Done_Face(font->pcf);
        font->pcf = NULL;
    }
    if (valid) pcf_default(font);
    return valid;
}

static bool open_file(SvFont *font, const char *path)
{
    if (!path || SDL_strlcpy(font->resource,path,sizeof(font->resource))>=sizeof(font->resource)) return false;
    const char *suffix=SDL_strrchr(path,'.');
    if (suffix && !SDL_strcasecmp(suffix,".pcf")) return open_pcf(font);
    font->ttf=TTF_OpenFont(path,18);
    if (font->ttf && ascii_ttf(font->ttf)) { font->size=18; return true; }
    if (font->ttf) TTF_CloseFont(font->ttf);
    font->ttf=NULL;
    return false;
}

SvFont *sv_font_open_file(const char *path)
{
    return sv_font_open_profile_file(path,NULL);
}

SvFont *sv_font_open_selected(const char *root, const char *library, const char *requested)
{
    SvFont *font=SDL_calloc(1,sizeof(*font));
    char path[4096],visited[4][4096];
    unsigned visited_count=0;
    if (!font) return NULL;
    if (!requested || !*requested) requested="CascadiaMono-Regular.ttf";
    /* Requested overlay then bundle, followed only by declared bundled fallbacks.
     * The requested name remains in the settings owner, never in this effective object. */
    const char *roots[]={root,library,library,library};
    const char *names[]={requested,requested,"CascadiaMono-Regular.ttf","16x24x.pcf"};
    for (unsigned i=0;i<4;++i) {
        int length;
        if (i<2 && requested[0]=='/') {
            if (i==1) continue;
            length=SDL_snprintf(path,sizeof(path),"%s",requested);
        } else {
            char relative[4096];
            SvFilePath resolved;
            length=SDL_snprintf(relative,sizeof(relative),"xtra/font/%s",names[i]);
            if (length<0 || length>=(int)sizeof(relative) ||
                !sv_file_path(&resolved,roots[i],relative,i?SV_FILE_BUNDLED:SV_FILE_SHARED)) continue;
            length=SDL_strlcpy(path,resolved.path,sizeof(path));
        }
        if (length<0 || length>=(int)sizeof(path)) continue;
        bool duplicate=false;
        for (unsigned prior=0;prior<visited_count;++prior)
            if (!strcmp(path,visited[prior])) duplicate=true;
        if (duplicate) continue;
        SDL_strlcpy(visited[visited_count++],path,sizeof(visited[0]));
        if (open_file(font,path)) {
            SDL_strlcpy(font->profile,font->pcf?"sv-pcf-encoded-v1":"sv-ttf-ascii-v1",sizeof(font->profile));
            for (unsigned c=32;c<127;++c) { font->unicode[c]=c; font->mapped[c]=true; }
            if (i>=2) fprintf(stderr,"SV effective text font fallback: %s; requested selection retained\n",path);
            return font;
        }
        fprintf(stderr,"SV text font unavailable: %s; trying declared fallback\n",path);
    }
    fprintf(stderr,"SV fatal text resource failure: requested and bundled fallback fonts unavailable\n");
    sv_font_close(font);
    return NULL;
}

SvFont *sv_font_open(const char *root, const char *library)
{
    return sv_font_open_selected(root,library,"CascadiaMono-Regular.ttf");
}

void sv_font_close(SvFont *font)
{
    if (!font) return;
    if (font->ttf) TTF_CloseFont(font->ttf);
    if (font->pcf) FT_Done_Face(font->pcf);
    if (font->ft) FT_Done_FreeType(font->ft);
    SDL_free(font);
}

const char *sv_font_resource(const SvFont *font) { return font->resource; }

static bool draw_surface(SDL_Renderer *renderer, SDL_Surface *surface, int x, int y)
{
    SDL_Texture *texture;
    bool ok;
    if (!surface) return false;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) { SDL_DestroySurface(surface); return false; }
    SDL_FRect dest = {(float)x, (float)y, (float)surface->w, (float)surface->h};
    ok = SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST) && SDL_RenderTexture(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    return ok;
}

SDL_Surface *sv_font_render_surface(SvFont *font, const char *text, float points,
                                    float scale, SDL_Color color, bool bold)
{
    if (font->ttf) {
        float size = points * scale;
        if (size != font->size && !TTF_SetFontSize(font->ttf, size)) return NULL;
        font->size = size;
        TTF_SetFontStyle(font->ttf,bold?TTF_STYLE_BOLD:TTF_STYLE_NORMAL);
        SDL_Surface *surface=TTF_RenderText_Blended(font->ttf,text,0,color);
        TTF_SetFontStyle(font->ttf,TTF_STYLE_NORMAL);
        return surface;
    }
    /* PCF shell text has the existing ASCII profile. Public UTF-8 text outside
     * it gets one visible replacement per character, never a guessed charset. */
    char ascii[257];
    size_t ascii_length=0;
    for (const unsigned char *p=(const unsigned char *)text;*p;++p) {
        if ((*p&0xc0)==0x80) continue;
        if (ascii_length==sizeof(ascii)-1) { SDL_SetError("Shell text too long"); return NULL; }
        ascii[ascii_length++]=(*p>=32 && *p<127)?(char)*p:'?';
    }
    ascii[ascii_length]=0;
    text=ascii;
    scale *= points / 18.f;
    int source_w = font->pcf->available_sizes[0].width;
    int source_h = font->pcf->available_sizes[0].height;
    int cell_w = (int)SDL_roundf(source_w * scale);
    int cell_h = (int)SDL_roundf(source_h * scale);
    int ascent = (int)(font->pcf->size->metrics.ascender >> 6);
    size_t len = strlen(text);
    if (!len || len > 256 || cell_w < 1 || cell_h < 1) { SDL_SetError("Invalid shell text geometry"); return NULL; }
    SDL_Surface *surface = SDL_CreateSurface((int)len * cell_w, cell_h, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return NULL;
    SDL_ClearSurface(surface, 0, 0, 0, 0);
    for (size_t i = 0; i < len; ++i) {
        if (FT_Load_Char(font->pcf, (unsigned char)text[i], FT_LOAD_RENDER | FT_LOAD_MONOCHROME)) {
            SDL_DestroySurface(surface); SDL_SetError("PCF glyph load failed"); return NULL;
        }
        FT_GlyphSlot glyph = font->pcf->glyph;
        FT_Bitmap *bitmap = &glyph->bitmap;
        if (bitmap->pixel_mode != FT_PIXEL_MODE_MONO) {
            SDL_DestroySurface(surface); SDL_SetError("PCF glyph is not monochrome"); return NULL;
        }
        for (int dy = 0; dy < cell_h; ++dy) {
            int sy = dy * source_h / cell_h - (ascent - glyph->bitmap_top);
            if (sy < 0 || sy >= (int)bitmap->rows) continue;
            const unsigned char *row = bitmap->buffer + (bitmap->pitch >= 0 ? sy : (int)bitmap->rows - 1 - sy) * SDL_abs(bitmap->pitch);
            for (int dx = 0; dx < cell_w; ++dx) {
                int sx = dx * source_w / cell_w - glyph->bitmap_left;
                if (sx < 0 || sx >= (int)bitmap->width || !(row[sx / 8] & (0x80 >> (sx % 8)))) continue;
                unsigned char *out = (unsigned char *)surface->pixels + dy * surface->pitch + ((int)i * cell_w + dx) * 4;
                out[0] = color.r; out[1] = color.g; out[2] = color.b; out[3] = color.a;
            }
        }
    }
    return surface;
}

bool sv_font_draw(SvFont *font, SDL_Renderer *renderer, const char *text,
                  int x, int y, float scale, SDL_Color color)
{
    /* Editable field bytes keep the original explicit ASCII profile. */
    for (const unsigned char *p=(const unsigned char *)text;*p;++p)
        if (*p<32 || *p>126) return SDL_SetError("Unsupported shell UI profile character");
    return draw_surface(renderer,sv_font_render_surface(font,text,18,scale,color,false),x,y);
}

bool sv_font_measure(SvFont *font,const char *text,float points,float scale,int *width,int *height,bool bold)
{
    if (font->ttf) {
        float size=points*scale;
        if (size!=font->size && !TTF_SetFontSize(font->ttf,size)) return false;
        font->size=size;
        TTF_SetFontStyle(font->ttf,bold?TTF_STYLE_BOLD:TTF_STYLE_NORMAL);
        bool ok=TTF_GetStringSize(font->ttf,text,0,width,height);
        TTF_SetFontStyle(font->ttf,TTF_STYLE_NORMAL);
        return ok;
    }
    size_t characters=0;
    for (const unsigned char *p=(const unsigned char *)text;*p;++p)
        if ((*p&0xc0)!=0x80) characters++;
    *width=(int)characters*(int)SDL_roundf(font->pcf->available_sizes[0].width*scale*points/18.f);
    *height=(int)SDL_roundf(font->pcf->available_sizes[0].height*scale*points/18.f);
    return *width>=0 && *height>0;
}

int sv_font_cell_width(SvFont *font, float scale)
{
    if (!font->ttf) return (int)SDL_roundf(font->pcf->available_sizes[0].width * scale);
    int width, height;
    float size = 18 * scale;
    if (size != font->size && !TTF_SetFontSize(font->ttf, size)) return 0;
    font->size = size;
    if (!TTF_GetStringSize(font->ttf, "M", 1, &width, &height)) return 0;
    return width;
}

static bool scalar(uint32_t value)
{
    return value<=0x10ffff && !(value>=0xd800 && value<=0xdfff);
}

static bool load_profile(SvFont *font, const char *name)
{
    const char *builtin=font->pcf?"sv-pcf-encoded-v1":"sv-ttf-ascii-v1";
    for (unsigned c=32;c<127;++c) { font->unicode[c]=c; font->mapped[c]=true; }
    if (!name || !*name || !strcmp(name,builtin)) {
        SDL_strlcpy(font->profile,builtin,sizeof(font->profile));
        return true;
    }
    /* Native PCF IDs already are the resource's visual profile. A Unicode
     * correspondence cannot be silently substituted for its encoded charmap. */
    if (strlen(name)>=sizeof(font->profile))
        return SDL_SetError("Unsupported font profile for resource");
    FILE *stream=fopen(name,"rb");
    if (!stream) return SDL_SetError("Font profile unavailable: %s",name);
    char line[256];
    bool valid=fgets(line,sizeof(line),stream)!=NULL;
    const char *magic=font->pcf?"SVPCF1":"SVFONT1";
    if (valid) {
        line[strcspn(line,"\r\n")]=0;
        valid=!strcmp(line,magic);
    }
    bool seen[256]={false};
    while (valid && fgets(line,sizeof(line),stream)) {
        char *p=line,*end;
        while (*p==' ' || *p=='\t') ++p;
        if (*p=='#' || *p=='\n' || *p=='\r' || !*p) continue;
        if (font->pcf) { valid=false; break; }
        errno=0;
        unsigned long id=strtoul(p,&end,p[0]=='0' && (p[1]=='x' || p[1]=='X')?16:10);
        if (end==p || errno || id>255 || (*end!=' ' && *end!='\t')) { valid=false; break; }
        p=end; errno=0;
        while (*p==' ' || *p=='\t') ++p;
        unsigned long code=strtoul(p,&end,p[0]=='0' && (p[1]=='x' || p[1]=='X')?16:10);
        if (end==p || errno || code>0x10ffff || !scalar((uint32_t)code) || seen[id]) { valid=false; break; }
        while (*end==' ' || *end=='\t' || *end=='\r' || *end=='\n') ++end;
        if (*end && *end!='#') { valid=false; break; }
        if (id>=32 && id<127 && id!=code) { valid=false; break; }
        seen[id]=true; font->mapped[id]=true; font->unicode[id]=(uint32_t)code;
    }
    if (ferror(stream)) valid=false;
    fclose(stream);
    if (!valid) return SDL_SetError("Invalid font profile: %s",name);
    SDL_strlcpy(font->profile,name,sizeof(font->profile));
    return true;
}

SvFont *sv_font_open_profile_file(const char *path, const char *profile)
{
    SvFont *font=SDL_calloc(1,sizeof(*font));
    if (font && open_file(font,path) && load_profile(font,profile)) return font;
    sv_font_close(font);
    return NULL;
}

const char *sv_font_profile(const SvFont *font) { return font?font->profile:""; }
bool sv_font_is_pcf(const SvFont *font) { return font && font->pcf; }

bool sv_font_metrics(SvFont *font,SvFontMetrics *out)
{
    if (!font || !out) return SDL_SetError("Missing font/metrics");
    SvFontMetrics info={0};
    info.pcf=font->pcf!=NULL;
    if (font->pcf) {
        info.width=(int)(font->pcf->size->metrics.max_advance>>6);
        info.height=font->pcf->available_sizes[0].height;
        info.ascent=(int)(font->pcf->size->metrics.ascender>>6);
        FT_UInt index;
        FT_ULong id=FT_Get_First_Char(font->pcf,&index);
        if (index) info.first_id=(uint32_t)id;
        while (index) {
            info.last_id=(uint32_t)id;
            ++info.glyph_count;
            id=FT_Get_Next_Char(font->pcf,id,&index);
        }
        BDF_PropertyRec property;
        if (!FT_Get_BDF_Property(font->pcf,"DEFAULT_CHAR",&property)) {
            if (property.type==BDF_PROPERTY_TYPE_CARDINAL) {
                info.default_id=(uint32_t)property.u.cardinal; info.has_default=true;
            } else if (property.type==BDF_PROPERTY_TYPE_INTEGER && property.u.integer>=0) {
                info.default_id=(uint32_t)property.u.integer; info.has_default=true;
            }
            info.has_default=info.has_default && FT_Get_Char_Index(font->pcf,info.default_id)!=0;
        }
        if (!info.has_default && font->encoded_default) {
            info.default_id=font->default_id; info.has_default=true;
        }
    } else {
        int w,h;
        if (!TTF_SetFontSize(font->ttf,18) || !TTF_GetStringSize(font->ttf,"M",1,&w,&h)) return false;
        font->size=18;
        info.width=w; info.height=h; info.ascent=TTF_GetFontAscent(font->ttf);
        info.first_id=32; info.last_id=126;
        for (unsigned id=0;id<256;++id) if (font->mapped[id]) {
            if (id<info.first_id) info.first_id=id;
            if (id>info.last_id) info.last_id=id;
            if (TTF_FontHasGlyph(font->ttf,font->unicode[id])) ++info.glyph_count;
        }
        info.default_id='?'; info.has_default=TTF_FontHasGlyph(font->ttf,'?');
    }
    if (info.width<1 || info.height<1 || info.width>4096 || info.height>4096)
        return SDL_SetError("Invalid font cell metrics");
    *out=info;
    return true;
}

bool sv_font_has_glyph(SvFont *font,uint32_t id)
{
    if (!font || id>255) return false;
    if (id==2) return true;
    return font->pcf ? FT_Get_Char_Index(font->pcf,id)!=0 :
        font->mapped[id] && TTF_FontHasGlyph(font->ttf,font->unicode[id]);
}

static void pixel(SDL_Surface *surface,int x,int y,SDL_Color color)
{
    unsigned char *out=(unsigned char *)surface->pixels+y*surface->pitch+x*4;
    out[0]=color.r; out[1]=color.g; out[2]=color.b; out[3]=color.a;
}

SDL_Surface *sv_font_render_glyph(SvFont *font,uint32_t id,int width,int height,
                                 SDL_Color color,SvFontGlyphInfo *out)
{
    if (!font || id>255 || width<1 || height<1 || width>4096 || height>4096) {
        SDL_SetError("Invalid numeric font glyph or cell geometry"); return NULL;
    }
    SvFontGlyphInfo info={.requested_id=id,.rendered_id=id,.present=sv_font_has_glyph(font,id)};
    SvFontMetrics metrics;
    if (!sv_font_metrics(font,&metrics)) return NULL;
    if (!info.present) {
        info.used_default=metrics.has_default;
        info.replacement=!info.used_default;
        if (info.used_default) info.rendered_id=metrics.default_id;
    }
    SDL_Surface *cell=SDL_CreateSurface(width,height,SDL_PIXELFORMAT_RGBA32);
    if (!cell || !SDL_ClearSurface(cell,0,0,0,0)) { SDL_DestroySurface(cell); return NULL; }
    if (id==2) {
        for (int y=0;y<height;++y) for (int x=0;x<width;++x) pixel(cell,x,y,color);
    } else if (info.replacement) {
        /* A bounded visible marker; unknown numeric identity remains in info. */
        for (int y=0;y<height;++y) for (int x=0;x<width;++x)
            if (!x || !y || x==width-1 || y==height-1 || x==(y*width/height)) pixel(cell,x,y,color);
    } else if (font->pcf) {
        FT_UInt slot=FT_Get_Char_Index(font->pcf,info.rendered_id);
        if (!slot || FT_Load_Glyph(font->pcf,slot,FT_LOAD_RENDER|FT_LOAD_MONOCHROME)) {
            SDL_DestroySurface(cell); SDL_SetError("PCF encoded glyph load failed"); return NULL;
        }
        FT_GlyphSlot glyph=font->pcf->glyph;
        FT_Bitmap *bitmap=&glyph->bitmap;
        if (bitmap->pixel_mode!=FT_PIXEL_MODE_MONO) {
            SDL_DestroySurface(cell); SDL_SetError("Unsupported PCF bitmap mode"); return NULL;
        }
        for (int y=0;y<height;++y) {
            int sy=y*metrics.height/height-(metrics.ascent-glyph->bitmap_top);
            if (sy<0 || sy>=(int)bitmap->rows) continue;
            const unsigned char *row=bitmap->buffer+(bitmap->pitch>=0?sy:(int)bitmap->rows-1-sy)*SDL_abs(bitmap->pitch);
            for (int x=0;x<width;++x) {
                int sx=x*metrics.width/width-glyph->bitmap_left;
                if (sx>=0 && sx<(int)bitmap->width && (row[sx/8]&(0x80>>(sx%8)))) pixel(cell,x,y,color);
            }
        }
    } else {
        uint32_t code=info.used_default?'?':font->unicode[id];
        float points=(float)height;
        if (!TTF_SetFontSize(font->ttf,points)) { SDL_DestroySurface(cell); return NULL; }
        int actual=TTF_GetFontHeight(font->ttf);
        if (actual>height) points*=((float)height/actual);
        if (!TTF_SetFontSize(font->ttf,points)) { SDL_DestroySurface(cell); return NULL; }
        font->size=points;
        TTF_SetFontStyle(font->ttf,TTF_STYLE_NORMAL);
        SDL_Surface *glyph=TTF_RenderGlyph_Blended(font->ttf,code,color);
        if (!glyph) { SDL_DestroySurface(cell); return NULL; }
        SDL_Rect dest={(width-glyph->w)/2,(height-glyph->h)/2,glyph->w,glyph->h};
        bool ok=SDL_BlitSurface(glyph,NULL,cell,&dest);
        SDL_DestroySurface(glyph);
        if (!ok) { SDL_DestroySurface(cell); return NULL; }
    }
    if (out) *out=info;
    return cell;
}

bool sv_font_validate(SvFont *font,unsigned *checked)
{
    SvFontMetrics metrics;
    if (!sv_font_metrics(font,&metrics)) return false;
    unsigned count=0;
    for (unsigned id=0;id<256;++id) {
        bool present=font->pcf?FT_Get_Char_Index(font->pcf,id)!=0:
            font->mapped[id] && TTF_FontHasGlyph(font->ttf,font->unicode[id]);
        if (!present) continue;
        SDL_Surface *glyph=sv_font_render_glyph(font,id,metrics.width,metrics.height,
                                               (SDL_Color){255,255,255,255},NULL);
        if (!glyph) return false;
        SDL_DestroySurface(glyph); ++count;
    }
    if (checked) *checked=count;
    return count>0;
}

typedef struct {
    SvFilePath directory;
    SvFontInventory inventory;
    SvFontInventoryCallback callback;
    void *context;
} FontInventoryContext;

static SDL_EnumerationResult SDLCALL inventory_file(void *userdata,const char *directory,const char *name)
{
    (void)directory;
    FontInventoryContext *scan=userdata;
    const char *suffix=SDL_strrchr(name,'.');
    if (!suffix) return SDL_ENUM_CONTINUE;
    if (!SDL_strcasecmp(suffix,".fon")) { ++scan->inventory.deferred_fon; return SDL_ENUM_CONTINUE; }
    if (SDL_strcasecmp(suffix,".pcf") && SDL_strcasecmp(suffix,".ttf") && SDL_strcasecmp(suffix,".otf"))
        return SDL_ENUM_CONTINUE;
    SvFilePath path;
    if (!sv_file_path(&path,scan->directory.path,name,scan->directory.owner)) return SDL_ENUM_FAILURE;
    ++scan->inventory.discovered;
    SvFontMetrics metrics={0};
    unsigned checked=0;
    SvFont *font=sv_font_open_profile_file(path.path,NULL);
    bool valid=font && sv_font_metrics(font,&metrics) && sv_font_validate(font,&checked);
    if (valid) ++scan->inventory.valid; else ++scan->inventory.invalid;
    if (scan->callback) scan->callback(scan->context,&path,&metrics,valid,checked);
    sv_font_close(font);
    return SDL_ENUM_CONTINUE;
}

bool sv_font_validate_directory(const char *root,SvFileOwner owner,SvFontInventory *inventory,
    SvFontInventoryCallback callback,void *context)
{
    if (!inventory) return SDL_SetError("Missing font inventory output");
    FontInventoryContext scan={.callback=callback,.context=context};
    if (!sv_file_path(&scan.directory,root,"xtra/font",owner)) return false;
    bool ok=SDL_EnumerateDirectory(scan.directory.path,inventory_file,&scan);
    *inventory=scan.inventory;
    return ok;
}
