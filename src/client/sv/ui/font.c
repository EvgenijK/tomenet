#include "files/files.h"
#include "ui/font.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <string.h>
#include <math.h>

struct SvFont {
    TTF_Font *ttf;
    FT_Library ft;
    FT_Face pcf;
    char resource[4096];
    float size;
};

static bool ascii_ttf(TTF_Font *font)
{
    for (unsigned c = 32; c < 127; ++c)
        if (!TTF_FontHasGlyph(font, c)) return false;
    return TTF_FontIsFixedWidth(font);
}

static bool open_pcf(SvFont *font)
{
    if (!font->ft && FT_Init_FreeType(&font->ft)) return false;
    if (FT_New_Face(font->ft, font->resource, 0, &font->pcf)) return false;
    bool valid = font->pcf->num_charmaps == 1 && font->pcf->num_fixed_sizes > 0 &&
        !FT_Set_Charmap(font->pcf, font->pcf->charmaps[0]) &&
        !FT_Select_Size(font->pcf, 0);
    for (unsigned c = 32; valid && c < 127; ++c)
        if (!FT_Get_Char_Index(font->pcf, c)) valid = false;
    if (!valid) {
        FT_Done_Face(font->pcf);
        font->pcf = NULL;
    }
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
    SvFont *font=SDL_calloc(1,sizeof(*font));
    if (font && open_file(font,path)) return font;
    sv_font_close(font);
    return NULL;
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
