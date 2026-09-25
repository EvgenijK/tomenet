#include "resource.h"
#include <string.h>

static bool relative_name(const char *name)
{
    if (!name || !*name || *name == '/' || *name == '\\') return false;
    const char *segment = name;
    for (const unsigned char *p = (const unsigned char *)name;; ++p) {
        if (*p == 0 || *p == '/' || *p == '\\') {
            size_t length = (const char *)p - segment;
            if (!length || (length == 1 && segment[0] == '.') ||
                (length == 2 && segment[0] == '.' && segment[1] == '.')) return false;
            if (!*p) break;
            segment = (const char *)p + 1;
        } else if (*p < 32 || *p == 127 || *p == ':') return false;
    }
    return true;
}

bool sv_resource_path(const char *user_root, const char *library,
                      const char *relative, SvResourceOwner owner, SvResourceRef *out)
{
    const char *root = owner == SV_RESOURCE_USER ? user_root : library;
    if (!out || !root || !*root || !relative_name(relative) ||
        (owner != SV_RESOURCE_USER && owner != SV_RESOURCE_BUNDLED)) return false;
    int size = SDL_snprintf(out->path, sizeof(out->path), "%s/%s", root, relative);
    if (size < 0 || size >= (int)sizeof(out->path)) return false;
    out->owner = owner;
    return true;
}

bool sv_resource_find(const char *user_root, const char *library,
                      const char *relative, SDL_PathType type, SvResourceRef *out)
{
    SDL_PathInfo info;
    for (int owner = SV_RESOURCE_USER; owner <= SV_RESOURCE_BUNDLED; ++owner)
        if (sv_resource_path(user_root, library, relative, (SvResourceOwner)owner, out) &&
            SDL_GetPathInfo(out->path, &info) && info.type == type) return true;
    return false;
}

bool sv_resource_write_target(const char *user_root, const char *relative,
                              SvResourceRef *out)
{
    return sv_resource_path(user_root, NULL, relative, SV_RESOURCE_USER, out);
}

bool sv_resource_sidecar(const SvResourceRef *owner, const char *suffix,
                         SvResourceRef *out)
{
    if (!owner || owner->owner != SV_RESOURCE_USER ||
        !suffix || !*suffix || !out) return false;
    for (const unsigned char *p = (const unsigned char *)suffix; *p; ++p)
        if (*p < 33 || *p > 126 || *p == '/' || *p == '\\' || *p == ':') return false;
    int size = SDL_snprintf(out->path, sizeof(out->path), "%s%s", owner->path, suffix);
    if (size < 0 || size >= (int)sizeof(out->path)) return false;
    out->owner = owner->owner;
    return true;
}

bool sv_resource_stat(const SvResourceRef *ref, SDL_PathInfo *info)
{
    return ref && info && SDL_GetPathInfo(ref->path, info);
}

bool sv_resource_rename(const SvResourceRef *from, const SvResourceRef *to)
{
    return from && to && from->owner == SV_RESOURCE_USER &&
           to->owner == SV_RESOURCE_USER &&
           SDL_RenamePath(from->path, to->path);
}

bool sv_resource_remove(const SvResourceRef *ref)
{
    return ref && ref->owner == SV_RESOURCE_USER && SDL_RemovePath(ref->path);
}
