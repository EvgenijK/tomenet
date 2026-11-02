#ifndef SV_STUB_PLAY_PROFILE_H
#define SV_STUB_PLAY_PROFILE_H

#include <stddef.h>
#include <string.h>

/* Temporary profile/resource substitutes until the client profile workflows
 * supply player options, map viewport and selected audio/graphics resources.
 * These stubs remain part of the production login path. */
enum {
    SV_STUB_PLAY_AUDIO_SFX = 0,
    SV_STUB_PLAY_AUDIO_MUSIC = 0,
    SV_STUB_PLAY_USE_GRAPHICS = 0
};
static const char sv_stub_play_graphics[] = "NO_GRAPHICS";

/* Fixed 66 x 22 map viewport, encoded as two network-order 32-bit values. */
static const unsigned char sv_stub_play_geometry[] = {0, 0, 0, 66, 0, 0, 0, 22};

/* Baseline: c-tables.c option_info, initialize_main_pref_files and shipped
 * lib/user/options.prf (199 slots); no player preference loading yet. */
static const unsigned char sv_stub_play_options[199] = {
    0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1,
    1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1,
    1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0,
    0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* No selected font/tileset mappings: zero attribute/character pairs retain
 * authoritative server identity, as baseline Client_setup initialization does. */
static inline void sv_stub_play_visuals(unsigned char *out, size_t size)
{
    memset(out, 0, size);
}

#endif
