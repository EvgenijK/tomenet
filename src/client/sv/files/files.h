#ifndef SV_FILES_H
#define SV_FILES_H
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#define SV_FILE_PATH 4096
typedef enum { SV_FILE_SHARED, SV_FILE_PROFILE, SV_FILE_BUNDLED } SvFileOwner;
typedef struct { char path[SV_FILE_PATH]; SvFileOwner owner; } SvFilePath;
typedef enum { SV_FILE_OK, SV_FILE_MISSING, SV_FILE_INVALID, SV_FILE_IO,
               SV_FILE_REPLACE_REQUIRED, SV_FILE_CONFLICT } SvFileResult;
bool sv_file_path(SvFilePath *, const char *root, const char *relative, SvFileOwner);
bool sv_file_shared_user(SvFilePath *, const char *user_root, const char *basename);
/* Only missing user candidates fall through; content fallback belongs to consumer. */
SvFileResult sv_file_resource(SvFilePath *, const char *user_root,
                              const char *library, const char *relative);
FILE *sv_file_open(const SvFilePath *, const char *mode);
int sv_file_stat(const SvFilePath *, struct stat *);
int sv_file_remove(const SvFilePath *);
int sv_file_rename(const SvFilePath *, const SvFilePath *);
/* Stack-owned operation: cancel on every abandoned/error path. Replace consent
 * refers to the exact observed destination. Shared legacy writers do not lock. */
typedef struct {
    SvFilePath target;
    char temporary[SV_FILE_PATH];
    FILE *stream;
    struct stat observed;
    bool active, existed, generated;
} SvFilePublication;
SvFileResult sv_file_publish_begin(SvFilePublication *, const SvFilePath *, bool generated);
SvFileResult sv_file_publish_write(SvFilePublication *, const void *, size_t);
SvFileResult sv_file_publish_commit(SvFilePublication *, bool replace);
void sv_file_publish_cancel(SvFilePublication *);
#endif
