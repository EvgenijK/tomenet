#ifndef SV_TEXT_FIELD_H
#define SV_TEXT_FIELD_H
#include <stdbool.h>
#include <stddef.h>

/* Protocol fields are bounded in bytes, independently of glyph width. */
#define SV_TEXT_CAPACITY 255
#define SV_TEXT_HISTORY 16
typedef enum {
    SV_EDIT_LEFT, SV_EDIT_RIGHT, SV_EDIT_HOME, SV_EDIT_END,
    SV_EDIT_WORD_LEFT, SV_EDIT_WORD_RIGHT, SV_EDIT_BACKSPACE,
    SV_EDIT_DELETE, SV_EDIT_WORD_DELETE, SV_EDIT_HISTORY_BACK,
    SV_EDIT_HISTORY_FORWARD, SV_EDIT_SEARCH, SV_EDIT_ACCEPT, SV_EDIT_CANCEL
} SvEditKey;
typedef enum { SV_TEXT_OK, SV_TEXT_TRUNCATED, SV_TEXT_ENCODING_ERROR,
               SV_TEXT_INVALID } SvTextResult;
typedef struct {
    char entries[SV_TEXT_HISTORY][SV_TEXT_CAPACITY + 1];
    size_t count;
} SvTextHistory;
typedef struct {
    char bytes[SV_TEXT_CAPACITY + 1];
    size_t length, cursor, anchor, limit;
    bool private_field, searching, accepted, cancelled, dirty;
    char history[SV_TEXT_HISTORY][SV_TEXT_CAPACITY + 1];
    size_t history_count, history_cursor, search_cursor;
    char preview[SV_TEXT_CAPACITY + 1];
    SvTextHistory *shared_history;
} SvTextField;
void sv_text_begin(SvTextField *field, const char *initial, size_t limit, bool private_field);
void sv_text_begin_history(SvTextField *field, SvTextHistory *history,
                           const char *initial, size_t limit, bool private_field);
bool sv_text_select(SvTextField *field, size_t start, size_t end);
SvTextResult sv_text_insert_utf8(SvTextField *field, const char *utf8);
SvTextResult sv_text_paste(SvTextField *field, const char *utf8, size_t length);
void sv_text_key(SvTextField *field, SvEditKey key);
void sv_text_remember(SvTextField *field);
#endif
