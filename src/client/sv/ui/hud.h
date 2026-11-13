#ifndef SV_HUD_H
#define SV_HUD_H
#include "app.h"
#include "settings/config.h"
#include "settings/options.h"
#include "ui/font.h"
#include "ui/hud-icons.h"
#define SV_HUD_ROWS 96
#define SV_HUD_ROW_TEXT 256
typedef struct {
    bool small, bar[4], huge[5], solid, graphics;
    bool experience_bar, linear_stats;
    int ui_scale_percent;
    /* Zero selects the initial viewport-derived default; explicit selections
     * are logical panel extents and survive resize/DPI changes. */
    int left_width, right_width;
} SvHudPreferences;
typedef struct {
    char label[32], value[SV_HUD_ROW_TEXT];
    SDL_Color colour;
    bool formatted;
    unsigned char colours[SV_HUD_ROW_TEXT];
} SvHudRow;
typedef enum {
    SV_HUD_CONDITION_FOOD, SV_HUD_CONDITION_VISION, SV_HUD_CONDITION_CONFUSION,
    SV_HUD_CONDITION_FEAR, SV_HUD_CONDITION_POISON, SV_HUD_CONDITION_ACTIVITY,
    SV_HUD_CONDITION_CUT, SV_HUD_CONDITION_STUN, SV_HUD_CONDITION_COUNT
} SvHudConditionIndex;
typedef struct {
    bool available, active, critical;
    int severity;
    SDL_Color colour;
    SvHudIcon icon;
} SvHudCondition;
typedef enum {
    SV_HUD_CHARACTER_PROFILE, SV_HUD_CHARACTER_SKILLS, SV_HUD_CHARACTER_RESISTS,
    SV_HUD_CHARACTER_COUNT
} SvHudCharacterPage;
typedef struct {
    uint64_t generation, revision, redraw_revision;
    bool valid, character_open, pointer_known, widths_selected;
    int left_width, right_width;
    float left_scale, left_content_height;
    SvHudCondition conditions[SV_HUD_CONDITION_COUNT];
    /* Derived outputs consumed by the ordinary HUD and Character rows. */
    int encumbrance_slots[12], sanity_bar_segments;
    char level_value[32];
    SDL_Color level_colour, experience_colour;
    double experience_fraction, experience_maximum_fraction;
    float pointer_x, pointer_y;
    int scroll, widget_scroll, scroll_max, widget_scroll_max, width, height;
    SvHudCharacterPage character_page, summary_page;
    int page_scroll[SV_HUD_CHARACTER_COUNT], page_widget_scroll[SV_HUD_CHARACTER_COUNT];
    bool dump_notice;
    float scale;
    uint64_t font_revision;
    SvHudPreferences preferences;
    SvHudRow rows[SV_HUD_ROWS];
    size_t row_count;
    SDL_FRect shortcut, primary, widget;
} SvHudPresentation;
SvHudPreferences sv_hud_preferences(const SvSettingsConfig *, const SvSettingsOptions *);
/* Pure derived presentation: holds copies, never takes effects or sends commands. */
bool sv_hud_prepare(SvHudPresentation *, SvAppView, SvHudPreferences,
    int width, int height, float display_scale, uint64_t font_revision);
bool sv_hud_draw(SvHudPresentation *, SDL_Renderer *, SvFont *, SvAppView,
    SvHudPreferences, float display_scale, uint64_t font_revision);
/* Caller routes events here only in ordinary game input. Pending requests retain ownership. */
bool sv_hud_event(SvHudPresentation *, SvAppView, const SDL_Event *);
#endif
