/* Synthetic-only external transport. Packet_scanf/printf are production code.
 * Locked receive/output buffers must never reach a real socket. */
#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../common/angband.h"
#include <errno.h>
bool is_client_side = TRUE;
bool rl_connection_destroyed = FALSE;
bool rl_connection_destructible = FALSE, rl_connection_state = FALSE;
int DgramRead(int fd, char *data, int size)
{
    (void)fd; (void)data; (void)size;
    errno = EIO;
    return -1;
}
int DgramWrite(int fd, char *data, int size)
{
    (void)fd; (void)data; (void)size;
    errno = EIO;
    return -1;
}
int GetSocketError(int fd) { (void)fd; return -1; }
