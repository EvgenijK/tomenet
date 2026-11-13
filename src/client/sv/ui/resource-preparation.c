#include "ui/resource-preparation.h"

#include <string.h>
#include <limits.h>
#include <png.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include "settings/preferences.h"

SvResourceFilter sv_resource_filter(const char *name, SvResourceFilter fallback) {
    if (!name) return fallback;
    if (!strcmp(name, "nearest")) return SV_RESOURCE_NEAREST;
    if (!strcmp(name, "linear")) return SV_RESOURCE_LINEAR;
    if (!strcmp(name, "pixelart")) return SV_RESOURCE_PIXELART;
    return fallback;
}

const char *sv_resource_filter_name(SvResourceFilter filter) {
    switch (filter) {
    case SV_RESOURCE_NEAREST: return "nearest";
    case SV_RESOURCE_LINEAR: return "linear";
    case SV_RESOURCE_PIXELART: return "pixelart";
    }
    return "invalid";
}

SvResourceMasks sv_resource_default_masks(bool dual_mask) {
    SvResourceMasks result = {true, true, dual_mask,
        dual_mask ? 0x1d211c : 0x3e3d00, 0xfc00fb, 0x3e3d00};
    return result;
}

void sv_resource_prepared_dispose(SvPreparedResource *prepared) {
    if (!prepared) return;
    SDL_DestroySurface(prepared->authored);
    SDL_DestroySurface(prepared->recolour);
    SDL_DestroySurface(prepared->outline);
    memset(prepared, 0, sizeof(*prepared));
}

static unsigned char *pixel(SDL_Surface *surface, int x, int y) {
    return (unsigned char *)surface->pixels + y * surface->pitch + x * 4;
}

static uint64_t hash_bytes(uint64_t hash, const void *bytes, size_t size) {
    const unsigned char *p = bytes;
    for (size_t i = 0; i < size; i++) {
        hash ^= p[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static SDL_Surface *blank(int width, int height) {
    SDL_Surface *surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (surface) SDL_FillSurfaceRect(surface, NULL, 0);
    return surface;
}

static bool rect_valid(SDL_Surface *surface, const SDL_Rect *rect) {
    return rect->x >= 0 && rect->y >= 0 && rect->w > 0 && rect->h > 0 &&
        rect->x <= surface->w && rect->y <= surface->h &&
        rect->w <= surface->w - rect->x && rect->h <= surface->h - rect->y;
}

static SDL_Surface *scaled(SDL_Surface *surface, int width, int height,
                           SvResourceFilter filter) {
    SDL_Surface *result = SDL_ScaleSurface(surface, width, height,
        filter == SV_RESOURCE_LINEAR ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
    if (result) SDL_SetSurfaceBlendMode(result, SDL_BLENDMODE_BLEND);
    return result;
}

/* All samples are from an already isolated decoded cell. Outlines dilate its
 * silhouette at final geometry, and never read an adjacent atlas cell. */
static bool generate_outline(SvPreparedResource *prepared, int radius_x, int radius_y) {
    int width = prepared->authored->w, height = prepared->authored->h;
    SDL_Surface *generated = blank(width, height);
    if (!generated) return false;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (pixel(prepared->authored, x, y)[3] ||
                pixel(prepared->recolour, x, y)[3]) continue;
            bool covered = false;
            int top = SDL_max(0, y - radius_y), bottom = SDL_min(height - 1, y + radius_y);
            int left = SDL_max(0, x - radius_x), right = SDL_min(width - 1, x + radius_x);
            for (int sy = top; sy <= bottom && !covered; sy++) {
                for (int sx = left; sx <= right; sx++) {
                    if (pixel(prepared->authored, sx, sy)[3] ||
                        pixel(prepared->recolour, sx, sy)[3]) {
                        covered = true;
                        break;
                    }
                }
            }
            if (covered) {
                unsigned char *p = pixel(generated, x, y);
                p[0] = p[1] = p[2] = p[3] = 255;
            }
        }
    }
    SDL_DestroySurface(prepared->outline);
    prepared->outline = generated;
    SDL_SetSurfaceBlendMode(generated, SDL_BLENDMODE_BLEND);
    return true;
}

static SvResourceResult prepare(SDL_Surface *source, const SDL_Rect *rectangle,
    int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *options, SvPreparedResource *out) {
    if (!source || !options || !out || width <= 0 || height <= 0 ||
        options->filter < SV_RESOURCE_NEAREST || options->filter > SV_RESOURCE_PIXELART ||
        options->outline_radius < -1) return SV_RESOURCE_INVALID;
    SDL_Rect rect = rectangle ? *rectangle : (SDL_Rect){0, 0, source->w, source->h};
    if (!rect_valid(source, &rect)) return SV_RESOURCE_INVALID;
    SDL_Surface *rgba = SDL_ConvertSurface(source, SDL_PIXELFORMAT_RGBA32);
    if (!rgba) return SV_RESOURCE_MEMORY;
    SDL_Surface *authored = blank(rect.w, rect.h);
    SDL_Surface *recolour = masks ? blank(rect.w, rect.h) : NULL;
    SDL_Surface *outline = masks ? blank(rect.w, rect.h) : NULL;
    if (!authored || (masks && (!recolour || !outline))) {
        SDL_DestroySurface(rgba);
        SDL_DestroySurface(authored);
        SDL_DestroySurface(recolour);
        SDL_DestroySurface(outline);
        return SV_RESOURCE_MEMORY;
    }
    uint64_t hash = UINT64_C(14695981039346656037);
    hash = hash_bytes(hash, &rect, sizeof(rect));
    hash = hash_bytes(hash, &width, sizeof(width));
    hash = hash_bytes(hash, &height, sizeof(height));
    hash = hash_bytes(hash, &options->source_generation, sizeof(options->source_generation));
    hash = hash_bytes(hash, &options->profile_generation, sizeof(options->profile_generation));
    hash = hash_bytes(hash, &options->filter, sizeof(options->filter));
    hash = hash_bytes(hash, &options->outline_radius, sizeof(options->outline_radius));
    /* Hash named fields, not structure padding. */
    if (masks) {
        hash = hash_bytes(hash, &masks->background, sizeof(masks->background));
        hash = hash_bytes(hash, &masks->foreground, sizeof(masks->foreground));
        hash = hash_bytes(hash, &masks->outline, sizeof(masks->outline));
        hash = hash_bytes(hash, &masks->background_rgb, sizeof(masks->background_rgb));
        hash = hash_bytes(hash, &masks->foreground_rgb, sizeof(masks->foreground_rgb));
        hash = hash_bytes(hash, &masks->outline_rgb, sizeof(masks->outline_rgb));
    }
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            unsigned char *p = pixel(rgba, rect.x + x, rect.y + y);
            hash = hash_bytes(hash, p, 4);
            uint32_t rgb = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
            if (!p[3] || (masks && masks->background && rgb == masks->background_rgb)) continue;
            SDL_Surface *destination = authored;
            bool coverage = false;
            if (masks && masks->foreground && rgb == masks->foreground_rgb) {
                destination = recolour;
                coverage = true;
            } else if (masks && masks->outline && rgb == masks->outline_rgb) {
                destination = outline;
                coverage = true;
            }
            unsigned char *d = pixel(destination, x, y);
            if (coverage) {
                d[0] = d[1] = d[2] = 255;
                d[3] = p[3];
            } else memcpy(d, p, 4);
        }
    }
    SDL_DestroySurface(rgba);
    SvPreparedResource candidate = {0};
    candidate.source_rect = rect;
    candidate.requested_filter = options->filter;
    candidate.effective_filter = options->filter == SV_RESOURCE_PIXELART ?
        SV_RESOURCE_NEAREST : options->filter;
    candidate.pixelart_fallback = options->filter == SV_RESOURCE_PIXELART;
    candidate.generation = hash;
    candidate.authored = scaled(authored, width, height, candidate.effective_filter);
    if (masks) {
        candidate.recolour = scaled(recolour, width, height, SV_RESOURCE_NEAREST);
        candidate.outline = scaled(outline, width, height, SV_RESOURCE_NEAREST);
    }
    if (masks && candidate.authored && candidate.effective_filter == SV_RESOURCE_LINEAR) {
        /* Authored color interpolation cannot change categorical layer ownership.
         * The nearest decoded coverage gates filtered pixels at mask boundaries. */
        SDL_Surface *ownership = scaled(authored, width, height, SV_RESOURCE_NEAREST);
        if (!ownership) {
            SDL_DestroySurface(authored);
            SDL_DestroySurface(recolour);
            SDL_DestroySurface(outline);
            sv_resource_prepared_dispose(&candidate);
            return SV_RESOURCE_MEMORY;
        }
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char *p = pixel(candidate.authored, x, y);
                p[3] = pixel(ownership, x, y)[3];
                if (!p[3]) p[0] = p[1] = p[2] = 0;
            }
        }
        SDL_DestroySurface(ownership);
    }
    SDL_DestroySurface(authored);
    SDL_DestroySurface(recolour);
    SDL_DestroySurface(outline);
    if (!candidate.authored || (masks && (!candidate.recolour || !candidate.outline))) {
        sv_resource_prepared_dispose(&candidate);
        return SV_RESOURCE_MEMORY;
    }
    if (masks && options->outline_radius >= 0) {
        if (options->outline_radius == 0) SDL_FillSurfaceRect(candidate.outline, NULL, 0);
        else {
            /* Thickness is applied to final pixels after layer preparation. */
            int rx = SDL_min(width, options->outline_radius);
            int ry = SDL_min(height, options->outline_radius);
            if (!generate_outline(&candidate, SDL_max(1, rx), SDL_max(1, ry))) {
                sv_resource_prepared_dispose(&candidate);
                return SV_RESOURCE_MEMORY;
            }
        }
    }
    sv_resource_prepared_dispose(out);
    *out = candidate;
    return SV_RESOURCE_OK;
}

SvResourceResult sv_resource_prepare_tile(SDL_Surface *source,
    const SDL_Rect *rectangle, int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *options, SvPreparedResource *out) {
    if (!masks) return SV_RESOURCE_INVALID;
    return prepare(source, rectangle, width, height, masks, options, out);
}

SvResourceResult sv_resource_prepare_raw(SDL_Surface *source,
    const SDL_Rect *rectangle, int width, int height,
    const SvResourcePreparation *options, SvPreparedResource *out) {
    return prepare(source, rectangle, width, height, NULL, options, out);
}

SvResourceResult sv_resource_prepare_coverage(SDL_Surface *source,
    int width, int height, SvResourceFilter filter, SvPreparedResource *out) {
    SvResourcePreparation options = {0, 0, filter, 0};
    return prepare(source, NULL, width, height, NULL, &options, out);
}

SDL_Surface *sv_resource_load_image(const char *resolved_path, SvResourceResult *result) {
    if (result) *result = SV_RESOURCE_INVALID;
    if (!resolved_path || !*resolved_path) return NULL;
    size_t size = 0;
    void *bytes = SDL_LoadFile(resolved_path, &size);
    if (!bytes) {
        if (result) *result = SV_RESOURCE_IO;
        return NULL;
    }
    SDL_Surface *surface = NULL;
    if (size >= 8 && !png_sig_cmp(bytes, 0, 8)) {
        png_image image = {0};
        image.version = PNG_IMAGE_VERSION;
        if (png_image_begin_read_from_memory(&image, bytes, size)) {
            image.format = PNG_FORMAT_RGBA;
            if (image.width <= INT_MAX / 4 && image.height <= INT_MAX)
                surface = blank((int)image.width, (int)image.height);
            if (surface && !png_image_finish_read(&image, NULL, surface->pixels,
                                                 surface->pitch, NULL)) {
                SDL_DestroySurface(surface);
                surface = NULL;
            }
        }
        png_image_free(&image);
    } else if (size >= 2 && ((unsigned char *)bytes)[0] == 'B' &&
               ((unsigned char *)bytes)[1] == 'M') {
        SDL_IOStream *input = SDL_IOFromConstMem(bytes, size);
        if (input) surface = SDL_LoadBMP_IO(input, true);
    }
    SDL_free(bytes);
    if (result) *result = surface ? SV_RESOURCE_OK : SV_RESOURCE_INVALID;
    return surface;
}

SvResourceResult sv_resource_prepare_image(const char *resolved_path,
    const SDL_Rect *rectangle, int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *options, SvPreparedResource *out) {
    SvResourceResult result;
    SDL_Surface *source = sv_resource_load_image(resolved_path, &result);
    if (!source) return result;
    result = prepare(source, rectangle, width, height, masks, options, out);
    SDL_DestroySurface(source);
    return result;
}

SvResourceResult sv_resource_prepare_bmp(const char *resolved_path,
    const SDL_Rect *rectangle, int width, int height, const SvResourceMasks *masks,
    const SvResourcePreparation *options, SvPreparedResource *out) {
    return sv_resource_prepare_image(resolved_path, rectangle, width, height, masks, options, out);
}

SDL_Surface *sv_resource_composite(const SvPreparedResource *prepared,
    SDL_Color foreground, SDL_Color outline) {
    if (!prepared || !prepared->authored) return NULL;
    SDL_Surface *result = SDL_DuplicateSurface(prepared->authored);
    if (!result) return NULL;
    SDL_Surface *layers[] = {prepared->recolour, prepared->outline};
    SDL_Color colors[] = {foreground, outline};
    for (size_t i = 0; i < SDL_arraysize(layers); i++) {
        SDL_Surface *layer = layers[i];
        if (!layer) continue;
        if (!SDL_SetSurfaceColorMod(layer, colors[i].r, colors[i].g, colors[i].b) ||
            !SDL_SetSurfaceAlphaMod(layer, colors[i].a) ||
            !SDL_BlitSurface(layer, NULL, result, NULL)) {
            SDL_DestroySurface(result);
            return NULL;
        }
    }
    return result;
}

static bool graphics_masks(const char *root, SvFileOwner owner, const char *stem,
    bool dual, SvResourceMasks *masks, SvGraphicsInventory *inventory)
{
    char base[SV_FILE_PATH], name[SV_FILE_PATH];
    SDL_strlcpy(base, stem, sizeof(base));
    char *suffix = strchr(base, '#');
    int subset = -1;
    if (suffix) {
        char *end;
        long value = strtol(suffix + 1, &end, 10);
        if (end == suffix + 1 || value < 0 || value >= 10) return false;
        subset = (int)value;
        *suffix = 0;
    }
    SvPreferences *preferences = sv_preferences_create(root, root, NULL);
    if (!preferences) return false;
    SvPreferenceContext context = sv_preferences_default_context();
    context.graphics_enabled = true;
    context.character_offset = 256;
    SvOptionOwner role = owner == SV_FILE_BUNDLED ? SV_OPTION_BUNDLED : SV_OPTION_SHARED;
    bool valid = true;
    for (unsigned pass = 0; pass < (suffix ? 2u : 1u); pass++) {
        const char *source = pass ? stem : base;
        if (snprintf(name, sizeof(name), "graphics-%s.prf", source) >= (int)sizeof(name)) {
            valid = false;
            break;
        }
        SvPreferenceResult result = sv_preferences_load(preferences, role, name,
            SV_PREF_EXPLICIT, &context);
        if (result.status == SV_PREF_UNAVAILABLE || result.status == SV_PREF_FATAL) {
            ++inventory->missing_associations;
            valid = false;
            break;
        }
        if (result.status == SV_PREF_PARTIAL) ++inventory->partial_preferences;
    }
    *masks = sv_resource_default_masks(dual);
    if (valid) {
        /* Base definitions first; an actual subset override replaces its roles.
         * These are decoded semantic records from the one production PRF owner. */
        for (unsigned pass = 0; pass < (suffix ? 2u : 1u); pass++) {
            int selected = pass ? subset : -1;
            for (size_t i = 0; i < sv_preferences_mapping_count(preferences); i++) {
                const SvPreferenceMapping *mapping = sv_preferences_mapping(preferences, i);
                if (mapping->family != 'm' || mapping->subtileset != selected) continue;
                uint32_t rgb = (uint32_t)mapping->fields[0] >> 8;
                switch (mapping->index) {
                case 0: masks->background = true; masks->background_rgb = rgb; break;
                case 1: masks->foreground = true; masks->foreground_rgb = rgb; break;
                case 2: masks->outline = dual; masks->outline_rgb = rgb; break;
                }
            }
        }
    }
    sv_preferences_destroy(preferences);
    return valid;
}

SvResourceResult sv_resource_inventory_graphics(const char *root, SvFileOwner owner,
    bool dual_mask, SvGraphicsInventory *inventory)
{
    if (!root || !inventory || (owner != SV_FILE_BUNDLED && owner != SV_FILE_SHARED))
        return SV_RESOURCE_INVALID;
    memset(inventory, 0, sizeof(*inventory));
    SvFilePath directory;
    if (!sv_file_path(&directory, root, "xtra/graphics", owner)) return SV_RESOURCE_INVALID;
    DIR *stream = opendir(directory.path);
    if (!stream) return SV_RESOURCE_IO;
    struct dirent *entry;
    while ((entry = readdir(stream)) != NULL) {
        const char *extension = strrchr(entry->d_name, '.');
        bool png = extension && !SDL_strcasecmp(extension, ".png");
        bool bmp = extension && !SDL_strcasecmp(extension, ".bmp");
        if (!png && !bmp) continue;
        ++inventory->files;
        if (png) ++inventory->png;
        if (bmp) ++inventory->bmp;
        SvFilePath source;
        bool valid = sv_file_path(&source, directory.path, entry->d_name, owner);
        if (!valid) memset(&source, 0, sizeof(source));
        char stem[SV_FILE_PATH];
        size_t length = (size_t)(extension - entry->d_name);
        if (length >= sizeof(stem)) valid = false;
        else { memcpy(stem, entry->d_name, length); stem[length] = 0; }
        if (strchr(entry->d_name, '#')) ++inventory->subsets;
        int width = 0, height = 0;
        if (valid) {
            char *end, *height_start;
            long w = strtol(stem, &end, 10), h = 0;
            if (end == stem || *end != 'x' || w <= 0 || w > INT_MAX) valid = false;
            else {
                height_start = end + 1;
                h = strtol(height_start, &end, 10);
                if (end == height_start || h <= 0 || h > INT_MAX) valid = false;
            }
            if (valid) { width = (int)w; height = (int)h; }
        }
        SvResourceResult result = SV_RESOURCE_INVALID;
        SDL_Surface *loaded = valid ? sv_resource_load_image(source.path, &result) : NULL;
        SDL_Surface *rgba = loaded ? SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32) : NULL;
        SDL_DestroySurface(loaded);
        if (!rgba) valid = false;
        if (valid && (rgba->w < width || rgba->h < height || rgba->w % width || rgba->h % height))
            valid = false;
        SvResourceMasks masks;
        if (valid && !graphics_masks(root, owner, stem, dual_mask, &masks, inventory)) valid = false;
        SDL_Rect rectangle = {0, 0, width, height};
        bool nonempty = false;
        if (valid) {
            for (int y = 0; y < rgba->h && !nonempty; y++) {
                for (int x = 0; x < rgba->w; x++) {
                    unsigned char *p = pixel(rgba, x, y);
                    uint32_t rgb = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
                    if (p[3] && (!masks.background || rgb != masks.background_rgb) &&
                        (!masks.outline || rgb != masks.outline_rgb)) {
                        rectangle.x = (x / width) * width;
                        rectangle.y = (y / height) * height;
                        nonempty = true;
                        break;
                    }
                }
            }
            if (!nonempty) valid = false;
        }
        if (valid) {
            SvResourcePreparation options = {0, 0, SV_RESOURCE_LINEAR, -1};
            SvPreparedResource prepared = {0};
            valid = sv_resource_prepare_tile(rgba, &rectangle, width, height,
                &masks, &options, &prepared) == SV_RESOURCE_OK;
            if (valid) ++inventory->prepared_tiles;
            sv_resource_prepared_dispose(&prepared);
            if (valid) {
                valid = sv_resource_prepare_raw(rgba, &rectangle, width, height,
                    &options, &prepared) == SV_RESOURCE_OK;
                if (valid) ++inventory->prepared_raw;
                sv_resource_prepared_dispose(&prepared);
            }
        }
        SDL_DestroySurface(rgba);
        if (valid) ++inventory->validated;
        else {
            ++inventory->failed;
            SDL_strlcpy(inventory->last_error, source.path[0] ? source.path : entry->d_name,
                sizeof(inventory->last_error));
        }
    }
    closedir(stream);
    return inventory->failed || !inventory->files ? SV_RESOURCE_INVALID : SV_RESOURCE_OK;
}
