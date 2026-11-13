#include "files/message-sinks.h"
#include "files/files.h"
#include <string.h>

static void reset(void *context)
{
    SvMessageFiles *files = context;
    files->got_note = false;
    files->generation = 0;
}
bool sv_message_files_begin(SvMessageFiles *files, const char *user_root,
    const char *account, FILE *console,
    void (*notice)(void *, SvMessageFileFailure), void *context)
{
    SvFilePath path;
    char name[96];
    if (!files || !user_root || !*user_root || !account || !*account ||
        strlen(user_root) >= sizeof(files->user_root) ||
        strlen(account) >= sizeof(files->account) ||
        snprintf(name, sizeof(name), "notes-%s.txt", account) >= (int)sizeof(name) ||
        !sv_file_shared_user(&path, user_root, name)) return false;
    *files = (SvMessageFiles){.console = console ? console : stdout,
        .notice = notice, .notice_context = context};
    strcpy(files->user_root, user_root);
    strcpy(files->account, account);
    return true;
}
void sv_message_files_options(SvMessageFiles *files, bool console, bool file)
{
    files->clone_console = console;
    files->clone_file = file;
}
/* Baseline Receive_message notes removes all routing markers. c_msg_print
 * clones only remove its leading routing markers, then colour pairs everywhere. */
static size_t strip(char *out, const unsigned char *text, size_t length, bool notes)
{
    size_t n = 0;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = text[i];
        if (notes && c >= 252 && c <= 254) continue;
        if (c == 255) {
            if (i + 1 < length) ++i;
            continue; /* Includes broken final colour marker. */
        }
        out[n++] = (char)c;
    }
    out[n] = 0;
    return n;
}
static bool append(SvMessageFiles *files, const char *name, const char *bytes, size_t length)
{
    SvFilePath path;
    if (!sv_file_shared_user(&path, files->user_root, name)) return false;
    FILE *file = sv_file_open(&path, "a");
    if (!file) return false;
    bool ok = fwrite(bytes, 1, length, file) == length;
    if (fclose(file)) ok = false; /* Includes buffered write/flush failure. */
    return ok;
}
static bool failure(SvMessageFiles *files, SvMessageFileFailure kind)
{
    if (files->notice) files->notice(files->notice_context, kind);
    return false;
}
static int deliver(void *context, uint64_t generation, const SvMessage *message)
{
    SvMessageFiles *files = context;
    if (!message || message->length >= SV_MESSAGE_BYTES) return 0;
    if (files->generation != generation) {
        reset(files);
        files->generation = generation;
    }
    const unsigned char *raw = message->bytes;
    size_t length = message->length;
    bool note = (length >= 16 && !memcmp(raw + 6, "Note from ", 10)) ||
                (files->got_note && length >= 3 && raw[2] == ' ');
    bool ok = true;
    if (note) {
        /* Intentionally use the previous chain state even for another header,
         * matching baseline Receive_message's got_note ? 2 : 16 offset. */
        size_t offset = files->got_note ? 2 : 16;
        char body[SV_MESSAGE_BYTES], line[SV_MESSAGE_BYTES + 32], name[96];
        size_t n = strip(body, raw + offset, length - offset, true);
        time_t clock = time(NULL);
        struct tm *local = localtime(&clock);
        int prefix = local ? snprintf(line, sizeof(line), "[%04d/%02d/%02d - %02d:%02d] ",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min) : -1;
        snprintf(name, sizeof(name), "notes-%s.txt", files->account);
        if (prefix < 0 || (size_t)prefix + n + 1 >= sizeof(line))
            ok = failure(files, SV_MESSAGE_FILE_NOTES);
        else {
            memcpy(line + prefix, body, n);
            line[prefix + n] = '\n';
            if (!append(files, name, line, (size_t)prefix + n + 1))
                ok = failure(files, SV_MESSAGE_FILE_NOTES);
        }
    }
    files->got_note = note; /* Also advance after disk failure, as baseline. */
    /* Receive_message's clear-topline sentinel never reaches c_msg_print(text). */
    if (length == 1 && raw[0] == 255) return ok;
    size_t offset = 0;
    if (length >= 2 && raw[0] == 255 && raw[1] == 255) offset = 2;
    if (offset < length && raw[offset] == 252) ++offset;
    if (offset < length && raw[offset] == 253) ++offset;
    if (offset < length && raw[offset] == 254) ++offset;
    char line[SV_MESSAGE_BYTES + 1];
    size_t n = strip(line, raw + offset, length - offset, false);
    line[n++] = '\n';
    if (files->clone_console) {
        bool written = fwrite(line, 1, n, files->console) == n;
        if (fflush(files->console)) written = false;
        if (!written) ok = failure(files, SV_MESSAGE_FILE_CONSOLE);
    }
    if (files->clone_file && !append(files, "stdout.txt", line, n))
        ok = failure(files, SV_MESSAGE_FILE_CLONE);
    return ok;
}
SvMessageSink sv_message_files_sink(SvMessageFiles *files)
{
    return (SvMessageSink){files, deliver, reset};
}
