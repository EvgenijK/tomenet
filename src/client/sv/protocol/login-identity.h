#ifndef SV_PROTOCOL_LOGIN_IDENTITY_H
#define SV_PROTOCOL_LOGIN_IDENTITY_H
#include <stdbool.h>

/* SDL3 baseline's deterministic six-byte iaddr for versioned first login. */
bool sv_login_identity(const char *profile_root, unsigned char iaddr[6]);
#endif
