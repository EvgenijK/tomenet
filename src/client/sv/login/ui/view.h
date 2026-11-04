#ifndef SV_LOGIN_UI_VIEW_H
#define SV_LOGIN_UI_VIEW_H
#include "input/text-field.h"
#include "login/account/credentials.h"
struct SvLoginView;
struct SvBirthView;
struct SvBirthViewport;
struct SvChronicleSnapshot;
/* A synchronous frame projection. All borrowed pointers expire before mutation. */
typedef struct {
    SvTextResult text_error;
    bool clipboard_unavailable;
    const char *contact_status, *temporary_surface_text;
    const struct SvLoginView *login_view;
    const SvCredentialPresentation *credentials;
    const char *private_password; /* Reveal only: never retained/cached. */
    bool contact_failed, retry_available;
    size_t hover, hover_link;
    bool hover_enter, hover_manual, list_focus_manual;
    int manual_focus;
    float chronicle_scroll;
    const struct SvChronicleSnapshot *chronicle;
    const struct SvBirthView *birth_view;
    const struct SvBirthViewport *birth_viewport;
} SvEndpointView;
typedef struct {
    unsigned summary_max, detail_max;
} SvEndpointRenderMetrics;
#endif
