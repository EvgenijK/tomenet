/* Evidence observer only: forwards the real public font query unchanged. */
#define _GNU_SOURCE
#include "ui/font.h"
#include <link.h>
#include <stdio.h>

SvFont *__real_sv_font_open(const char *root, const char *library_root);
const char *__real_sv_font_resource(const SvFont *font);
static int library(struct dl_phdr_info *info, size_t size, void *data)
{
    (void)size; (void)data;
    if (info->dlpi_name[0] == '/')
        fprintf(stderr, "SV sealed library: %s\n", info->dlpi_name);
    return 0;
}
const char *__wrap_sv_font_resource(const SvFont *font)
{
    const char *resource = __real_sv_font_resource(font);
    fprintf(stderr, "SV sealed effective font: %s\n", resource);
    dl_iterate_phdr(library, NULL);
    return resource;
}

SvFont *__wrap_sv_font_open(const char *root, const char *library_root)
{
    SvFont *font = __real_sv_font_open(root, library_root);
    if (font) {
        fprintf(stderr, "SV sealed loaded font: %s\n", __real_sv_font_resource(font));
        dl_iterate_phdr(library, NULL);
    }
    return font;
}
