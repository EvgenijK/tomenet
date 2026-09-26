#include "input/command.h"
#include "protocol/protocol.h"
#include "../../common/pack.h"
#include <string.h>

static unsigned char control(unsigned char key) { return key & 31; }
static unsigned char direction(unsigned char key, bool rogue)
{
    switch (key) {
    case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9': return key - '0';
    case 'b': return rogue ? 1 : 0;
    case 'j': return rogue ? 2 : 0;
    case 'n': return rogue ? 3 : 0;
    case 'h': return rogue ? 4 : 0;
    case 'l': return rogue ? 6 : 0;
    case 'y': return rogue ? 7 : 0;
    case 'k': return rogue ? 8 : 0;
    case 'u': return rogue ? 9 : 0;
    default: return 0;
    }
}

static unsigned char keymap(unsigned char key, bool rogue, unsigned char *dir)
{
    *dir = 0;
    if (key == control('M') || (!rogue && key == control('J'))) return 13;
    if (!rogue) {
        if (key == 'T') return '+';
        if (key == ',' || key == '5') { *dir = 5; return ','; }
        *dir = direction(key, false);
        return *dir ? ';' : key;
    }
    if ((key >= '1' && key <= '9') || strchr("bjn hlyku", key)) {
        *dir = direction(key, true);
        if (*dir) return *dir == 5 ? ',' : ';';
    }
    if (strchr("BJNHLYKU", key)) {
        *dir = direction(key + ('a' - 'A'), true);
        if (*dir) return '.';
    }
    if (key == control('B') || key == control('J') || key == control('N') ||
        key == control('H') || key == control('L') || key == control('Y') ||
        key == control('K') || key == control('U')) {
        static const unsigned char keys[] = {'B','J','N','H','L','Y','K','U'};
        static const unsigned char dirs[] = {1,2,3,4,6,7,8,9};
        for (size_t i = 0; i < sizeof(keys); ++i)
            if (key == control(keys[i])) { *dir = dirs[i]; return '+'; }
    }
    switch (key) {
    case 6: return 21; /* Ctrl-F -> Ctrl-U */
    case 22: return 14; /* Ctrl-V -> Ctrl-N */
    case 3: return 'K';
    case 4: return 'k';
    case 24: return 'U';
    case 23: return 'L';
    case 'P': return 'b';
    case 1: return 'j';
    case '#': return 'S';
    case 'Z': return 'u';
    case 'T': return 't';
    case 't': return 'f';
    case 'f': return 'B';
    case 'x': return 'l';
    case 'z': return 'a';
    case 'a': return 'z';
    case 'O': return 'P';
    case 'S': return 'x';
    case 5: return 'h';
    case 7: return 'H';
    case 26: return 7;
    case ',': return '.';
    case '.': case '5': *dir = 5; return ',';
    default: return key;
    }
}

SvResult sv_command_override(SvCommandRouter *router, unsigned char key,
                             unsigned char command, unsigned char direction_value)
{
    if (!router || !key || key >= 128 || !command || command >= 128 ||
        direction_value > 9 || direction_value == 5) return SV_INVALID;
    router->override[key] = true;
    router->override_command[key] = command;
    router->override_direction[key] = direction_value;
    return SV_OK;
}

SvResult sv_command_key(SvCommandRouter *router, unsigned char key, SvCommand *output)
{
    if (!router || !output) return SV_INVALID;
    *output = (SvCommand){0};
    if (router->chat) {
        if (key == 27) { router->chat = false; router->text_size = 0; return SV_OK; }
        if (key == 13 || key == 10) {
            output->kind = SV_COMMAND_CHAT;
            output->size = router->text_size;
            memcpy(output->text, router->text, output->size);
            router->chat = false; router->text_size = 0;
            return SV_OK;
        }
        if (key == 8) { if (router->text_size) --router->text_size; return SV_OK; }
        if (key < 32 || router->text_size == SV_COMMAND_TEXT - 1) return SV_INVALID;
        router->text[router->text_size++] = key;
        return SV_OK;
    }
    if (router->direction_pending) {
        SvCommandKind kind = router->direction_pending;
        router->direction_pending = SV_COMMAND_NONE;
        if (key == 27) return SV_OK;
        unsigned char dir = direction(key, router->roguelike);
        if (!dir) return SV_INVALID;
        output->kind = kind; output->direction = dir;
        return SV_OK;
    }
    if (key == '\\' && !router->bypass && !router->control) {
        router->bypass = true; return SV_OK;
    }
    if (key == '^' && !router->control) {
        router->control = true; return SV_OK;
    }
    if (router->control) { key = control(key); router->control = false; }
    unsigned char dir = 0;
    if (router->bypass) router->bypass = false;
    else if (router->override[key & 127]) {
        dir = router->override_direction[key & 127];
        key = router->override_command[key & 127];
    } else key = keymap(key, router->roguelike, &dir);
    if (!key || key == 27 || key == 13 || key == 10 || key == '-' || key == ' ')
        return SV_OK;
    if (key == ':') { router->chat = true; router->text_size = 0; return SV_OK; }
    if (key == ';' || key == '.' || key == '+') {
        output->kind = key == ';' ? SV_COMMAND_WALK : key == '.' ? SV_COMMAND_RUN :
            SV_COMMAND_TUNNEL;
        if (!dir) { router->direction_pending = output->kind; output->kind = SV_COMMAND_NONE; }
        else output->direction = dir;
        return SV_OK;
    }
    if (key == ',' && dir == 5) { output->kind = SV_COMMAND_STAND; return SV_OK; }
    output->kind = SV_COMMAND_RAW; output->key = key;
    return SV_OK;
}

SvResult sv_command_dispatch(SvCommandRouter *router, SvProtocol *protocol,
                             unsigned char key)
{
    if (!router || !protocol) return SV_INVALID;
    SvCommandRouter next = *router;
    SvCommand command;
    SvResult result = sv_command_key(&next, key, &command);
    if (result != SV_OK) return result;
    switch (command.kind) {
    case SV_COMMAND_NONE: break;
    case SV_COMMAND_RAW:
        result = sv_protocol_raw_key(protocol, command.key); break;
    case SV_COMMAND_WALK: case SV_COMMAND_RUN: case SV_COMMAND_TUNNEL:
        result = sv_protocol_direction(protocol,
            command.kind == SV_COMMAND_WALK ? PKT_WALK :
            command.kind == SV_COMMAND_RUN ? PKT_RUN : PKT_TUNNEL,
            command.direction);
        break;
    case SV_COMMAND_STAND: result = sv_protocol_stand(protocol); break;
    case SV_COMMAND_CHAT:
        result = command.size ? sv_protocol_chat(protocol, command.text, command.size) : SV_OK;
        break;
    }
    if (result == SV_OK) *router = next;
    return result;
}
