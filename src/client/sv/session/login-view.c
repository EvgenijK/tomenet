#include "session/login-view.h"
#include <string.h>

void sv_login_view_prepare(SvLoginView *view, const SvPregame *pregame)
{
    *view = (SvLoginView){0};
    if (!pregame) return;
    if (pregame->phase == SV_PREGAME_OVERVIEW ||
        pregame->phase == SV_PREGAME_CHARACTER_PENDING) {
        view->overview = true;
        view->count = pregame->character_count;
        for (size_t i = 0; i < view->count; ++i) {
            const SvPregameCharacter *source = &pregame->characters[i];
            SvLoginRowView *row = &view->rows[i];
            memcpy(row->name, source->name, sizeof(row->name));
            memcpy(row->location, source->location, sizeof(row->location));
            row->level = source->level;
            row->mode = source->mode;
            memcpy(row->race, source->race, sizeof(row->race));
            memcpy(row->class_title, source->class_title, sizeof(row->class_title));
        }
    } else if (pregame->phase == SV_PREGAME_MOTD) {
        view->motd = pregame->motd;
        view->motd_size = pregame->motd_size;
    } else if (pregame->phase == SV_PREGAME_LIVE_HANDOFF) {
        view->live_handoff = true;
    }
}
