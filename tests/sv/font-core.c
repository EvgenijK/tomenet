#define _POSIX_C_SOURCE 200809L
#include "ui/font.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const SDL_Color white={255,255,255,255};

static void report(void *context,const SvFilePath *path,const SvFontMetrics *metrics,
                   bool valid,unsigned checked)
{
    (void)context;
    if (!valid) fprintf(stderr,"INVALID %s: %s\n",path->path,SDL_GetError());
    assert(valid && metrics->glyph_count && checked);
}

int main(int argc,char **argv)
{
    assert(argc<=2);
    const char *library=argc==2?argv[1]:"lib";
    assert(SDL_Init(0) && TTF_Init());
    const char *names[]={"9x15.pcf","9x15tg.pcf","12x24.pcf","8x16.pcf",
                         "16x22.pcf","16x24tg.pcf","16x24x.pcf"};
    for (unsigned i=0;i<sizeof(names)/sizeof(*names);++i) {
        char path[4096];
        snprintf(path,sizeof(path),"%s/xtra/font/%s",library,names[i]);
        SvFont *font=sv_font_open_profile_file(path,"sv-pcf-encoded-v1");
        assert(font && sv_font_is_pcf(font));
        SvFontMetrics metrics;
        assert(sv_font_metrics(font,&metrics));
        assert(metrics.has_default);
        if (!strcmp(names[i],"12x24.pcf") || !strcmp(names[i],"8x16.pcf")) assert(metrics.first_id==1);
        unsigned checked;
        assert(sv_font_validate(font,&checked) && checked);
        SvFontGlyphInfo info;
        SDL_Surface *glyph=sv_font_render_glyph(font,255,13,21,white,&info);
        assert(glyph && glyph->w==13 && glyph->h==21 && info.requested_id==255);
        if (!info.present) assert(info.used_default && !info.replacement && info.rendered_id==metrics.default_id);
        SDL_DestroySurface(glyph);
        glyph=sv_font_render_glyph(font,2,3,4,white,&info);
        assert(glyph && ((unsigned char *)glyph->pixels)[3]==255);
        SDL_DestroySurface(glyph);
        assert(!sv_font_render_glyph(font,256,16,24,white,NULL));
        printf("%s range=%u..%u glyphs=%u default=%s%u cell=%dx%d checked=%u\n",
            names[i],metrics.first_id,metrics.last_id,metrics.glyph_count,
            metrics.has_default?"":"absent/",metrics.default_id,metrics.width,metrics.height,checked);
        sv_font_close(font);
    }
    char path[4096];
    snprintf(path,sizeof(path),"%s/xtra/font/CascadiaMono-Regular.ttf",library);
    SvFont *font=sv_font_open_profile_file(path,"sv-ttf-ascii-v1");
    assert(font && !sv_font_is_pcf(font));
    SDL_Surface *surface=sv_font_render_surface(font,"UI \xE2\x82\xAC",18,1,white,false);
    assert(surface); SDL_DestroySurface(surface); sv_font_close(font);
    char profile[]="/tmp/sv-font-profile-XXXXXX";
    int fd=mkstemp(profile); assert(fd>=0);
    FILE *stream=fdopen(fd,"wb"); assert(stream);
    assert(fputs("SVFONT1\n0 0x2500\n128 0x20ac\n255 0x2588\n",stream)>=0);
    assert(!fclose(stream));
    font=sv_font_open_profile_file(path,profile); assert(font);
    SvFontGlyphInfo info;
    surface=sv_font_render_glyph(font,0,11,19,white,&info);
    assert(surface && info.present && !info.used_default && info.requested_id==0);
    SDL_DestroySurface(surface);
    surface=sv_font_render_glyph(font,128,11,19,white,&info);
    assert(surface && info.present && !info.used_default);
    SDL_DestroySurface(surface); sv_font_close(font);
    stream=fopen(profile,"wb"); assert(stream);
    assert(fputs("SVFONT1\n128 0xd800\n",stream)>=0 && !fclose(stream));
    assert(!sv_font_open_profile_file(path,profile));
    unlink(profile);
    /* The same production glyph path handles a custom proportional face. */
    const char *proportional="/usr/share/fonts/Adwaita/AdwaitaSans-Regular.ttf";
    if (!access(proportional,R_OK)) {
        font=sv_font_open_profile_file(proportional,NULL); assert(font);
        surface=sv_font_render_glyph(font,'W',7,17,white,&info); assert(surface);
        SDL_DestroySurface(surface); sv_font_close(font);
    }
    SvFontInventory inventory;
    assert(sv_font_validate_directory(library,SV_FILE_BUNDLED,&inventory,report,NULL));
    assert(inventory.discovered && inventory.valid==inventory.discovered && !inventory.invalid);
    printf("bundled corpus valid=%u invalid=%u FON-deferred=%u\n",
        inventory.valid,inventory.invalid,inventory.deferred_fon);
    TTF_Quit(); SDL_Quit();
    return 0;
}
