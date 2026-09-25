#include "input/physical.h"
#include <assert.h>
#include <string.h>

static SDL_KeyboardEvent key(SDL_Keycode code, SDL_Scancode scan, SDL_Keymod mod)
{
    SDL_KeyboardEvent event = {0};
    event.key = code; event.scancode = scan; event.mod = mod;
    return event;
}

int main(void)
{
    SvPhysicalKey output;
    SDL_KeyboardEvent up = key(SDLK_UP,SDL_SCANCODE_UP,0);
    assert(sv_physical_key(&up,0,&output));
    const unsigned char up_bytes[] = {31,'_','F','F','5','2',13};
    assert(output.size == sizeof(up_bytes) && !memcmp(output.bytes,up_bytes,sizeof(up_bytes)));
    SDL_KeyboardEvent left = key(SDLK_LEFT,SDL_SCANCODE_LEFT,SDL_KMOD_CTRL);
    assert(sv_physical_key(&left,0,&output));
    const unsigned char left_bytes[] = {31,'N','_','F','F','5','1',13};
    assert(output.size == sizeof(left_bytes) && !memcmp(output.bytes,left_bytes,sizeof(left_bytes)));
    SDL_KeyboardEvent kp = key(SDLK_KP_8,SDL_SCANCODE_KP_8,SDL_KMOD_NUM);
    assert(sv_physical_key(&kp,0,&output));
    const unsigned char kp_bytes[] = {31,'_','F','F','9','7',13};
    assert(output.size == sizeof(kp_bytes) && !memcmp(output.bytes,kp_bytes,sizeof(kp_bytes)));
    SDL_KeyboardEvent control = key(SDLK_C,SDL_SCANCODE_C,SDL_KMOD_CTRL);
    assert(sv_physical_key(&control,0,&output));
    assert(output.size == 1 && output.bytes[0] == 3);
    SDL_KeyboardEvent lock = key(SDLK_SCROLLLOCK,SDL_SCANCODE_SCROLLLOCK,0);
    assert(sv_physical_key(&lock,0,&output));
    const unsigned char lock_bytes[] = {31,'_','F','F','1','4',13};
    assert(output.size == sizeof(lock_bytes) && !memcmp(output.bytes,lock_bytes,sizeof(lock_bytes)));
    SDL_KeyboardEvent tab = key(SDLK_TAB,SDL_SCANCODE_TAB,SDL_KMOD_CTRL);
    assert(sv_physical_key(&tab,0,&output));
    const unsigned char tab_bytes[] = {31,'N','_','F','E','2','0',13};
    assert(output.size == sizeof(tab_bytes) && !memcmp(output.bytes,tab_bytes,sizeof(tab_bytes)));
    return 0;
}
