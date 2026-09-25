#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "protocol/contact-socket.h"
#include <limits.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SvSocket;
#define SV_BAD_SOCKET INVALID_SOCKET
#define sv_close closesocket
#define sv_last_error WSAGetLastError()
#define sv_would_block(error) ((error) == WSAEWOULDBLOCK)
#else
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int SvSocket;
#define SV_BAD_SOCKET (-1)
#define sv_close close
#define sv_last_error errno
#define sv_would_block(error) ((error) == EWOULDBLOCK || (error) == EAGAIN)
#endif

struct SvContactSocket {
    SDL_Mutex *mutex;
    SDL_AtomicInt refs, cancelled;
    SvContact *contact;
    SvSocket socket;
    SvSocketState state;
    int worker_done;
    char host[256], service[8];
    unsigned char sending[SV_CONTACT_OUTPUT_CAPACITY];
    size_t send_size, sent;
    uint64_t last_progress;
    uint64_t last_sent;
#ifdef _WIN32
    int winsock_started;
#endif
};

static void release(SvContactSocket *connection)
{
    if (SDL_AddAtomicInt(&connection->refs, -1) == 1) {
#ifdef _WIN32
        if (connection->winsock_started) WSACleanup();
#endif
        SDL_DestroyMutex(connection->mutex);
        volatile unsigned char *bytes = (void *)connection;
        for (size_t i = 0; i < sizeof(*connection); ++i) bytes[i] = 0;
        SDL_free(connection);
    }
}
static int nonblocking(SvSocket socket)
{
#ifdef _WIN32
    u_long enabled = 1;
    return ioctlsocket(socket, FIONBIO, &enabled) == 0;
#else
    int flags = fcntl(socket, F_GETFL, 0);
    return flags >= 0 && fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}
static int wait_connect(SvContactSocket *connection, SvSocket socket)
{
    for (int i = 0; i < 25 && !SDL_GetAtomicInt(&connection->cancelled); ++i) {
        fd_set writable;
        FD_ZERO(&writable); FD_SET(socket, &writable);
        struct timeval timeout = {0, 200000};
        int ready = select((int)socket + 1, NULL, &writable, NULL, &timeout);
        if (ready > 0) {
            int error = 0;
#ifdef _WIN32
            int size = sizeof(error);
#else
            socklen_t size = sizeof(error);
#endif
            return !getsockopt(socket, SOL_SOCKET, SO_ERROR, (char *)&error, &size) && !error ? 1 : -1;
        }
        if (ready < 0) return -1;
    }
    return 0;
}
static int resolve_and_connect(void *context)
{
    SvContactSocket *connection = context;
    struct addrinfo hints = {0}, *addresses = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    SvSocketState state = SV_SOCKET_CONNECT_ERROR;
    SvSocket connected = SV_BAD_SOCKET;
    if (getaddrinfo(connection->host, connection->service, &hints, &addresses)) {
        state = SV_SOCKET_DNS_ERROR;
        goto done;
    }
    SDL_LockMutex(connection->mutex);
    if (!SDL_GetAtomicInt(&connection->cancelled)) connection->state = SV_SOCKET_CONNECTING;
    SDL_UnlockMutex(connection->mutex);
    for (struct addrinfo *address = addresses;
         address && !SDL_GetAtomicInt(&connection->cancelled); address = address->ai_next) {
        SvSocket descriptor = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (descriptor == SV_BAD_SOCKET) continue;
        if (!nonblocking(descriptor)) { sv_close(descriptor); continue; }
        int result = connect(descriptor, address->ai_addr, (int)address->ai_addrlen);
        if (result != 0) {
            int error = sv_last_error;
#ifdef _WIN32
            if (error != WSAEINPROGRESS && error != WSAEWOULDBLOCK) {
#else
            if (error != EINPROGRESS && error != EWOULDBLOCK) {
#endif
                sv_close(descriptor); continue;
            }
            result = wait_connect(connection, descriptor);
        } else result = 1;
        if (result == 1 && !SDL_GetAtomicInt(&connection->cancelled)) {
            connected = descriptor;
            break;
        }
        if (result == 0) state = SV_SOCKET_TIMEOUT;
        sv_close(descriptor);
    }
done:
    if (addresses) freeaddrinfo(addresses);
    SDL_LockMutex(connection->mutex);
    if (SDL_GetAtomicInt(&connection->cancelled)) {
        if (connected != SV_BAD_SOCKET) sv_close(connected);
    } else {
        connection->socket = connected;
        connection->state = connected != SV_BAD_SOCKET ? SV_SOCKET_NEGOTIATING : state;
        connection->last_progress = SDL_GetTicks();
        connection->last_sent = connection->last_progress;
    }
    connection->worker_done = 1;
    SDL_UnlockMutex(connection->mutex);
    release(connection);
    return 0;
}
SvContactSocket *sv_contact_socket_start(const char *host, uint16_t port, int protocol,
                                         const SvContactIdentity *identity)
{
    if (!host || !*host || strlen(host) >= 256 || !port) return NULL;
    SvContactSocket *connection = SDL_calloc(1, sizeof(*connection));
    if (!connection) return NULL;
    SDL_SetAtomicInt(&connection->refs, 1);
    connection->socket = SV_BAD_SOCKET;
    connection->contact = sv_contact_create(protocol, identity);
    connection->mutex = SDL_CreateMutex();
    if (!connection->contact || !connection->mutex) goto failed;
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) goto failed;
    connection->winsock_started = 1;
#endif
    memcpy(connection->host, host, strlen(host) + 1);
    SDL_snprintf(connection->service, sizeof(connection->service), "%u", (unsigned)port);
    SDL_SetAtomicInt(&connection->cancelled, 0);
    connection->state = SV_SOCKET_RESOLVING;
    SDL_AddAtomicInt(&connection->refs, 1);
    SDL_Thread *worker = SDL_CreateThread(resolve_and_connect, "sv-contact", connection);
    if (!worker) { SDL_AddAtomicInt(&connection->refs, -1); goto failed; }
    SDL_DetachThread(worker);
    return connection;
failed:
    sv_contact_socket_stop(connection);
    return NULL;
}
SvSocketState sv_contact_socket_poll(SvContactSocket *connection)
{
    SDL_LockMutex(connection->mutex);
    SvSocketState state = connection->state;
    SDL_UnlockMutex(connection->mutex);
    if (state != SV_SOCKET_NEGOTIATING) return state;
    if (SDL_GetTicks() - connection->last_progress >
        (sv_contact_state(connection->contact) == SV_CONTACT_WAIT_CONTACT ? 10000u : 5000u))
        return connection->state = SV_SOCKET_TIMEOUT;
    if (connection->sent == connection->send_size) {
        SvOutput output = sv_contact_take_output(connection->contact,
            connection->sending, sizeof(connection->sending));
        if (output.result == SV_OK) {
            connection->send_size = output.size;
            connection->sent = 0;
        }
    }
    if (connection->sent < connection->send_size) {
        SvOutput sent = sv_contact_socket_write(connection,
            connection->sending + connection->sent, connection->send_size - connection->sent);
        if (sent.result == SV_CLOSED) return connection->state = SV_SOCKET_CLOSED;
        if (sent.result == SV_OK) connection->sent += sent.size;
    }
    unsigned char incoming[4096];
    for (int i = 0; i < 4; ++i) {
        int received = recv(connection->socket, (char *)incoming, sizeof(incoming), 0);
        if (received > 0) {
            connection->last_progress = SDL_GetTicks();
            SvResult parsed = sv_contact_receive(connection->contact, incoming, (size_t)received);
            if (parsed != SV_OK) {
                SvContactState phase = sv_contact_failure_phase(connection->contact);
                if (phase == SV_CONTACT_WAIT_CONTACT && sv_contact_rejection(connection->contact))
                    return connection->state = SV_SOCKET_REJECTED;
                return connection->state = phase == SV_CONTACT_WAIT_VERIFY ? SV_SOCKET_VERIFY_ERROR :
                    phase == SV_CONTACT_WAIT_SETUP ? SV_SOCKET_SETUP_ERROR : SV_SOCKET_PROTOCOL_ERROR;
            }
            if (sv_contact_state(connection->contact) == SV_CONTACT_READY)
                return connection->state = SV_SOCKET_READY;
        } else if (received == 0) {
            SvContactState phase = sv_contact_state(connection->contact);
            if (phase == SV_CONTACT_WAIT_VERIFY) return connection->state = SV_SOCKET_VERIFY_ERROR;
            if (phase == SV_CONTACT_WAIT_SETUP) return connection->state = SV_SOCKET_SETUP_ERROR;
            return connection->state = SV_SOCKET_CLOSED;
        } else if (sv_would_block(sv_last_error)) break;
        else return connection->state = SV_SOCKET_CLOSED;
    }
    return connection->state;
}

SvOutput sv_contact_socket_read(SvContactSocket *connection, void *bytes, size_t capacity)
{
    if (!connection || connection->state != SV_SOCKET_READY) return (SvOutput){SV_CLOSED, 0};
    if (!bytes || !capacity || capacity > INT_MAX) return (SvOutput){SV_INVALID, 0};
    int received = recv(connection->socket, bytes, (int)capacity, 0);
    if (received > 0) {
        connection->last_progress = SDL_GetTicks();
        return (SvOutput){SV_OK, (size_t)received};
    }
    if (received < 0 && sv_would_block(sv_last_error)) return (SvOutput){SV_WAITING, 0};
    connection->state = SV_SOCKET_CLOSED;
    return (SvOutput){SV_CLOSED, 0};
}

SvOutput sv_contact_socket_write(SvContactSocket *connection, const void *bytes, size_t size)
{
    if (!connection || (connection->state != SV_SOCKET_READY &&
                        connection->state != SV_SOCKET_NEGOTIATING))
        return (SvOutput){SV_CLOSED, 0};
    if (!bytes || !size || size > INT_MAX) return (SvOutput){SV_INVALID, 0};
    int sent = send(connection->socket, (const char *)bytes, (int)size,
#ifdef MSG_NOSIGNAL
                    MSG_NOSIGNAL);
#else
                    0);
#endif
    if (sent > 0) {
        connection->last_progress = connection->last_sent = SDL_GetTicks();
        return (SvOutput){SV_OK, (size_t)sent};
    }
    if (sent < 0 && sv_would_block(sv_last_error)) return (SvOutput){SV_WAITING, 0};
    connection->state = SV_SOCKET_CLOSED;
    return (SvOutput){SV_CLOSED, 0};
}

SvOutput sv_contact_socket_take_remaining(SvContactSocket *connection, void *bytes, size_t capacity)
{
    return sv_contact_take_remaining(connection->contact, bytes, capacity);
}

uint64_t sv_contact_socket_last_sent(const SvContactSocket *connection)
{
    return connection->last_sent;
}

unsigned sv_contact_socket_rejection(const SvContactSocket *connection)
{
    return sv_contact_rejection(connection->contact);
}
const int *sv_contact_socket_version(const SvContactSocket *connection)
{
    return sv_contact_version(connection->contact);
}
const SvContactSetup *sv_contact_socket_setup(const SvContactSocket *connection)
{
    return sv_contact_setup(connection->contact);
}
void sv_contact_socket_stop(SvContactSocket *connection)
{
    if (!connection) return;
    SDL_SetAtomicInt(&connection->cancelled, 1);
    if (connection->mutex) SDL_LockMutex(connection->mutex);
    if (connection->socket != SV_BAD_SOCKET) sv_close(connection->socket);
    if (connection->mutex) SDL_UnlockMutex(connection->mutex);
    sv_contact_destroy(connection->contact);
    if (connection->mutex) release(connection);
    else SDL_free(connection);
}
