#ifndef SV_RESOURCE_PREPARATION_H
#define SV_RESOURCE_PREPARATION_H

#include <SDL3/SDL.h>
#include <stdint.h>
#include "files/files.h"

/* Selections are independent for graphics and PCF. CPU preparation cannot
 * implement SDL renderer PixelArt, and therefore reports an explicit fallback. */
typedef enum {
    SV_RESOURCE_NEAREST,
    SV_RESOURCE_LINEAR,
    SV_RESOURCE_PIXELART
} SvResourceFilter;

typedef enum {
    SV_RESOURCE_OK,
    SV_RESOURCE_INVALID,
    SV_RESOURCE_MEMORY,
    SV_RESOURCE_IO
} SvResourceResult;

typedef struct {
    bool background, foreground, outline;
    uint32_t background_rgb, foreground_rgb, outline_rgb; /* 0xRRGGBB */
} SvResourceMasks;

typedef struct {
    uint64_t source_generation, profile_generation;
    SvResourceFilter filter;
    int outline_radius; /* final-pixel radius; 0 off, -1 preserve authored */
} SvResourcePreparation;

typedef struct {
    SDL_Surface *authored, *recolour, *outline;
    SDL_Rect source_rect;
    SvResourceFilter requested_filter, effective_filter;
    bool pixelart_fallback;
    uint64_t generation;
} SvPreparedResource;

SvResourceFilter sv_resource_filter(const char *name, SvResourceFilter fallback);
const char *sv_resource_filter_name(SvResourceFilter filter);
SvResourceMasks sv_resource_default_masks(bool dual_mask);
typedef struct {
    unsigned files, validated, failed, png, bmp, subsets;
    unsigned prepared_tiles, prepared_raw, missing_associations, partial_preferences;
    char last_error[SV_FILE_PATH];
} SvGraphicsInventory;
/* Concrete owned installation/user root, without overlay fallback. Enumerates
 * xtra/graphics, resolves base/subset PRF mask associations with SvPreferences,
 * and validates one nonempty native tile plus a raw rectangle for every sheet.
 * Counters distinguish discovery from actual successful production preparation. */
SvResourceResult sv_resource_inventory_graphics(const char *root, SvFileOwner owner,
    bool dual_mask, SvGraphicsInventory *inventory);
/* Zero-initialize out before its first use. Success replaces its old generation;
 * failure leaves it unchanged. Each call isolates one tile/rectangle/glyph, so
 * sampling and outlines cannot reach adjacent cells in an atlas. */
SvResourceResult sv_resource_prepare_tile(SDL_Surface *source,
    const SDL_Rect *rectangle, int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *preparation, SvPreparedResource *out);
SvResourceResult sv_resource_prepare_raw(SDL_Surface *source,
    const SDL_Rect *rectangle, int width, int height,
    const SvResourcePreparation *preparation, SvPreparedResource *out);
SvResourceResult sv_resource_prepare_coverage(SDL_Surface *source,
    int width, int height, SvResourceFilter filter, SvPreparedResource *out);
/* Decode/load is caller-owned when it has an existing image provider. The PNG/BMP
 * entry points are also a production resource validator; missing/invalid input
 * cannot publish an empty success. Paths must already be resolved by C003. */
SDL_Surface *sv_resource_load_image(const char *resolved_path, SvResourceResult *result);
SvResourceResult sv_resource_prepare_image(const char *resolved_path,
    const SDL_Rect *rectangle, int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *preparation, SvPreparedResource *out);
SvResourceResult sv_resource_prepare_bmp(const char *resolved_path,
    const SDL_Rect *rectangle, int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *preparation, SvPreparedResource *out);
void sv_resource_prepared_dispose(SvPreparedResource *prepared);
/* Runtime palette tint is applied after preparation without changing generation.
 * The returned surface is final-sized and owned by the caller. */
SDL_Surface *sv_resource_composite(const SvPreparedResource *prepared,
    SDL_Color foreground, SDL_Color outline);

#endif
