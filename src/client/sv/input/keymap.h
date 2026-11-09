#ifndef SV_INPUT_KEYMAP_H
#define SV_INPUT_KEYMAP_H
#include "result.h"
#include "settings/preferences.h"

typedef struct {
    unsigned char commands[128], directions[128];
    bool rogue;
    unsigned char prefix, last_direction;
} SvKeymap;
typedef struct { unsigned char command, direction; } SvCommand;
/* Fresh owner/reset. Rebuild only replaces maps, like baseline keymap_init. */
void sv_keymap_init(SvKeymap *, bool rogue);
void sv_keymap_rebuild(SvKeymap *);
/* A zero input or incomplete prefix returns WAITING, with no command. */
SvResult sv_keymap_key(SvKeymap *, unsigned char, SvCommand *);
/* Consume ordered typed records once. Caller retains cursor for this owner.
 * Option records set the layout choice; only explicit rebuild regenerates maps.
 * Fresh preference owner starts at cursor zero. */
void sv_keymap_preferences(SvKeymap *, const SvPreferences *, size_t *cursor);
#endif
