#include "protocol/login-identity.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **argv)
{
    assert(argc == 3);
    unsigned char first[6], same[6], other[6];
    assert(sv_login_identity(argv[1], first));
    assert(sv_login_identity(argv[1], same));
    assert(sv_login_identity(argv[2], other));
    assert(first[0] == 0xf4 && first[1] == 0x43);
    assert(!memcmp(first, same, sizeof(first)));
    assert(memcmp(first, other, sizeof(first)));
    (void)SDL_GetError();
    return 0;
}
