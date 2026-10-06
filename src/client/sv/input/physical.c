#include "input/physical.h"
#include <stdio.h>
#include <string.h>

static int keysym(SDL_Keycode key)
{
    if (key >= SDLK_F1 && key <= SDLK_F12) return 0xffbe + (int)(key - SDLK_F1);
    switch (key) {
    case SDLK_UP: return 0xff52;
    case SDLK_DOWN: return 0xff54;
    case SDLK_LEFT: return 0xff51;
    case SDLK_RIGHT: return 0xff53;
    case SDLK_INSERT: return 0xff63;
    case SDLK_PAGEUP: return 0xff55;
    case SDLK_PAGEDOWN: return 0xff56;
    case SDLK_HOME: return 0xff50;
    case SDLK_END: return 0xff57;
    case SDLK_DELETE: return 127;
    case SDLK_PRINTSCREEN: return 0xff61;
    case SDLK_SCROLLLOCK: return 0xff14;
    case SDLK_PAUSE: return 0xff13;
    case SDLK_ESCAPE: return 27;
    case SDLK_RETURN: return 13;
    case SDLK_TAB: return 9;
    case SDLK_LEFT_TAB: return 9;
    case SDLK_BACKSPACE: return 8;
    case SDLK_SPACE: return 32;
    default: return -1;
    }
}

static int keypad(SDL_Scancode code)
{
    switch (code) {
    case SDL_SCANCODE_KP_0: return 0xff9e;
    case SDL_SCANCODE_KP_1: return 0xff9c;
    case SDL_SCANCODE_KP_2: return 0xff99;
    case SDL_SCANCODE_KP_3: return 0xff9b;
    case SDL_SCANCODE_KP_4: return 0xff96;
    case SDL_SCANCODE_KP_5: return 0xff9d;
    case SDL_SCANCODE_KP_6: return 0xff98;
    case SDL_SCANCODE_KP_7: return 0xff95;
    case SDL_SCANCODE_KP_8: return 0xff97;
    case SDL_SCANCODE_KP_9: return 0xff9a;
    case SDL_SCANCODE_KP_PERIOD: return 0xff9f;
    case SDL_SCANCODE_KP_DIVIDE: return 0xffaf;
    case SDL_SCANCODE_KP_MULTIPLY: return 0xffaa;
    case SDL_SCANCODE_KP_MINUS: return 0xffad;
    case SDL_SCANCODE_KP_PLUS: return 0xffab;
    case SDL_SCANCODE_KP_ENTER: return 0xff8d;
    default: return -1;
    }
}

static bool special_plain(SDL_Keycode key)
{
    return key == SDLK_ESCAPE || key == SDLK_RETURN || key == SDLK_TAB || key == SDLK_LEFT_TAB ||
           key == SDLK_BACKSPACE || key == SDLK_DELETE || key == SDLK_SPACE;
}

static bool sequence(SvPhysicalKey *out, int sym, bool scancode)
{
    char encoded[32];
    unsigned pos = 0;
    encoded[pos++] = 31;
    if (out->modifiers & SDL_KMOD_CTRL) encoded[pos++] = 'N';
    if (out->modifiers & SDL_KMOD_SHIFT) encoded[pos++] = 'S';
    if (out->modifiers & SDL_KMOD_ALT) encoded[pos++] = 'O';
    if (scancode) encoded[pos++] = 'K';
    int count = snprintf(encoded + pos, sizeof(encoded) - pos, "_%X\r", (unsigned)sym);
    if (count <= 0 || (size_t)count >= sizeof(encoded) - pos) return false;
    out->size = pos + (size_t)count;
    memcpy(out->bytes, encoded, out->size);
    return true;
}

bool sv_physical_key(const SDL_KeyboardEvent *event, SDL_Keymod latched,
                     SvPhysicalKey *out)
{
    if (!event || !out || event->repeat || event->scancode == SDL_SCANCODE_UNKNOWN) return false;
    *out = (SvPhysicalKey){.key = event->key,
                           .modifiers = (SDL_Keymod)(event->mod | latched)};
    int sym = keysym(event->key);
    if (sym < 0) sym = keypad(event->scancode);
    if (sym >= 0) {
        if ((event->key == SDLK_TAB || event->key == SDLK_LEFT_TAB) &&
            (out->modifiers & (SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT)))
            return sequence(out, 0xfe20, false);
        if (special_plain(event->key) && event->scancode != SDL_SCANCODE_KP_ENTER) {
            if (out->modifiers & (SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT)) {
                if (!sequence(out,sym,false) || out->size + 3 > sizeof(out->bytes)) return false;
                out->bytes[out->size++] = 28;
                out->bytes[out->size++] = (unsigned char)sym;
                out->bytes[out->size++] = 28;
                return true;
            }
            out->bytes[0] = (unsigned char)sym;
            out->size = 1;
            return true;
        }
        return sequence(out, sym, false);
    }
    if (event->key == SDLK_UNKNOWN) return sequence(out, (int)event->scancode, true);
    if (out->modifiers & (SDL_KMOD_CTRL | SDL_KMOD_ALT)) {
        if ((out->modifiers & SDL_KMOD_CTRL) && !(out->modifiers & (SDL_KMOD_SHIFT | SDL_KMOD_ALT)) &&
            event->key >= SDLK_A && event->key <= SDLK_Z) {
            out->bytes[0] = (unsigned char)(event->key - SDLK_A + 1);
            out->size = 1;
            return true;
        }
        if (!sequence(out, (int)event->key, false)) return false;
        if (out->size + 3 > sizeof(out->bytes)) return false;
        out->bytes[out->size++] = 28;
        out->bytes[out->size++] = (unsigned char)event->key;
        out->bytes[out->size++] = 28;
        return true;
    }
    if (event->key < 32 || event->key > 255) return false;
    out->bytes[0] = (unsigned char)event->key;
    out->size = 1;
    return true;
}
int sv_physical_symbol(const unsigned char *bytes, size_t size)
{
    if (!bytes || !size) return -1;
    if (size == 1) return bytes[0];
    if (bytes[0] != 31) return -1;
    size_t at = 1;
    while (at < size && (bytes[at] == 'N' || bytes[at] == 'S' || bytes[at] == 'O')) ++at;
    if (at < size && bytes[at] == 'K') ++at;
    if (at >= size || bytes[at++] != '_') return -1;
    unsigned symbol = 0;
    size_t digits = 0;
    while (at < size && bytes[at] != 13) {
        unsigned c = bytes[at++];
        unsigned hex = c >= '0' && c <= '9' ? c - '0' :
                       c >= 'A' && c <= 'F' ? c - 'A' + 10 : 16;
        if (hex == 16 || ++digits > 6) return -1;
        symbol = symbol * 16 + hex;
    }
    if (!digits || at >= size || bytes[at] != 13) return -1;
    return (int)symbol;
}
