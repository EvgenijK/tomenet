#ifndef SV_STARTUP_INS_H
#define SV_STARTUP_INS_H
#include "files/files.h"
#include <stddef.h>

#define SV_INS_RULES 500
#define SV_INS_MATCH 55
#define SV_INS_TAG 19
typedef struct {
    char match[SV_INS_MATCH], tag[SV_INS_TAG];
    bool force, ignore, autopickup, autodestroy, bags_only, disabled, invalid_regex;
} SvStartupInsRule;
typedef enum { SV_INS_MISSING, SV_INS_READY, SV_INS_PARTIAL, SV_INS_IO } SvStartupInsStatus;
typedef struct {
    SvStartupInsStatus status;
    SvFilePath source;
    size_t count;
    unsigned format, warnings;
    bool converted, conversion_failed;
    SvStartupInsRule rules[SV_INS_RULES];
} SvStartupIns;

/* A fresh owner per character. Shared U/user files only, first successfully
 * loaded character/class/trait/race/global file, never layer merging. Names
 * come from accepted identity/setup; NULL skips unavailable identity fields.
 * Old-format conversion is the sole permitted write. Caller frees the owner. */
SvStartupIns *sv_startup_ins_load(const char *user_root, const char *character,
    const char *class_name, const char *trait_name, const char *race_name);
void sv_startup_ins_destroy(SvStartupIns *ins);
#endif
