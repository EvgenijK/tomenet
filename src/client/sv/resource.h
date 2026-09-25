#ifndef SV_RESOURCE_H
#define SV_RESOURCE_H
#include <stdbool.h>
#include <SDL3/SDL.h>

#define SV_RESOURCE_PATH 4096
typedef enum { SV_RESOURCE_USER, SV_RESOURCE_BUNDLED } SvResourceOwner;
typedef struct { char path[SV_RESOURCE_PATH]; SvResourceOwner owner; } SvResourceRef;

/* A concrete shared-resource owner is selected once. Read probes the user
 * overlay before B. Mutations and writable sidecars accept U refs only;
 * callers retain the concrete ref across raw stat/temp/rename/remove. */
bool sv_resource_path(const char *user_root, const char *library,
                      const char *relative, SvResourceOwner owner, SvResourceRef *out);
bool sv_resource_find(const char *user_root, const char *library,
                      const char *relative, SDL_PathType type, SvResourceRef *out);
bool sv_resource_write_target(const char *user_root, const char *relative,
                              SvResourceRef *out);
bool sv_resource_sidecar(const SvResourceRef *owner, const char *suffix,
                         SvResourceRef *out);
bool sv_resource_stat(const SvResourceRef *ref, SDL_PathInfo *info);
bool sv_resource_rename(const SvResourceRef *from, const SvResourceRef *to);
bool sv_resource_remove(const SvResourceRef *ref);
#endif
