#ifndef SV_FILE_HISTORY_H
#define SV_FILE_HISTORY_H
#include "files/files.h"
#define SV_HISTORY_CAPACITY 2000
#define SV_HISTORY_BYTES 256
typedef struct SvInputHistory SvInputHistory;
typedef enum { SV_HISTORY_PUBLIC_CHAT, SV_HISTORY_SECRET } SvHistoryInput;
SvInputHistory *sv_history_create(void);
void sv_history_destroy(SvInputHistory *);
/* root is the shared SDL3 U; this owner alone resolves its private U/sv path. */
SvFileResult sv_history_begin(SvInputHistory *, const char *root, const char *account);
SvFileResult sv_history_accept(SvInputHistory *, SvHistoryInput, const char *input);
SvFileResult sv_history_save(SvInputHistory *);
size_t sv_history_count(const SvInputHistory *);
const char *sv_history_entry(const SvInputHistory *, size_t oldest_first);
#endif
