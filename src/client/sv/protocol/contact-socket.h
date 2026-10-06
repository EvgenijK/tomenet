#ifndef SV_CONTACT_SOCKET_H
#define SV_CONTACT_SOCKET_H
#include "protocol/contact.h"
#include <SDL3/SDL.h>
typedef enum { SV_SOCKET_RESOLVING, SV_SOCKET_CONNECTING, SV_SOCKET_NEGOTIATING,
               SV_SOCKET_READY, SV_SOCKET_DNS_ERROR, SV_SOCKET_CONNECT_ERROR,
               SV_SOCKET_TIMEOUT, SV_SOCKET_CLOSED, SV_SOCKET_PROTOCOL_ERROR,
               SV_SOCKET_REJECTED, SV_SOCKET_VERIFY_ERROR, SV_SOCKET_SETUP_ERROR } SvSocketState;
typedef struct SvContactSocket SvContactSocket;
SvContactSocket *sv_contact_socket_start(const char *host, uint16_t port, int protocol,
                                         const SvContactIdentity *identity);
SvSocketState sv_contact_socket_poll(SvContactSocket *connection);
SvOutput sv_contact_socket_read(SvContactSocket *connection, void *bytes, size_t capacity);
SvOutput sv_contact_socket_write(SvContactSocket *connection, const void *bytes, size_t size);
SvOutput sv_contact_socket_take_remaining(SvContactSocket *connection, void *bytes, size_t capacity);
uint64_t sv_contact_socket_last_sent(const SvContactSocket *connection);
unsigned sv_contact_socket_rejection(const SvContactSocket *connection);
const int *sv_contact_socket_version(const SvContactSocket *connection);
const SvContactSetup *sv_contact_socket_setup(const SvContactSocket *connection);
void sv_contact_socket_stop(SvContactSocket *connection);
#endif
