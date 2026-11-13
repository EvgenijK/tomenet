/* Original prototype SVG geometry. See hud-icons/sources.json. */
#ifndef SV_HUD_ICONS_H
#define SV_HUD_ICONS_H
#include <SDL3/SDL.h>
typedef enum {
    SV_HUD_ICON_ATTR_STRENGTH,
    SV_HUD_ICON_ATTR_INTELLIGENCE,
    SV_HUD_ICON_ATTR_WISDOM,
    SV_HUD_ICON_ATTR_DEXTERITY,
    SV_HUD_ICON_ATTR_CONSTITUTION,
    SV_HUD_ICON_ATTR_CHARISMA,
    SV_HUD_ICON_SANITY,
    SV_HUD_ICON_ARMOUR,
    SV_HUD_ICON_CHARACTER,
    SV_HUD_ICON_ENC_EASY_WIELD,
    SV_HUD_ICON_ENC_HEAVY_WIELD,
    SV_HUD_ICON_ENC_AWKWARD_WIELD,
    SV_HUD_ICON_ENC_ICKY_WIELD,
    SV_HUD_ICON_ENC_HEAVY_TOOL,
    SV_HUD_ICON_ENC_HEAVY_SHIELD,
    SV_HUD_ICON_ENC_HEAVY_SHOOT,
    SV_HUD_ICON_ENC_AWKWARD_SHOOT,
    SV_HUD_ICON_ENC_CUMBER_ARMOR,
    SV_HUD_ICON_ENC_MONK_HEAVYARMOR,
    SV_HUD_ICON_ENC_CUMBER_WEIGHT,
    SV_HUD_ICON_ENC_ROGUE_HEAVYARMOR,
    SV_HUD_ICON_ENC_AWKWARD_ARMOR,
    SV_HUD_ICON_ENC_CUMBER_GLOVE,
    SV_HUD_ICON_ENC_HEAVY_SWIM,
    SV_HUD_ICON_STATUS_SPEED,
    SV_HUD_ICON_STATUS_BPR,
    SV_HUD_ICON_STATUS_FIRE,
    SV_HUD_ICON_STATUS_COLD,
    SV_HUD_ICON_STATUS_ELEC,
    SV_HUD_ICON_STATUS_ACID,
    SV_HUD_ICON_STATUS_POISON,
    SV_HUD_ICON_STATUS_MANA,
    SV_HUD_ICON_STATUS_ESP,
    SV_HUD_ICON_STATUS_BRAND,
    SV_HUD_ICON_STATUS_REGEN,
    SV_HUD_ICON_STATUS_DISPERSION,
    SV_HUD_ICON_STATUS_CHARM,
    SV_HUD_ICON_STATUS_THUNDER,
    SV_HUD_ICON_STATUS_PFE,
    SV_HUD_ICON_STATUS_CRIT,
    SV_HUD_ICON_STATUS_SHIELD,
    SV_HUD_ICON_STATUS_MUSHROOM,
    SV_HUD_ICON_STATUS_FOOD,
    SV_HUD_ICON_STATUS_BLIND,
    SV_HUD_ICON_STATUS_HALLU,
    SV_HUD_ICON_STATUS_CONFUSED,
    SV_HUD_ICON_STATUS_FEAR,
    SV_HUD_ICON_STATUS_DISEASE,
    SV_HUD_ICON_STATUS_SEARCH,
    SV_HUD_ICON_STATUS_REST,
    SV_HUD_ICON_STATUS_PARALYSIS,
    SV_HUD_ICON_STATUS_WOUND,
    SV_HUD_ICON_STATUS_STUN,
    SV_HUD_ICON_SOURCE_WEAPON,
    SV_HUD_ICON_SOURCE_SHOOTER,
    SV_HUD_ICON_SOURCE_RING,
    SV_HUD_ICON_SOURCE_AMULET,
    SV_HUD_ICON_SOURCE_LIGHT,
    SV_HUD_ICON_SOURCE_BODY,
    SV_HUD_ICON_SOURCE_CLOAK,
    SV_HUD_ICON_SOURCE_SHIELD,
    SV_HUD_ICON_SOURCE_HAT,
    SV_HUD_ICON_SOURCE_GLOVES,
    SV_HUD_ICON_SOURCE_BOOTS,
    SV_HUD_ICON_SOURCE_QUIVER,
    SV_HUD_ICON_SOURCE_TOOL,
    SV_HUD_ICON_SOURCE_PLAYER,
    SV_HUD_ICON_COUNT
} SvHudIcon;
/* Original SVG strokes are rasterized at destination physical size and phase.
 * Cached textures are copied 1:1 and owned by the renderer. */
bool sv_hud_icon_draw(SDL_Renderer *, SvHudIcon, SDL_FRect destination, SDL_Color);
/* CSS drop-shadow: sigma and offset are physical renderer pixels. */
bool sv_hud_icon_shadow_draw(SDL_Renderer *, SvHudIcon, SDL_FRect destination,
    SDL_Color, float sigma_pixels, float offset_x_pixels, float offset_y_pixels);
typedef struct {
    SDL_Color colour;
    float sigma_pixels, offset_x_pixels, offset_y_pixels;
} SvHudIconShadow;
/* CSS filter chain: each drop-shadow uses the preceding image's alpha. Draws
 * the final filtered icon, including its original foreground. At most 2 stages. */
bool sv_hud_icon_filter_draw(SDL_Renderer *, SvHudIcon, SDL_FRect destination,
    SDL_Color foreground, const SvHudIconShadow *shadows, size_t count);
#endif
