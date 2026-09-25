#include "session/login-view.h"
#include <string.h>

void sv_login_view_prepare(SvLoginView *view, const SvLogin *login,
                           const SvContactSetup *setup, bool motd_complete)
{
    *view = (SvLoginView){0};
    if (!login || !setup) return;
    SvLoginState state = sv_login_state(login);
    if (state == SV_LOGIN_OVERVIEW) {
        view->overview = true;
        view->count = sv_login_count(login);
        for (size_t i = 0; i < view->count; ++i) {
            const SvLoginCharacter *source = sv_login_character(login, i);
            SvLoginRowView *row = &view->rows[i];
            memcpy(row->name, source->name, sizeof(row->name));
            memcpy(row->location, source->location, sizeof(row->location));
            row->level = source->level;
            row->mode = source->mode;
            if (source->race >= 0 && source->race < setup->race_count)
                memcpy(row->race, setup->races[source->race].title, sizeof(row->race));
            else strcpy(row->race, "?");
            if (source->class_id >= 0 && source->class_id < setup->class_count)
                memcpy(row->class_title, setup->classes[source->class_id].title,
                       sizeof(row->class_title));
            else strcpy(row->class_title, "?");
        }
    } else if (state == SV_LOGIN_SELECTED && !motd_complete) {
        view->motd = setup->motd;
        view->motd_size = setup->motd_size;
    }
}
