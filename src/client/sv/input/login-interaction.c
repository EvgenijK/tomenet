#include "input/login-interaction.h"
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define SV_MODE_DED_IDDC 0x40
#define SV_MODE_DED_PVP 0x80
#define SV_SFLAG0_RPG 0x01u
#define SV_SFLAG0_ARCADE 0x08u
#define SV_SFLAG0_RPG_ADMIN 0x20u
#define SV_SFLAG0_DED_IDDC 0x40u
#define SV_SFLAG0_DED_PVP 0x80u

typedef struct {
    size_t ordinary_limit, baseline_ordinary_limit;
    size_t iddc_limit, pvp_limit, total_limit;
    size_t ordinary_count, iddc_count, pvp_count;
    bool iddc_supported, pvp_supported;
} SvCreationAvailability;

static bool version_newer(const int version[6], const int threshold[6])
{
    for (int i = 0; i < 6; ++i)
        if (version[i] != threshold[i]) return version[i] > threshold[i];
    return false;
}

static bool version_older(const int version[6], const int threshold[6])
{
    return version_newer(threshold, version);
}

static bool creation_availability(const SvLoginInteraction *input,
                                  SvCreationAvailability *available)
{
    if (!input || !input->login || sv_login_state(input->login) != SV_LOGIN_OVERVIEW)
        return false;
    const uint32_t *flags = sv_login_flags(input->login);
    const int *version = sv_login_version(input->login);
    uint32_t limits = flags[0], server = flags[3];
    *available = (SvCreationAvailability){
        .ordinary_limit = limits & 0xffu,
        .iddc_limit = (limits >> 8) & 0xffu,
        .pvp_limit = (limits >> 16) & 0xffu
    };
    if (!available->ordinary_limit) {
        available->ordinary_limit = 8;
        if (!(limits & 0xff00u)) available->iddc_limit = 2;
        if (!(limits & 0xff0000u)) available->pvp_limit = 1;
    }

    const int dynamic_dedicated_since[6] = {4, 5, 8, 1, 0, 0};
    const int encoded_limits_since[6] = {4, 9, 0, 5, 0, 0};
    bool arcade = (server & SV_SFLAG0_ARCADE) != 0;
    bool rpg = (server & SV_SFLAG0_RPG) != 0;
    if (version_older(version, encoded_limits_since) && (arcade || rpg)) {
        if (rpg && !(server & SV_SFLAG0_RPG_ADMIN)) available->ordinary_limit = 1;
        available->iddc_limit = available->pvp_limit = 0;
    }

    size_t dedicated_capacity = 0;
    if (version_newer(version, dynamic_dedicated_since)) {
        available->iddc_supported = (server & SV_SFLAG0_DED_IDDC) != 0;
        available->pvp_supported = (server & SV_SFLAG0_DED_PVP) != 0;
    } else if (!arcade && !rpg) {
        available->iddc_supported = (server & SV_SFLAG0_DED_IDDC) != 0;
        available->pvp_supported = (server & SV_SFLAG0_DED_PVP) != 0;
        if (available->iddc_supported) available->iddc_limit = 1;
        if (available->pvp_supported) available->pvp_limit = 1;
    }
    if (available->iddc_supported) dedicated_capacity += available->iddc_limit;
    if (available->pvp_supported) dedicated_capacity += available->pvp_limit;

    available->total_limit = available->ordinary_limit + dedicated_capacity;
    if (available->total_limit > SV_LOGIN_MAX_CHARACTERS)
        available->total_limit = SV_LOGIN_MAX_CHARACTERS;
    available->baseline_ordinary_limit = available->ordinary_limit;
    if (available->baseline_ordinary_limit > SV_LOGIN_MAX_CHARACTERS)
        available->baseline_ordinary_limit = SV_LOGIN_MAX_CHARACTERS;
    for (size_t slot = 0; slot < sv_login_count(input->login); ++slot) {
        int16_t mode = sv_login_character(input->login, slot)->mode;
        if (mode & SV_MODE_DED_IDDC) ++available->iddc_count;
        else if (mode & SV_MODE_DED_PVP) ++available->pvp_count;
        else ++available->ordinary_count;
    }
    return true;
}

bool sv_login_interaction_ordinary_available(const SvLoginInteraction *input)
{
    SvCreationAvailability available;
    if (!creation_availability(input, &available)) return false;
    /* Baseline first-run state restricts exclusive E, not ordinary N. */
    return sv_login_count(input->login) < available.total_limit &&
           available.ordinary_count < available.baseline_ordinary_limit;
}

uint32_t sv_login_interaction_exclusive_modes(const SvLoginInteraction *input)
{
    SvCreationAvailability available;
    if (!creation_availability(input, &available) || input->first_run ||
        sv_login_count(input->login) >= available.total_limit)
        return 0;
    uint32_t modes = 0;
    if (available.iddc_supported && available.iddc_count < available.iddc_limit)
        modes |= SV_SFLAG0_DED_IDDC;
    if (available.pvp_supported && available.pvp_count < available.pvp_limit)
        modes |= SV_SFLAG0_DED_PVP;
    return modes;
}

void sv_login_interaction_begin(SvLoginInteraction *input, SvLogin *login,
                                const char *default_character, bool skip_motd,
                                bool first_run, uint32_t creation_flags)
{
    *input = (SvLoginInteraction){.login = login, .default_character = default_character,
                                   .skip_motd = skip_motd, .first_run = first_run,
                                   .creation_supported = creation_flags <= 1};
}

SvLoginTransition sv_login_interaction_sync(SvLoginInteraction *input)
{
    SvLoginState state = sv_login_state(input->login);
    if (state == SV_LOGIN_OVERVIEW && !input->overview_seen) {
        input->overview_seen = true;
        if (input->default_character) {
            char desired[SV_LOGIN_NAME_CAPACITY];
            size_t length = strlen(input->default_character);
            if (length >= sizeof(desired)) return SV_LOGIN_TRANSITION_OVERVIEW;
            memcpy(desired, input->default_character, length + 1);
            desired[0] = (char)toupper((unsigned char)desired[0]);
            for (size_t slot = 0; slot < sv_login_count(input->login); ++slot)
                if (!strcmp(desired, sv_login_character(input->login, slot)->name)) {
                    (void)sv_login_choose(input->login, slot);
                    break;
                }
        }
        return SV_LOGIN_TRANSITION_OVERVIEW;
    }
    if (state == SV_LOGIN_SELECTED && !input->selected_seen) {
        input->selected_seen = true;
        if (sv_login_creating_character(input->login))
            return SV_LOGIN_TRANSITION_BIRTH;
        input->motd_complete = input->skip_motd;
        return SV_LOGIN_TRANSITION_MOTD;
    }
    return SV_LOGIN_TRANSITION_NONE;
}

static bool name_byte(unsigned char byte)
{
    return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') ||
           (byte >= '0' && byte <= '9') || strchr(" .,-'&_$%~#", byte);
}

static uint32_t next_random(uint32_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

static void random_name(char out[SV_LOGIN_NAME_CAPACITY])
{
    static const char *const first[] = {
        "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar",
        "B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et",
        "Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha",
        "Ib", "Jer", "K", "Ka", "Ked", "L", "Loth", "Lar", "Leg", "M", "Mir",
        "N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T",
        "Tr", "Th", "V", "Y", "Z", "W", "Wic"
    };
    static const char *const middle[] = {
        "a", "ae", "au", "ao", "are", "ale", "ali", "ay", "ardo", "e", "ei",
        "ea", "eri", "era", "ela", "eli", "enda", "erra", "i", "ia", "ie",
        "ire", "ira", "ila", "ili", "ira", "igo", "o", "oa", "oi", "oe",
        "ore", "u", "y"
    };
    static const char *const last[] = {
        "a", "and", "b", "bwyn", "baen", "bard", "c", "ctred", "cred", "ch",
        "can", "d", "dan", "don", "der", "dric", "dfrid", "dus", "f", "g",
        "gord", "gan", "l", "li", "lgrin", "lin", "lith", "lath", "loth",
        "ld", "ldric", "ldan", "m", "mas", "mos", "mar", "mond", "n",
        "nydd", "nidd", "nnon", "nwan", "nyth", "nad", "nn", "nnor", "nd",
        "p", "r", "ron", "rd", "s", "sh", "seth", "sean", "t", "th", "tha",
        "tlan", "trem", "tram", "v", "vudd", "w", "wan", "win", "wyn", "wyr",
        "wyr", "wyth"
    };
    static uint32_t random_state;
    if (!random_state) {
        random_state = (uint32_t)time(NULL) ^ (uint32_t)clock() ^
            (uint32_t)(uintptr_t)out;
        if (!random_state) random_state = 0x6d2b79f5u;
    }
    const char *a = first[next_random(&random_state) %
        (sizeof(first) / sizeof(first[0]))];
    const char *b = middle[next_random(&random_state) %
        (sizeof(middle) / sizeof(middle[0]))];
    const char *c = last[next_random(&random_state) %
        (sizeof(last) / sizeof(last[0]))];
    snprintf(out, SV_LOGIN_NAME_CAPACITY, "%s%s%s", a, b, c);
}

SvTextResult sv_login_interaction_name_insert(SvLoginInteraction *input,
                                               const char *utf8)
{
    if (!input || !input->ordinary_name_active || !utf8) return SV_TEXT_INVALID;
    SvTextField decoded;
    sv_text_begin(&decoded, "", SV_TEXT_CAPACITY, false);
    SvTextResult decoded_result = sv_text_insert_utf8(&decoded, utf8);
    if (decoded_result == SV_TEXT_ENCODING_ERROR || decoded_result == SV_TEXT_INVALID)
        return decoded_result;
    SvTextResult result = SV_TEXT_OK;
    for (size_t i = 0; i < decoded.length; ++i) {
        unsigned char byte = (unsigned char)decoded.bytes[i];
        size_t start = input->character_name.cursor < input->character_name.anchor ?
            input->character_name.cursor : input->character_name.anchor;
        if (!start) {
            if (byte >= 'a' && byte <= 'z') byte = (unsigned char)(byte - 'a' + 'A');
            if (!((byte >= 'A' && byte <= 'Z') || byte == '+')) continue;
        } else if (!name_byte(byte)) byte = '_';
        char insertion[2] = {(char)byte, 0};
        SvTextResult inserted = sv_text_insert_utf8(&input->character_name, insertion);
        if (inserted != SV_TEXT_OK) result = inserted;
    }
    if (input->character_name.length > input->name_maximum_draft_bytes)
        input->name_maximum_draft_bytes = input->character_name.length;
    if (result == SV_TEXT_TRUNCATED) input->name_input_truncated = true;
    return result;
}

SvLoginInputResult sv_login_interaction_name_key(SvLoginInteraction *input,
                                                  SvEditKey key)
{
    if (!input || !input->ordinary_name_active) return SV_LOGIN_INPUT_IGNORED;
    if (key == SV_EDIT_CANCEL) {
        sv_text_key(&input->character_name, key);
        input->ordinary_name_active = false;
        input->dedicated_creation = false;
        input->dedicated_mode_flags = 0;
        return SV_LOGIN_INPUT_NAME_CANCELLED;
    }
    if (key != SV_EDIT_ACCEPT) {
        sv_text_key(&input->character_name, key);
        return SV_LOGIN_INPUT_HANDLED;
    }
    char submitted[SV_LOGIN_NAME_CAPACITY];
    const char *source = input->character_name.bytes;
    bool reincarnation = source[0] == '+';
    if (reincarnation) ++source;
    while (*source == ' ') ++source;
    size_t length = strlen(source);
    while (length && source[length - 1] == ' ') --length;
    if (!length) {
        char suggestion[SV_LOGIN_NAME_CAPACITY];
        random_name(suggestion);
        char draft[SV_LOGIN_NAME_CAPACITY];
        if (reincarnation)
            snprintf(draft, sizeof(draft), "+%s", suggestion);
        else
            snprintf(draft, sizeof(draft), "%s", suggestion);
        sv_text_begin(&input->character_name, draft, 15, false);
        if (input->character_name.length > input->name_maximum_draft_bytes)
            input->name_maximum_draft_bytes = input->character_name.length;
        return SV_LOGIN_INPUT_NAME_SUGGESTED;
    }
    if (length >= sizeof(submitted)) return SV_LOGIN_INPUT_IGNORED;
    memcpy(submitted, source, length);
    submitted[length] = 0;
    if (submitted[0] >= 'a' && submitted[0] <= 'z')
        submitted[0] = (char)(submitted[0] - 'a' + 'A');
    if (sv_login_create_character(input->login, submitted, reincarnation) != SV_OK)
        return SV_LOGIN_INPUT_IGNORED;
    input->ordinary_name_active = false;
    return SV_LOGIN_INPUT_NAME_SUBMITTED;
}

SvLoginInputResult sv_login_interaction_restore_name(SvLoginInteraction *input,
                                                      const char *draft,
                                                      bool dedicated_creation)
{
    if (!input || !draft || strlen(draft) > 15 ||
        sv_login_interaction_command(input,
            (SvLoginCommand){dedicated_creation ?
                SV_LOGIN_COMMAND_CREATE_EXCLUSIVE : SV_LOGIN_COMMAND_CREATE_ORDINARY, 0}) !=
            SV_LOGIN_INPUT_NAME_STARTED)
        return SV_LOGIN_INPUT_IGNORED;
    sv_text_begin(&input->character_name, draft, 15, false);
    input->name_maximum_draft_bytes = input->character_name.length;
    return SV_LOGIN_INPUT_NAME_STARTED;
}

SvLoginInputResult sv_login_interaction_command(SvLoginInteraction *input,
                                                 SvLoginCommand command)
{
    SvLoginState state = sv_login_state(input->login);
    if (input->ordinary_name_active) {
        if (command.kind != SV_LOGIN_COMMAND_CANCEL) return SV_LOGIN_INPUT_IGNORED;
        sv_text_key(&input->character_name, SV_EDIT_CANCEL);
        input->ordinary_name_active = false;
        input->dedicated_creation = false;
        input->dedicated_mode_flags = 0;
        return SV_LOGIN_INPUT_NAME_CANCELLED;
    }
    if (state == SV_LOGIN_OVERVIEW) {
        if (command.kind == SV_LOGIN_COMMAND_QUIT) return SV_LOGIN_INPUT_QUIT;
        if ((command.kind == SV_LOGIN_COMMAND_CREATE_ORDINARY ||
             command.kind == SV_LOGIN_COMMAND_CREATE_EXCLUSIVE) &&
            !input->creation_supported)
            return SV_LOGIN_INPUT_CREATION_UNSUPPORTED;
        if (command.kind == SV_LOGIN_COMMAND_CREATE_ORDINARY &&
            sv_login_interaction_ordinary_available(input)) {
            sv_text_begin(&input->character_name, "", 15, false);
            input->ordinary_name_active = true;
            input->dedicated_creation = false;
            input->dedicated_mode_flags = 0;
            return SV_LOGIN_INPUT_NAME_STARTED;
        }
        uint32_t dedicated_modes = sv_login_interaction_exclusive_modes(input);
        if (command.kind == SV_LOGIN_COMMAND_CREATE_EXCLUSIVE && dedicated_modes) {
            sv_text_begin(&input->character_name, "", 15, false);
            input->ordinary_name_active = true;
            input->dedicated_creation = true;
            input->dedicated_mode_flags = dedicated_modes;
            return SV_LOGIN_INPUT_NAME_STARTED;
        }
        if (command.kind == SV_LOGIN_COMMAND_CHOOSE &&
            sv_login_choose(input->login, command.slot) == SV_OK)
            return SV_LOGIN_INPUT_HANDLED;
    } else if (state == SV_LOGIN_SELECTED && !input->motd_complete) {
        input->motd_complete = true;
        return SV_LOGIN_INPUT_HANDLED;
    }
    return SV_LOGIN_INPUT_IGNORED;
}

bool sv_login_interaction_complete(const SvLoginInteraction *input)
{
    return input->motd_complete && sv_login_state(input->login) == SV_LOGIN_SELECTED;
}
