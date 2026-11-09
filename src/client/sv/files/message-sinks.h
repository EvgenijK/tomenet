#ifndef SV_MESSAGE_SINKS_H
#define SV_MESSAGE_SINKS_H
#include "app.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
typedef enum {
    SV_MESSAGE_FILE_NOTES, SV_MESSAGE_FILE_CONSOLE, SV_MESSAGE_FILE_CLONE
} SvMessageFileFailure;
/* This owner receives game-message occurrences only, never editor/auth buffers.
 * FILE and notice context are borrowed; no callback may re-enter the app. */
typedef struct {
    char user_root[4096], account[80];
    FILE *console;
    void (*notice)(void *, SvMessageFileFailure);
    void *notice_context;
    bool clone_console, clone_file, got_note;
    uint64_t generation;
} SvMessageFiles;
bool sv_message_files_begin(SvMessageFiles *files, const char *user_root,
    const char *account, FILE *console,
    void (*notice)(void *, SvMessageFileFailure), void *context);
void sv_message_files_options(SvMessageFiles *files, bool console, bool file);
SvMessageSink sv_message_files_sink(SvMessageFiles *files);
#endif
