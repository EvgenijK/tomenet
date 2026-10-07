#include "input/native-birth.h"

static SvBirthInputResult apply(SvBirth *birth, uint64_t generation,
                                SvBirthCommand command, bool keyboard, bool mouse)
{
    SvBirthInputResult result = sv_birth_command(birth, generation, command);
    if (result != SV_BIRTH_INPUT_IGNORED) {
        birth->keyboard_navigated |= keyboard;
        birth->mouse_navigated |= mouse;
    }
    return result;
}

SvBirthInputResult sv_native_birth_event(SvBirth *birth, uint64_t generation,
                                         uint64_t since_ns,
                                         const SDL_Event *event)
{
    if (!birth || !event) return SV_BIRTH_INPUT_IGNORED;
    if (generation != birth->generation || !birth->active || birth->quit)
        return SV_BIRTH_INPUT_IGNORED;
    if (event->common.timestamp && event->common.timestamp < since_ns)
        return SV_BIRTH_INPUT_IGNORED;
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event->button.button == SDL_BUTTON_LEFT) {
        if (birth->owner == SV_BIRTH_MODE) {
            if (event->button.y < 320.0f) return SV_BIRTH_INPUT_IGNORED;
            unsigned position = (unsigned)((event->button.y - 320.0f) / 40.0f);
            if (position >= birth->legal_mode_count)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_MODE,
                    .mode = birth->legal_modes[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_STATS) {
            if (event->button.y < 320.0f) return SV_BIRTH_INPUT_IGNORED;
            unsigned position = (unsigned)((event->button.y - 320.0f) / 40.0f);
            if (position >= birth->legal_stat_count)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_STAT,
                    .stat = birth->legal_stats[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_BODY) {
            if (event->button.y < 320.0f) return SV_BIRTH_INPUT_IGNORED;
            unsigned position = (unsigned)((event->button.y - 320.0f) / 40.0f);
            if (position >= birth->legal_body_count)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_BODY,
                    .body = birth->legal_bodies[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_TRAIT) {
            if (event->button.y < 320.0f) return SV_BIRTH_INPUT_IGNORED;
            unsigned position = (unsigned)((event->button.y - 320.0f) / 40.0f);
            if (position >= birth->legal_trait_count)
                return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_TRAIT,
                    .trait_id = birth->legal_traits[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_RACE) {
            if (event->button.y < 320.0f) return SV_BIRTH_INPUT_IGNORED;
            unsigned id = (unsigned)((event->button.y - 320.0f) / 40.0f);
            if (id >= birth->setup->race_count) return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_RACE,
                    .race_id = (int)id}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner == SV_BIRTH_CLASS) {
            if (event->button.y < 320.0f) return SV_BIRTH_INPUT_IGNORED;
            unsigned position = (unsigned)((event->button.y - 320.0f) / 40.0f);
            if (position >= birth->legal_class_count) return SV_BIRTH_INPUT_IGNORED;
            SvBirthInputResult selected = apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_SELECT_CLASS,
                    .class_id = birth->legal_classes[position]}, false, true);
            if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
                return selected;
            return apply(birth, generation,
                (SvBirthCommand){.kind = SV_BIRTH_CONFIRM}, false, true);
        }
        if (birth->owner != SV_BIRTH_SEX) return SV_BIRTH_INPUT_IGNORED;
        SvBirthSex sex;
        if (event->button.y >= 300.0f && event->button.y < 350.0f)
            sex = SV_BIRTH_SEX_MALE;
        else if (event->button.y >= 350.0f && event->button.y < 400.0f)
            sex = SV_BIRTH_SEX_FEMALE;
        else return SV_BIRTH_INPUT_IGNORED;
        SvBirthInputResult selected = apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX, .sex = sex}, false, true);
        if (selected == SV_BIRTH_INPUT_IGNORED || event->button.clicks < 2)
            return selected;
        return apply(birth, generation,
                     (SvBirthCommand){.kind = SV_BIRTH_CONFIRM},
                     false, true);
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SV_BIRTH_INPUT_IGNORED;
    SDL_Keycode key = event->key.key;
    if (key == SDLK_QUESTION && birth->owner >= SV_BIRTH_SEX &&
        birth->owner <= SV_BIRTH_MODE)
        return SV_BIRTH_INPUT_PLACEHOLDER;
    SvBirthCommand command;
    if (birth->owner == SV_BIRTH_MODE && key != SDLK_Q &&
        key >= SDLK_A && key <= SDLK_Z) {
        SvBirthMode mode;
        if (birth->dedicated)
            mode = key == SDLK_I ? SV_BIRTH_MODE_IDDC :
                key == SDLK_S ? SV_BIRTH_MODE_IDDC_SOLOIST :
                key == SDLK_H ? SV_BIRTH_MODE_IDDC_HELLISH :
                key == SDLK_P ? SV_BIRTH_MODE_DEDICATED_PVP : SV_BIRTH_MODE_NONE;
        else
            mode = key == SDLK_N ? SV_BIRTH_MODE_NORMAL :
                key == SDLK_G ? SV_BIRTH_MODE_NO_GHOST :
                key == SDLK_S ? SV_BIRTH_MODE_SOLOIST :
                key == SDLK_H ? SV_BIRTH_MODE_HELLISH :
                key == SDLK_E ? SV_BIRTH_MODE_EVERLASTING :
                key == SDLK_P ? SV_BIRTH_MODE_PVP : SV_BIRTH_MODE_NONE;
        return apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_MODE, .mode = mode},
            true, false);
    } else if (birth->owner == SV_BIRTH_STATS && key != SDLK_Q &&
        key >= SDLK_A && key <= SDLK_Z) {
        unsigned stat = (unsigned)(key - SDLK_A);
        if (stat >= 6) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_STAT, .stat = (int)stat},
            true, false);
    } else if (birth->owner == SV_BIRTH_BODY && key != SDLK_Q &&
        key >= SDLK_A && key <= SDLK_Z) {
        SvBirthBody body = key == SDLK_N ? SV_BIRTH_BODY_NORMAL :
            key == SDLK_F ? SV_BIRTH_BODY_FRUIT_BAT : SV_BIRTH_BODY_NONE;
        if (body == SV_BIRTH_BODY_NONE) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_BODY, .body = body},
            true, false);
    } else if (birth->owner == SV_BIRTH_TRAIT && key != SDLK_Q &&
        key >= SDLK_A && key <= SDLK_Z) {
        unsigned position = (unsigned)(key - SDLK_A);
        if (position >= birth->legal_trait_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_TRAIT,
                .trait_id = birth->legal_traits[position]}, true, false);
    } else if (birth->owner == SV_BIRTH_RACE && key != SDLK_Q &&
        key >= SDLK_A && key <= SDLK_Z) {
        unsigned id = (unsigned)(key - SDLK_A);
        if (id >= birth->setup->race_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_RACE,
                .race_id = (int)id}, true, false);
    } else if (birth->owner == SV_BIRTH_CLASS && key != SDLK_Q &&
        key >= SDLK_A && key <= SDLK_Z) {
        unsigned position = (unsigned)(key - SDLK_A);
        if (position >= birth->legal_class_count) {
            birth->illegal_ignored = true;
            return SV_BIRTH_INPUT_IGNORED;
        }
        return apply(birth, generation,
            (SvBirthCommand){.kind = SV_BIRTH_SELECT_CLASS,
                .class_id = birth->legal_classes[position]}, true, false);
    }
    if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        key == SDLK_ESCAPE)
        command = (SvBirthCommand){.kind = SV_BIRTH_CONFIRM};
    else if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        (key == SDLK_LEFT || key == SDLK_KP_4 || key == SDLK_MINUS))
        command = (SvBirthCommand){.kind = SV_BIRTH_DECREASE_STAT};
    else if (birth->owner == SV_BIRTH_STATS && birth->stats_point_buy &&
        (key == SDLK_RIGHT || key == SDLK_KP_6 || key == SDLK_PLUS))
        command = (SvBirthCommand){.kind = SV_BIRTH_INCREASE_STAT};
    else if (key == SDLK_UP || key == SDLK_LEFT || key == SDLK_KP_8 || key == SDLK_KP_4)
        command = (SvBirthCommand){.kind = SV_BIRTH_MOVE_PREVIOUS};
    else if (key == SDLK_DOWN || key == SDLK_RIGHT || key == SDLK_KP_2 || key == SDLK_KP_6)
        command = (SvBirthCommand){.kind = SV_BIRTH_MOVE_NEXT};
    else if (key == SDLK_M)
        command = (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX,
            .sex = SV_BIRTH_SEX_MALE};
    else if (key == SDLK_F)
        command = (SvBirthCommand){.kind = SV_BIRTH_SELECT_SEX,
            .sex = SV_BIRTH_SEX_FEMALE};
    else if (key == SDLK_ASTERISK || key == SDLK_KP_MULTIPLY)
        command = (SvBirthCommand){.kind = SV_BIRTH_RANDOM};
    else if (key == SDLK_HASH)
        command = (SvBirthCommand){.kind = SV_BIRTH_DNA};
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        command = (SvBirthCommand){.kind = SV_BIRTH_CONFIRM};
    else if (key == SDLK_BACKSPACE)
        command = (SvBirthCommand){.kind = SV_BIRTH_BACK};
    else if (key == SDLK_Q || ((event->key.mod & SDL_KMOD_CTRL) && key == SDLK_Q))
        command = (SvBirthCommand){.kind = SV_BIRTH_QUIT};
    else {
        birth->illegal_ignored = true;
        return SV_BIRTH_INPUT_IGNORED;
    }
    return apply(birth, generation, command, true, false);
}
