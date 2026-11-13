/* Focused production preparation cases; no test implementation of filtering. */
#include "ui/resource-preparation.h"

#include <assert.h>
#include <stdio.h>

static unsigned char *at(SDL_Surface *surface, int x, int y) {
    return (unsigned char *)surface->pixels + y * surface->pitch + x * 4;
}

static void put(SDL_Surface *surface, int x, int y, uint32_t rgb, unsigned char alpha) {
    unsigned char *p = at(surface, x, y);
    p[0] = (unsigned char)(rgb >> 16);
    p[1] = (unsigned char)(rgb >> 8);
    p[2] = (unsigned char)rgb;
    p[3] = alpha;
}

int main(void) {
    SDL_Surface *atlas = SDL_CreateSurface(6, 3, SDL_PIXELFORMAT_RGBA32);
    assert(atlas);
    SvResourceMasks masks = sv_resource_default_masks(true);
    for (int y = 0; y < atlas->h; y++)
        for (int x = 0; x < atlas->w; x++) put(atlas, x, y, masks.background_rgb, 255);
    put(atlas, 2, 1, masks.foreground_rgb, 255);
    SvResourcePreparation options = {42, 7, SV_RESOURCE_PIXELART, 1};
    SvPreparedResource tile = {0}, other = {0}, raw = {0};
    SDL_Rect first = {0, 0, 3, 3}, second = {3, 0, 3, 3};
    assert(sv_resource_prepare_tile(atlas, &first, 3, 3, &masks, &options, &tile) == SV_RESOURCE_OK);
    assert(tile.pixelart_fallback && tile.requested_filter == SV_RESOURCE_PIXELART);
    assert(tile.effective_filter == SV_RESOURCE_NEAREST && tile.generation);
    assert(at(tile.recolour, 2, 1)[3] == 255 && at(tile.authored, 2, 1)[3] == 0);
    assert(at(tile.outline, 1, 1)[3] == 255 && at(tile.outline, 0, 1)[3] == 0);
    assert(at(tile.outline, 2, 1)[3] == 0);
    assert(sv_resource_prepare_tile(atlas, &second, 3, 3, &masks, &options, &other) == SV_RESOURCE_OK);
    for (int y = 0; y < 3; y++) for (int x = 0; x < 3; x++)
        assert(at(other.authored, x, y)[3] == 0 && at(other.outline, x, y)[3] == 0);
    SDL_Surface *output = sv_resource_composite(&tile, (SDL_Color){10, 20, 30, 255},
                                               (SDL_Color){0, 0, 0, 221});
    assert(output && at(output, 2, 1)[0] == 10 && at(output, 2, 1)[1] == 20);
    SDL_DestroySurface(output);
    uint64_t generation = tile.generation;
    SDL_Rect invalid = {5, 0, 2, 2};
    assert(sv_resource_prepare_tile(atlas, &invalid, 3, 3, &masks, &options, &tile) == SV_RESOURCE_INVALID);
    assert(tile.generation == generation && at(tile.recolour, 2, 1)[3] == 255);
    options.filter = SV_RESOURCE_LINEAR;
    options.outline_radius = -1;
    put(atlas, 1, 1, 0xff8800, 255);
    put(atlas, 0, 0, masks.outline_rgb, 255);
    assert(sv_resource_prepare_tile(atlas, &first, 6, 6, &masks, &options, &tile) == SV_RESOURCE_OK);
    assert(!tile.pixelart_fallback && tile.effective_filter == SV_RESOURCE_LINEAR);
    assert(at(tile.recolour, 5, 3)[3] == 255 && at(tile.authored, 5, 3)[3] == 0);
    assert(at(tile.outline, 0, 0)[3] == 255);
    assert(sv_resource_prepare_raw(atlas, &first, 6, 6, &options, &raw) == SV_RESOURCE_OK);
    assert(!raw.recolour && !raw.outline && raw.source_rect.w == 3);
    assert(at(raw.authored, 0, 0)[3] == 255); /* raw keeps encoded RGB as authored */
    SvPreparedResource coverage = {0};
    assert(sv_resource_prepare_coverage(other.authored, 9, 6, SV_RESOURCE_NEAREST, &coverage) == SV_RESOURCE_OK);
    assert(coverage.authored->w == 9 && coverage.authored->h == 6);
    SDL_Surface *ramp = SDL_CreateSurface(2, 1, SDL_PIXELFORMAT_RGBA32);
    assert(ramp);
    put(ramp, 0, 0, 0xffffff, 0);
    put(ramp, 1, 0, 0xffffff, 255);
    assert(sv_resource_prepare_coverage(ramp, 3, 1, SV_RESOURCE_NEAREST, &coverage) == SV_RESOURCE_OK);
    unsigned char nearest = at(coverage.authored, 1, 0)[3];
    assert(sv_resource_prepare_coverage(ramp, 3, 1, SV_RESOURCE_LINEAR, &coverage) == SV_RESOURCE_OK);
    assert(at(coverage.authored, 1, 0)[3] != nearest);
    SDL_DestroySurface(ramp);
    SvResourceResult result;
    SDL_Surface *loaded = sv_resource_load_image("lib/xtra/graphics/16x24sv.bmp", &result);
    assert(loaded && result == SV_RESOURCE_OK);
    SDL_DestroySurface(loaded);
    loaded = sv_resource_load_image("lib/xtra/sv/login/realm-1.png", &result);
    assert(loaded && result == SV_RESOURCE_OK);
    SDL_DestroySurface(loaded);
    assert(!sv_resource_load_image("/no/such/sv-resource.png", &result) && result == SV_RESOURCE_IO);
    assert(!sv_resource_load_image("/dev/null", &result) && result == SV_RESOURCE_INVALID);
    SvGraphicsInventory inventory;
    SvResourceResult inventory_result = sv_resource_inventory_graphics("lib", SV_FILE_BUNDLED, true, &inventory);
    printf("SV bundled graphics: files=%u validated=%u failed=%u bmp=%u png=%u subsets=%u tiles=%u raw=%u missing-prf=%u partial-prf=%u last=%s\n",
        inventory.files, inventory.validated, inventory.failed, inventory.bmp, inventory.png,
        inventory.subsets, inventory.prepared_tiles, inventory.prepared_raw,
        inventory.missing_associations, inventory.partial_preferences, inventory.last_error);
    assert(inventory_result == SV_RESOURCE_OK && inventory.files > 0 && inventory.validated == inventory.files);
    assert(inventory.prepared_tiles == inventory.files && inventory.prepared_raw == inventory.files);
    sv_resource_prepared_dispose(&coverage);
    sv_resource_prepared_dispose(&tile);
    sv_resource_prepared_dispose(&other);
    sv_resource_prepared_dispose(&raw);
    SDL_DestroySurface(atlas);
    puts("SV resource preparation: masks, filters/fallback, bounds, retained generation, BMP/PNG PASS");
    return 0;
}
