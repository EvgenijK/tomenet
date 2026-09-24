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

static bool pcf_name(const char *name)
{
    size_t length = strlen(name);
    return length >= 4 && !SDL_strcasecmp(name + length - 4, ".pcf");
}

SvFont *sv_font_open_requested(const char *root, const char *library,
                               const char *requested)
{
    SvFont *font = SDL_calloc(1, sizeof(*font));
    const char *roots[] = {root, library};
    const char *candidates[] = {requested, "CascadiaMono-Regular.ttf"};
    if (!font) return NULL;
    for (unsigned candidate = 0; candidate < 2; ++candidate) {
        if (candidate && !strcmp(candidates[0], candidates[1])) continue;
        for (unsigned i = candidate ? 1 : 0; i < 2; ++i) {
            if (SDL_snprintf(font->resource, sizeof(font->resource),
                             "%s/xtra/font/%s", roots[i], candidates[candidate]) >=
                (int)sizeof(font->resource)) continue;
            if (pcf_name(candidates[candidate])) {
                if (open_pcf(font)) return font;
                fprintf(stderr, "SV resource unavailable: %s (invalid PCF); trying declared fallback\n",
                        font->resource);
                continue;
            }
            font->ttf = TTF_OpenFont(font->resource, 18);
            if (font->ttf && ascii_ttf(font->ttf)) {
                font->size = 18;
                return font;
            }
            if (font->ttf) TTF_CloseFont(font->ttf);
            font->ttf = NULL;
            fprintf(stderr, "SV resource unavailable: %s (%s); trying declared fallback\n",
                    font->resource, SDL_GetError());
        }
    }
    /* PCF is loaded directly by FreeType; numeric encoding is not reinterpreted. */
    if (SDL_snprintf(font->resource, sizeof(font->resource),
                     "%s/xtra/font/16x24x.pcf", library) < (int)sizeof(font->resource) &&
        open_pcf(font)) {
        fprintf(stderr, "SV effective font fallback: %s (requested %s retained)\n", font->resource, requested);
        return font;
    }
    fprintf(stderr, "SV fatal resource failure: bundled Cascadia Mono and %s unavailable or invalid\n", font->resource);
    sv_font_close(font);
    return NULL;
}

SvFont *sv_font_open(const char *root, const char *library)
{
    return sv_font_open_requested(root, library, "CascadiaMono-Regular.ttf");
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

bool sv_font_draw(SvFont *font, SDL_Renderer *renderer, const char *text,
                  int x, int y, float scale, SDL_Color color)
{
    /* Shell strings are deliberately confined to the declared UI profile. */
    for (const unsigned char *p = (const unsigned char *)text; *p; ++p)
        if (*p < 32 || *p > 126) return SDL_SetError("Unsupported shell UI profile character");
    if (font->ttf) {
        float size = 18 * scale;
        if (size != font->size && !TTF_SetFontSize(font->ttf, size)) return false;
        font->size = size;
        return draw_surface(renderer, TTF_RenderText_Blended(font->ttf, text, 0, color), x, y);
    }
    int source_w = font->pcf->available_sizes[0].width;
    int source_h = font->pcf->available_sizes[0].height;
    int cell_w = (int)SDL_roundf(source_w * scale);
    int cell_h = (int)SDL_roundf(source_h * scale);
    int ascent = (int)(font->pcf->size->metrics.ascender >> 6);
    size_t len = strlen(text);
    if (!len || len > 256 || cell_w < 1 || cell_h < 1) return SDL_SetError("Invalid shell text geometry");
    SDL_Surface *surface = SDL_CreateSurface((int)len * cell_w, cell_h, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return false;
    SDL_ClearSurface(surface, 0, 0, 0, 0);
    for (size_t i = 0; i < len; ++i) {
        if (FT_Load_Char(font->pcf, (unsigned char)text[i], FT_LOAD_RENDER | FT_LOAD_MONOCHROME)) {
            SDL_DestroySurface(surface); return SDL_SetError("PCF glyph load failed");
        }
        FT_GlyphSlot glyph = font->pcf->glyph;
        FT_Bitmap *bitmap = &glyph->bitmap;
        if (bitmap->pixel_mode != FT_PIXEL_MODE_MONO) {
            SDL_DestroySurface(surface); return SDL_SetError("PCF glyph is not monochrome");
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
    return draw_surface(renderer, surface, x, y);
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
