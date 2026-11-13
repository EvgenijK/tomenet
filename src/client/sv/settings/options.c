#define _XOPEN_SOURCE 700
#include "settings/options.h"
#include "settings/preferences.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
/* SV-local baseline option_info slot identities and compiled defaults.
 * Menu visibility does not remove slots. Baseline slots 188..198 are zero. */
static const struct { const char *name; unsigned char value; } definitions[199] = {
    [0] = {"rogue_like_commands", 0},
    [1] = {"newbie_hints", 1},
    [2] = {"censor_swearing", 1},
    [3] = {"highlight_chat", 1},
    [4] = {"highbeep_chat", 1},
    [5] = {"page_on_privmsg", 0},
    [6] = {"page_on_afk_privmsg", 1},
    [7] = {"big_map", 0},
    [8] = {"font_map_solid_walls", 1},
    [9] = {"view_animated_light", 1},
    [10] = {"wall_lighting", 1},
    [11] = {"view_lamp_walls", 1},
    [12] = {"view_shade_walls", 1},
    [13] = {"floor_lighting", 1},
    [14] = {"view_lamp_floor", 1},
    [15] = {"view_shade_floor", 1},
    [16] = {"view_light_extra", 1},
    [17] = {"alert_hitpoint", 0},
    [18] = {"alert_mana", 0},
    [19] = {"alert_afk_dam", 0},
    [20] = {"alert_offpanel_dam", 1},
    [21] = {"exp_bar", 1},
    [22] = {"uniques_alive", 0},
    [23] = {"warn_unique_credit", 0},
    [24] = {"limit_chat", 0},
    [25] = {"no_afk_msg", 0},
    [26] = {"overview_startup", 0},
    [27] = {"allow_paging", 1},
    [28] = {"ring_bell", 1},
    [29] = {"linear_stats", 0},
    [30] = {"exp_need", 0},
    [31] = {"depth_in_feet", 1},
    [32] = {"newb_suicide", 1},
    [33] = {"show_weights", 1},
    [34] = {"time_stamp_chat", 0},
    [35] = {"hide_unusable_skills", 1},
    [36] = {"short_item_names", 0},
    [37] = {"keep_topline", 0},
    [38] = {"target_history", 0},
    [39] = {"taciturn_messages", 0},
    [40] = {"always_show_lists", 1},
    [41] = {"no_weather", 0},
    [42] = {"player_list", 0},
    [43] = {"player_list2", 0},
    [44] = {"flash_player", 1},
    [45] = {"highlight_player", 0},
    [46] = {"basic_players_symb", 0},
    [47] = {"subterm_flicker", 1},
    [48] = {"no_verify_sell", 0},
    [49] = {"no_verify_destroy", 0},
    [50] = {"auto_afk", 1},
    [51] = {"idle_starve_kick", 1},
    [52] = {"safe_float", 0},
    [53] = {"safe_macros", 1},
    [54] = {"auto_untag", 0},
    [55] = {"clear_inscr", 0},
    [56] = {"auto_inscr_server", 0},
    [57] = {"stack_force_notes", 1},
    [58] = {"stack_force_costs", 1},
    [59] = {"stack_allow_items", 1},
    [60] = {"stack_allow_devices", 1},
    [61] = {"whole_ammo_stack", 0},
    [62] = {"always_repeat", 1},
    [63] = {"always_pickup", 0},
    [64] = {"use_old_target", 1},
    [65] = {"autooff_retaliator", 0},
    [66] = {"fail_no_melee", 0},
    [67] = {"basic_players_col", 0},
    [68] = {"auto_target", 0},
    [69] = {"thin_down_flush", 1},
    [70] = {"disable_flush", 0},
    [71] = {"find_ignore_stairs", 0},
    [72] = {"find_ignore_doors", 1},
    [73] = {"find_cut", 1},
    [74] = {"find_examine", 1},
    [75] = {"disturb_move", 0},
    [76] = {"disturb_near", 0},
    [77] = {"disturb_panel", 0},
    [78] = {"disturb_state", 0},
    [79] = {"disturb_minor", 0},
    [80] = {"disturb_other", 0},
    [81] = {"view_perma_grids", 1},
    [82] = {"view_torch_grids", 0},
    [83] = {"view_reduce_light", 0},
    [84] = {"view_reduce_view", 0},
    [85] = {"easy_open", 1},
    [86] = {"easy_disarm", 0},
    [87] = {"easy_tunnel", 0},
    [88] = {"audio_paging", 1},
    [89] = {"paging_master_vol", 0},
    [90] = {"paging_max_vol", 0},
    [91] = {"no_ovl_close_sfx", 1},
    [92] = {"ovl_sfx_attack", 1},
    [93] = {"no_combat_sfx", 0},
    [94] = {"no_magicattack_sfx", 0},
    [95] = {"no_defense_sfx", 0},
    [96] = {"half_sfx_attack", 0},
    [97] = {"cut_sfx_attack", 1},
    [98] = {"ovl_sfx_command", 1},
    [99] = {"ovl_sfx_misc", 1},
    [100] = {"ovl_sfx_mon_attack", 1},
    [101] = {"ovl_sfx_mon_spell", 1},
    [102] = {"ovl_sfx_mon_misc", 1},
    [103] = {"no_monsterattack_sfx", 0},
    [104] = {"positional_audio", 1},
    [105] = {"no_house_sfx", 0},
    [106] = {"quiet_house_sfx", 1},
    [107] = {"mute_when_idle", 0},
    [108] = {"alert_starvation", 1},
    [109] = {"use_color", 1},
    [110] = {"other_query_flag", 0},
    [111] = {"shuffle_music", 0},
    [112] = {"permawalls_shade", 0},
    [113] = {"topline_no_msg", 0},
    [114] = {"targetinfo_msg", 0},
    [115] = {"live_timeouts", 1},
    [116] = {"flash_insane", 0},
    [117] = {"last_words", 1},
    [118] = {"disturb_see", 0},
    [119] = {"diz_unique", 1},
    [120] = {"diz_death", 1},
    [121] = {"diz_death_any", 1},
    [122] = {"diz_first", 1},
    [123] = {"screenshot_format", 1},
    [124] = {"palette_animation", 1},
    [125] = {"play_all", 1},
    [126] = {"id_selection", 1},
    [127] = {"hp_bar", 0},
    [128] = {"mp_bar", 0},
    [129] = {"st_bar", 0},
    [130] = {"find_ignore_montraps", 1},
    [131] = {"quiet_os", 0},
    [132] = {"disable_lightning", 0},
    [133] = {"macros_in_stores", 0},
    [134] = {"item_error_beep", 1},
    [135] = {"keep_bottle", 0},
    [136] = {"easy_disarm_montraps", 0},
    [137] = {"no_house_magic", 0},
    [138] = {"no_light_fainting", 0},
    [139] = {"auto_pickup", 0},
    [140] = {"auto_destroy", 0},
    [141] = {"destroy_all_unmatched", 0},
    [142] = {"mp_huge_bar", 1},
    [143] = {"sn_huge_bar", 1},
    [144] = {"hp_huge_bar", 1},
    [145] = {"clone_to_stdout", 0},
    [146] = {"clone_to_file", 0},
    [147] = {"first_song", 1},
    [148] = {"equip_text_colour", 0},
    [149] = {"equip_set_colour", 1},
    [150] = {"colourize_bignum", 0},
    [151] = {"flash_player2", 0},
    [152] = {"load_form_macros", 1},
    [153] = {"auto_inscr_off", 0},
    [154] = {"ascii_feats", 0},
    [155] = {"ascii_items", 0},
    [156] = {"ascii_monsters", 0},
    [157] = {"ascii_uniques", 0},
    [158] = {"no_flicker", 0},
    [159] = {"stun_huge_bar", 1},
    [160] = {"gfx_autooff_fmsw", 0},
    [161] = {"solid_bars", 1},
    [162] = {"autopickup_chemicals", 1},
    [163] = {"add_kind_diz", 1},
    [164] = {"hide_lore_paste", 0},
    [165] = {"new_retaliator", 1},
    [166] = {"sunburn_msg", 1},
    [167] = {"wide_scroll_margin", 1},
    [168] = {"gfx_palanim_repaint", 1},
    [169] = {"gfx_hack_repaint", 1},
    [170] = {"topline_first", 1},
    [171] = {"ascii_weather", 0},
    [172] = {"no2mask_weather", 0},
    [173] = {"auto_inscr_server_ch", 1},
    [174] = {"screenshot_keys", 0},
    [175] = {"wild_resume_from_any", 1},
    [176] = {"tavern_town_resume", 1},
    [177] = {"st_huge_bar", 0},
    [178] = {"huge_bars_gfx", 1},
    [179] = {"autoinsc_debug", 0},
    [180] = {"autoloot_dunonly", 0},
    [181] = {"autoloot_dununown", 0},
    [182] = {"flash_starvation", 1},
    [183] = {"autoswitch_inven", 1},
    [184] = {"show_newest", 0},
    [185] = {"log_music", 0},
    [186] = {"misc_no_flicker", 0},
    [187] = {"prefer_subinven", 1},
};
int sv_settings_option_slot(const char *name)
{
    if (!name) return -1;
    for (int i = 0; i < SV_SETTINGS_OPTION_COUNT; ++i)
        if (definitions[i].name && !strcmp(definitions[i].name, name)) return i;
    return -1;
}
bool sv_settings_option_set(SvSettingsOptions *options, const char *name, bool value)
{
    int slot = sv_settings_option_slot(name);
    if (!options || slot < 0) return false;
    options->values[slot] = value;
    return true;
}
void sv_settings_options_defaults(SvSettingsOptions *options)
{
    memset(options, 0, sizeof(*options));
    for (int i = 0; i < SV_SETTINGS_OPTION_COUNT; ++i)
        options->values[i] = definitions[i].value;
}
bool sv_settings_option_apply_record(SvSettingsOptions *options, const char *name,
    bool value, int slots[2], size_t *count)
{
    size_t applied=0;
    if (count) *count=0;
    if (!options || !name) return false;
    static const struct { const char *old_name, *new_name; } aliases[] = {
        {"recall_flicker", "subterm_flicker"},
        {"autoloot_depth", "autoloot_dunonly"}, {"autoloot_off", "autoloot_dunonly"},
        {"kind_diz", "add_kind_diz"}, {"auto_inscribe", "auto_inscr_server"},
        {"hilite_chat", "highlight_chat"}, {"hibeep_chat", "highbeep_chat"},
        {"view_animated_lite", "view_animated_light"}, {"view_lite_extra", "view_light_extra"},
        {"no_lite_fainting", "no_light_fainting"}, {"hilite_player", "highlight_player"},
        {"colourize_prices", "colourize_bignum"}, {"sp_huge_bar", "sn_huge_bar"},
        {"auto_insc_off", "auto_inscr_off"}, {"stack_allow_wands", "stack_allow_devices"}
    };
    if (!strcmp(name, "instant_retaliator")) {
        name = "new_retaliator"; value = !value; ++options->aliases;
    } else if (!strcmp(name, "basic_players") || !strcmp(name, "consistent_players")) {
        name = "basic_players_symb";
        sv_settings_option_set(options, "basic_players_col", false);
        if (slots) slots[applied]=sv_settings_option_slot("basic_players_col");
        ++applied;
        ++options->aliases;
    } else if (!strcmp(name, "view_reduce_lite")) {
        ++options->aliases; return true;
    } else {
        for (size_t i = 0; i < sizeof(aliases)/sizeof(*aliases); ++i)
            if (!strcmp(name, aliases[i].old_name)) {
                name = aliases[i].new_name; ++options->aliases; break;
            }
    }
    int slot=sv_settings_option_slot(name);
    if (slot<0) return false;
    options->values[slot]=value;
    if (slots) slots[applied]=slot;
    if (count) *count=applied+1;
    return true;
}
bool sv_settings_option_apply(SvSettingsOptions *options,const char *name,bool value)
{
    return sv_settings_option_apply_record(options,name,value,NULL,NULL);
}
/* Compatibility entrypoints use the same production preference interpreter.
 * Stateful callers retain SvPreferences instead of discarding non-option data. */
bool sv_settings_options_file(SvSettingsOptions *options, const char *user_root,
    const char *library_root, SvOptionOwner owner, const char *name)
{
    if (!options || !user_root) return false;
    SvPreferences *preferences = sv_preferences_create(user_root, library_root, options);
    if (!preferences) return false;
    SvPreferenceResult result = sv_preferences_load(preferences, owner, name,
        SV_PREF_OPTIONAL, NULL);
    *options = *sv_preferences_options(preferences);
    sv_preferences_destroy(preferences);
    return result.status == SV_PREF_CLEAN;
}
bool sv_settings_options_load(SvSettingsOptions *options, const char *user_root,
                              const char *library_root)
{
    if (!options || !user_root) return false;
    SvPreferences *preferences = sv_preferences_create(user_root, library_root, NULL);
    if (!preferences) return false;
    SvPreferenceResult result = sv_preferences_bootstrap(preferences, NULL, NULL, NULL);
    *options = *sv_preferences_options(preferences);
    sv_preferences_destroy(preferences);
    return result.status == SV_PREF_CLEAN;
}
bool sv_settings_options_character(SvSettingsOptions *out,
    const SvSettingsOptions *base, const char *user_root, const char *library_root,
    const char *character)
{
    if (!out || !base || !user_root) return false;
    *out = *base;
    SvPreferences *preferences = sv_preferences_create(user_root, library_root, base);
    if (!preferences) return false;
    SvPreferenceResult result;
    SvPreferences *overlay = sv_preferences_character(preferences, character, NULL, &result);
    if (overlay) *out = *sv_preferences_options(overlay);
    sv_preferences_destroy(overlay);
    sv_preferences_destroy(preferences);
    return overlay && result.status == SV_PREF_CLEAN;
}
