#ifndef SV_SESSION_LOGIN_VIEW_H
#define SV_SESSION_LOGIN_VIEW_H
#include "protocol/login.h"
#include "session/character-setup.h"

typedef struct {
    char name[SV_LOGIN_NAME_CAPACITY];
    char race[SV_CONTACT_NAME_CAPACITY], class_title[SV_CONTACT_NAME_CAPACITY];
    char location[SV_LOGIN_NAME_CAPACITY];
    int16_t level, mode;
} SvLoginRowView;
typedef struct SvLoginView {
    bool overview;
    size_t count;
    SvLoginRowView rows[SV_LOGIN_MAX_CHARACTERS];
    const unsigned char *motd; /* Borrowed from contact setup for one frame. */
    size_t motd_size;
} SvLoginView;

void sv_login_view_prepare(SvLoginView *view, const SvLogin *login,
                           const SvContactSetup *setup, bool motd_complete);
#endif
