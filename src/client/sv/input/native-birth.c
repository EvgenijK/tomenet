#include "input/native-birth.h"

static bool letter_key(SDL_Keycode key)
{
    return key >= SDLK_A && key <= SDLK_Z;
}

static bool letter_modifiers(const SDL_KeyboardEvent *event)
{
    return !(event->mod & (SDL_KMOD_LEVEL5 | SDL_KMOD_CTRL | SDL_KMOD_ALT |
                           SDL_KMOD_GUI | SDL_KMOD_MODE));
}

static bool symbol_modifiers(const SDL_KeyboardEvent *event)
{
    return !(event->mod & (SDL_KMOD_CTRL | SDL_KMOD_LALT | SDL_KMOD_GUI));
}

static bool uppercase_letter(const SDL_KeyboardEvent *event, SDL_Keycode key)
{
    bool shifted = (event->mod & SDL_KMOD_SHIFT) != 0;
    bool caps_locked = (event->mod & SDL_KMOD_CAPS) != 0;
    return letter_key(key) && shifted != caps_locked;
}

static bool lowercase_letter(const SDL_KeyboardEvent *event, SDL_Keycode key)
{
    return letter_key(key) && letter_modifiers(event) &&
        !uppercase_letter(event, key);
}

static SDL_Keycode layout_key(const SDL_KeyboardEvent *event)
{
    SDL_Keycode translated = SDL_GetKeyFromScancode(
        event->scancode, event->mod, false);
    if (translated >= 'A' && translated <= 'Z')
        translated += 'a' - 'A';
    return translated == SDLK_UNKNOWN ? event->key : translated;
}

static unsigned choice_count(const SvBirth *birth)
{
    switch (birth->owner) {
    case SV_BIRTH_CLASS: return birth->visible_class_count;
    case SV_BIRTH_RACE: return birth->setup->race_count;
    case SV_BIRTH_TRAIT: return birth->legal_trait_count;
    case SV_BIRTH_BODY: return birth->legal_body_count;
    case SV_BIRTH_STATS: return birth->legal_stat_count;
    case SV_BIRTH_MODE: return birth->legal_mode_count;
    default: return 0;
    }
}

static int highlighted_identity(const SvBirth *birth)
{
    switch (birth->owner) {
    case SV_BIRTH_CLASS: return birth->highlighted_class;
    case SV_BIRTH_RACE: return birth->highlighted_race;
    case SV_BIRTH_TRAIT: return birth->highlighted_trait;
    case SV_BIRTH_BODY: return birth->highlighted_body;
    case SV_BIRTH_STATS: return birth->highlighted_stat;
    case SV_BIRTH_MODE: return birth->highlighted_mode;
    default: return -1;
    }
}

static int highlighted_position(const SvBirth *birth)
{
    unsigned count = choice_count(birth);
    int identity = highlighted_identity(birth);
    if (birth->owner == SV_BIRTH_RACE)
        return identity >= 0 && (unsigned)identity < count ? identity : -1;
    for (unsigned position = 0; position < count; ++position) {
        int candidate;
        switch (birth->owner) {
        case SV_BIRTH_CLASS: candidate = birth->visible_classes[position]; break;
        case SV_BIRTH_TRAIT: candidate = birth->legal_traits[position]; break;
        case SV_BIRTH_BODY: candidate = birth->legal_bodies[position]; break;
        case SV_BIRTH_STATS: candidate = birth->legal_stats[position]; break;
        case SV_BIRTH_MODE: candidate = birth->legal_modes[position]; break;
        default: return -1;
        }
        if (candidate == identity) return (int)position;
    }
    return -1;
}

void sv_birth_viewport_sync(SvBirthViewport *viewport, const SvBirth *birth,
                            int logical_height)
{
    if (!viewport || !birth) return;
    unsigned rows = logical_height > SV_BIRTH_CHOICE_ROW_TOP ?
        (unsigned)(logical_height - SV_BIRTH_CHOICE_ROW_TOP) /
            SV_BIRTH_CHOICE_ROW_HEIGHT : 1;
    if (!rows) rows = 1;
    bool owner_changed = viewport->owner != birth->owner;
    bool geometry_changed = viewport->rows != rows;
    int identity = highlighted_identity(birth);
    bool highlight_changed = viewport->highlighted_identity != identity;
    viewport->logical_height = logical_height;
    viewport->rows = rows;
    if (owner_changed) {
        viewport->owner = birth->owner;
        viewport->first = 0;
    }
    unsigned count = choice_count(birth);
    unsigned maximum = count > rows ? count - rows : 0;
    if (viewport->first > maximum) viewport->first = maximum;
    if (owner_changed || geometry_changed || highlight_changed) {
        int position = highlighted_position(birth);
        if (position >= 0 && (unsigned)position < viewport->first)
            viewport->first = (unsigned)position;
        else if (position >= 0 &&
                 (unsigned)position >= viewport->first + rows)
            viewport->first = (unsigned)position - rows + 1;
    }
    viewport->highlighted_identity = identity;
}

static void scroll_viewport(SvBirthViewport *viewport, const SvBirth *birth,
                            int direction)
{
    unsigned count = choice_count(birth);
    unsigned maximum = count > viewport->rows ? count - viewport->rows : 0;
    if (direction < 0 && viewport->first) --viewport->first;
    else if (direction > 0 && viewport->first < maximum) ++viewport->first;
}

static SvBirthCommand select_position(const SvBirth *birth, unsigned position)
{
    switch (birth->owner) {
    case SV_BIRTH_CLASS:
        return (SvBirthCommand){.kind = SV_BIRTH_SELECT_CLASS,
            .class_id = birth->visible_classes[position]};
    case SV_BIRTH_RACE:
        return (SvBirthCommand){.kind = SV_BIRTH_SELECT_RACE,
            .race_id = (int)position};
    case SV_BIRTH_TRAIT:
        return (SvBirthCommand){.kind = SV_BIRTH_SELECT_TRAIT,
            .trait_id = birth->legal_traits[position]};
    case SV_BIRTH_BODY:
        return (SvBirthCommand){.kind = SV_BIRTH_SELECT_BODY,
            .body = birth->legal_bodies[position]};
    case SV_BIRTH_STATS:
        return (SvBirthCommand){.kind = SV_BIRTH_SELECT_STAT,
            .stat = birth->legal_stats[position]};
    case SV_BIRTH_MODE:
        return (SvBirthCommand){.kind = SV_BIRTH_SELECT_MODE,
            .mode = birth->legal_modes[position]};
    default:
        return (SvBirthCommand){.kind = SV_BIRTH_MOVE_NEXT};
    }
}

static bool selectable_position(const SvBirth *birth, unsigned position)
{
    if (birth->owner == SV_BIRTH_CLASS) {
        if (position >= birth->visible_class_count) return false;
        for (unsigned i = 0; i < birth->legal_class_count; ++i)
            if (birth->legal_classes[i] == birth->visible_classes[position])
                return true;
        return false;
    }
    if (birth->owner != SV_BIRTH_RACE) return true;
    for (unsigned i = 0; i < birth->legal_race_count; ++i)
        if (birth->legal_races[i] == position) return true;
    return false;
}

static SvBirthInputResult apply(SvBirth *birth, uint64_t generation,
                                SvBirthViewport *viewport,
                                SvBirthCommand command, bool keyboard, bool mouse)
{
    SvBirthInputResult result = sv_birth_command(birth, generation, command);
    if (result != SV_BIRTH_INPUT_IGNORED) {
        birth->keyboard_navigated |= keyboard;
        birth->mouse_navigated |= mouse;
    }
    if (viewport)
        sv_birth_viewport_sync(viewport, birth, viewport->logical_height);
    return result;
}

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                         uint64_t since_ns,
                                         SvBirthViewport *viewport,
                                         const SDL_Event *event)
{
    if (!birth || !event) return SV_BIRTH_INPUT_IGNORED;
    if (generation != birth->generation || !birth->active || birth->quit)
        return SV_BIRTH_INPUT_IGNORED;
    if (event->common.timestamp && event->common.timestamp < since_ns)
        return SV_BIRTH_INPUT_IGNORED;
    if (event->type == SDL_EVENT_WINDOW_RESIZED && viewport) {
        sv_birth_viewport_sync(viewport, birth, event->window.data2);
        return SV_BIRTH_INPUT_IGNORED;
    }
    if (event->type == SDL_EVENT_MOUSE_WHEEL && viewport) {
        float y = event->wheel.y;
        if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) y = -y;
        if (y < 0.0f) scroll_viewport(viewport, birth, 1);
        else if (y > 0.0f) scroll_viewport(viewport, birth, -1);
        return SV_BIRTH_INPUT_HANDLED;
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event->button.button == SDL_BUTTON_LEFT) {
        unsigned row;
        if (!viewport || event->button.y < SV_BIRTH_CHOICE_ROW_TOP)
            row = (unsigned)-1;
        else row = (unsigned)(event->button.y - SV_BIRTH_CHOICE_ROW_TOP) /
            SV_BIRTH_CHOICE_ROW_HEIGHT;
        unsigned position = viewport ? viewport->first + row : row;
        bool visible = viewport && row < viewport->rows &&
            position < choice_count(birth);
        if (birth->owner == SV_BIRTH_MODE) {
            if (!visible)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_MODE,
                    .mode = birth->legal_modes[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_STATS) {
            if (!visible)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_STAT,
                    .stat = birth->legal_stats[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_BODY) {
            if (!visible)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_BODY,
                    .body = birth->legal_bodies[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_TRAIT) {
            if (!visible)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_TRAIT,
                    .trait_id = birth->legal_traits[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_RACE) {
            if (!visible) return SV_BIRTH_INPUT_IGNORED;
            unsigned id = position;
            SvBirthInputResult selected = apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_RACE,
                    .race_id = (int)id}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_CLASS) {
            if (!visible) return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_CLASS,
                    .class_id = birth->visible_classes[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation, viewport,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner != SV_BIRTH_SEX) return SV_BIRTH_INPUT_IGNORED;
        SvBirthSex sex;
        if (event->button.y >= 300.0f && event->button.y < 350.0f)
            sex = SV_BIRTH_SEX_MALE;
        else if (event->button.y >= 350.0f && event->button.y < 400.0f)
            sex = SV_BIRTH_SEX_FEMALE;
        else return SV_BIRTH_INPUT_IGNORED;
        SvBirthInputResult selected = apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX, .sex = sex}, false, true);
        if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
            return selected;
        return apply(birth, generation, viewport,
                     (SvBirthCommand){.kind = SV_BIRTH_CONFIRM},
                     false, true);
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SV_BIRTH_INPUT_IGNORED;
    SDL_Keycode key = layout_key(&event->key);
    if (key == SDLK_QUESTION && symbol_modifiers(&event->key)) {
        if (birth->owner == SV_BIRTH_SEX ||
            (birth->owner == SV_BIRTH_STATS && !birth->stats_point_buy))
            return SV_BIRTH_INPUT_HANDLED;
        if (birth->owner >= SV_BIRTH_CLASS && birth->owner <= SV_BIRTH_MODE)
            return SV_BIRTH_INPUT_PLACEHOLDER;
    }
    SvBirthCommand command;
    if (event->key.key == SDLK_Q && ((event->key.mod & SDL_KMOD_CTRL) ||
        (letter_modifiers(&event->key) &&
         uppercase_letter(&event->key, event->key.key))))
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_QUIT}, true, false);
    if (birth->owner == SV_BIRTH_MODE && letter_key(key) &&
        letter_modifiers(&event->key)) {
        bool uppercase = uppercase_letter(&event->key, key);
        SvBirthMode mode;
        if (birth->dedicated)
            mode = uppercase && key == SDLK_H ? SV_BIRTH_MODE_IDDC_HELLISH :
                !uppercase && key == SDLK_I ? SV_BIRTH_MODE_IDDC :
                !uppercase && key == SDLK_S ? SV_BIRTH_MODE_IDDC_SOLOIST :
                !uppercase && key == SDLK_P ? SV_BIRTH_MODE_DEDICATED_PVP :
                SV_BIRTH_MODE_NONE;
        else
            mode = uppercase && key == SDLK_H ? SV_BIRTH_MODE_HELLISH :
                !uppercase && key == SDLK_N ? SV_BIRTH_MODE_NORMAL :
                !uppercase && key == SDLK_G ? SV_BIRTH_MODE_NO_GHOST :
                !uppercase && key == SDLK_S ? SV_BIRTH_MODE_SOLOIST :
                !uppercase && key == SDLK_E ? SV_BIRTH_MODE_EVERLASTING :
                !uppercase && key == SDLK_P ? SV_BIRTH_MODE_PVP :
                SV_BIRTH_MODE_NONE;
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_MODE, .mode = mode},
            true, false);
    } else if (birth->owner == SV_BIRTH_STATS &&
        lowercase_letter(&event->key, key)) {
        unsigned stat = (unsigned)(key - SDLK_A);
        if (stat >= 6) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_STAT, .stat = (int)stat},
            true, false);
    } else if (birth->owner == SV_BIRTH_BODY &&
        lowercase_letter(&event->key, key)) {
        SvBirthBody body = key == SDLK_N ? SV_BIRTH_BODY_NORMAL :
            key == SDLK_F ? SV_BIRTH_BODY_FRUIT_BAT : SV_BIRTH_BODY_NONE;
        if (body == SV_BIRTH_BODY_NONE) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_BODY, .body = body},
            true, false);
    } else if (birth->owner == SV_BIRTH_TRAIT &&
        lowercase_letter(&event->key, key)) {
        unsigned position = (unsigned)(key - SDLK_A);
        if (position >= birth->legal_trait_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_TRAIT,
                .trait_id = birth->legal_traits[position]}, true, false);
    } else if (birth->owner == SV_BIRTH_RACE &&
        lowercase_letter(&event->key, key)) {
        unsigned id = (unsigned)(key - SDLK_A);
        if (id >= birth->setup->race_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_RACE,
                .race_id = (int)id}, true, false);
    } else if (birth->owner == SV_BIRTH_CLASS &&
        lowercase_letter(&event->key, key)) {
        unsigned position = (unsigned)(key - SDLK_A);
        if (position >= birth->visible_class_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation, viewport,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_CLASS,
                .class_id = birth->visible_classes[position]}, true, false);
    }
    if (viewport && birth->owner >= SV_BIRTH_CLASS &&
        birth->owner <= SV_BIRTH_MODE &&
        (key == SDLK_PAGEUP || key == SDLK_PAGEDOWN) &&
        choice_count(birth)) {
        int current = highlighted_position(birth);
        unsigned count = choice_count(birth);
        unsigned amount = viewport->rows > 1 ? viewport->rows - 1 : 1;
        unsigned target = current < 0 ? 0 : (unsigned)current;
        if (key == SDLK_PAGEUP)
            target = target > amount ? target - amount : 0;
        else if (target + amount < count) target += amount;
        else target = count - 1;
        int direction = key == SDLK_PAGEUP ? -1 : 1;
        while (!selectable_position(birth, target)) {
            if (direction < 0 && target) --target;
            else if (direction > 0 && target + 1 < count) ++target;
            else break;
        }
        return apply(birth, generation, viewport,
                     select_position(birth, target), true, false);
    }
    if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        key == SDLK_ESCAPE)
        command = (SvBirthCommand){.kind = SV_BIRTH_CONFIRM};
    else if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        (key == SDLK_LEFT || key == SDLK_KP_4 ||
         (key == SDLK_MINUS && symbol_modifiers(&event->key))))
        command = (SvBirthCommand){.kind = SV_BIRTH_DECREASE_STAT};
    else if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        (key == SDLK_RIGHT || key == SDLK_KP_6 ||
         (key == SDLK_PLUS && symbol_modifiers(&event->key))))
        command = (SvBirthCommand){.kind = SV_BIRTH_INCREASE_STAT};
    else if (key == SDLK_UP || key == SDLK_LEFT || key == SDLK_KP_8 || key == SDLK_KP_4)
        command = (SvBirthCommand){.kind = SV_BIRTH_MOVE_PREVIOUS};
    else if (key == SDLK_DOWN || key == SDLK_RIGHT || key == SDLK_KP_2 || key == SDLK_KP_6)
        command = (SvBirthCommand){.kind = SV_BIRTH_MOVE_NEXT};
    else if (key == SDLK_M && lowercase_letter(&event->key, key))
        command = (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX,
            .sex = SV_BIRTH_SEX_MALE};
    else if (key == SDLK_F && lowercase_letter(&event->key, key))
        command = (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX,
            .sex = SV_BIRTH_SEX_FEMALE};
    else if ((key == SDLK_ASTERISK || key == SDLK_KP_MULTIPLY) &&
        symbol_modifiers(&event->key))
        command = (SvBirthCommand){.kind = SV_BIRTH_RANDOM};
    else if (key == SDLK_HASH && symbol_modifiers(&event->key))
        command = (SvBirthCommand){.kind = SV_BIRTH_DNA};
    else if (key == SDLK_PERCENT && symbol_modifiers(&event->key))
        command = (SvBirthCommand){.kind = SV_BIRTH_AUTOMATIC_DNA};
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        command = (SvBirthCommand){.kind = SV_BIRTH_CONFIRM};
    else if (key == SDLK_BACKSPACE)
        command = (SvBirthCommand){.kind = SV_BIRTH_BACK};
    else {
        birth->illegal_ignored = true;
        return SV_BIRTH_INPUT_IGNORED;
    }
    return apply(birth, generation, viewport, command, true, false);
}
