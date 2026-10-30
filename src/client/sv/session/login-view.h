#ifndef SV_SESSION_LOGIN_VIEW_H
#define SV_SESSION_LOGIN_VIEW_H
#include "session/pregame.h"
#include "input/text-field.h"

typedef struct {
    char name[SV_LOGIN_NAME_CAPACITY];
    char race[SV_CONTACT_NAME_CAPACITY], class_title[SV_CONTACT_NAME_CAPACITY];
    char location[SV_LOGIN_NAME_CAPACITY];
    int16_t level, mode;
} SvLoginRowView;
typedef struct SvLoginView {
    bool overview;
    bool ordinary_name_entry;
    bool ordinary_creation_available;
    bool exclusive_creation_available, creation_supported, first_run, pending;
    size_t free_slots[3], selected, first;
    int focus;
    const SvTextField *name_field; /* One frame, interaction-owned. */
    bool birth;
    bool live_handoff;
    size_t count;
    SvLoginRowView rows[SV_LOGIN_MAX_CHARACTERS];
    const unsigned char *motd; /* Borrowed from contact setup for one frame. */
    size_t motd_size;
    const char *character_name_draft; /* Borrowed from the interaction for one frame. */
} SvLoginView;

void sv_login_view_prepare(SvLoginView *view, const SvPregame *pregame);
#endif
