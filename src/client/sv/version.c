#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../common/angband.h"
#include "../../common/version-newer.inc"
