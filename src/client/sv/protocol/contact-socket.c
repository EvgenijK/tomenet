#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "protocol/contact-socket.h"
#include "protocol/protocol.h"
#include "app.h"
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
    SvApp *app;
    uint64_t generation;
    SvSocket socket;
    SvSocketState state;
    int worker_done;
    char host[256], service[8];
    unsigned char sending[SV_PROTOCOL_CAPACITY];
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
    if (state != SV_SOCKET_NEGOTIATING && state != SV_SOCKET_READY) return state;
    if (state == SV_SOCKET_READY && !connection->app) {
        connection->app = sv_app_create((SvAlertSink){0});
        if (!connection->app || sv_app_open(connection->app,
                sv_contact_version(connection->contact)) != SV_OK)
            return connection->state = SV_SOCKET_PROTOCOL_ERROR;
        connection->generation = sv_app_view(connection->app).generation;
        SvOutput needed = sv_contact_take_remaining(connection->contact, NULL, 0);
        if (needed.result != SV_OK && needed.result != SV_OUTPUT_TOO_SMALL)
            return connection->state = SV_SOCKET_PROTOCOL_ERROR;
        unsigned char *remaining = SDL_malloc(needed.size ? needed.size : 1);
        if (!remaining) return connection->state = SV_SOCKET_PROTOCOL_ERROR;
        SvOutput extra = sv_contact_take_remaining(connection->contact,
                                                    remaining, needed.size);
        if (extra.result != SV_OK) {
            SDL_free(remaining);
            return connection->state = SV_SOCKET_PROTOCOL_ERROR;
        }
        for (size_t at = 0; at < extra.size;) {
            size_t capacity = sv_app_receive_capacity(connection->app);
            if (!capacity) {
                SvStep step = sv_app_step(connection->app, 64);
                if (step.result != SV_OK && step.result != SV_WAITING &&
                    step.result != SV_RECOVERED) {
                    SDL_free(remaining);
                    return connection->state = SV_SOCKET_PROTOCOL_ERROR;
                }
                capacity = sv_app_receive_capacity(connection->app);
                if (!capacity) {
                    SDL_free(remaining);
                    return connection->state = SV_SOCKET_PROTOCOL_ERROR;
                }
            }
            size_t count = extra.size - at;
            if (count > capacity) count = capacity;
            if (sv_app_receive(connection->app, connection->generation,
                               remaining + at, count) != SV_OK) {
                SDL_free(remaining);
                return connection->state = SV_SOCKET_PROTOCOL_ERROR;
            }
            at += count;
        }
        SDL_free(remaining);
    }
    if (state == SV_SOCKET_READY && sv_app_frame(connection->app,
            connection->generation, SDL_GetTicks()) != SV_OK)
        return connection->state = SV_SOCKET_PROTOCOL_ERROR;
    if (state == SV_SOCKET_NEGOTIATING && SDL_GetTicks() - connection->last_progress >
        (sv_contact_state(connection->contact) == SV_CONTACT_WAIT_CONTACT ? 10000u : 5000u))
        return connection->state = SV_SOCKET_TIMEOUT;
    if (connection->sent == connection->send_size) {
        if (state == SV_SOCKET_READY)
            (void)sv_app_keepalive(connection->app, connection->generation,
                                   SDL_GetTicks(), connection->last_sent);
        SvOutput output = state == SV_SOCKET_READY ?
            sv_app_take_output(connection->app, connection->generation,
                               connection->sending, sizeof(connection->sending)) :
            sv_contact_take_output(connection->contact, connection->sending,
                                   sizeof(connection->sending));
        if (output.result == SV_OK) { connection->send_size = output.size; connection->sent = 0; }
    }
    if (connection->sent < connection->send_size) {
        int sent = send(connection->socket, (const char *)connection->sending + connection->sent,
                        (int)(connection->send_size - connection->sent),
#ifdef MSG_NOSIGNAL
                        MSG_NOSIGNAL);
#else
                        0);
#endif
        if (sent > 0) {
            connection->sent += (size_t)sent;
            connection->last_progress = connection->last_sent = SDL_GetTicks();
        }
        else if (sent == 0 || !sv_would_block(sv_last_error))
            return connection->state = SV_SOCKET_CLOSED;
    }
    unsigned char incoming[4096];
    for (int i = 0; i < 4; ++i) {
        size_t capacity = state == SV_SOCKET_READY ? sv_app_receive_capacity(connection->app) : sizeof(incoming);
        if (!capacity) break;
        if (capacity > sizeof(incoming)) capacity = sizeof(incoming);
        int received = recv(connection->socket, (char *)incoming, (int)capacity, 0);
        if (received > 0) {
            connection->last_progress = SDL_GetTicks();
            SvResult parsed = state == SV_SOCKET_READY ?
                sv_app_receive(connection->app, connection->generation, incoming, (size_t)received) :
                sv_contact_receive(connection->contact, incoming, (size_t)received);
            if (parsed != SV_OK) {
                SvContactState phase = sv_contact_failure_phase(connection->contact);
                if (phase == SV_CONTACT_WAIT_CONTACT && sv_contact_rejection(connection->contact))
                    return connection->state = SV_SOCKET_REJECTED;
                return connection->state = phase == SV_CONTACT_WAIT_VERIFY ? SV_SOCKET_VERIFY_ERROR :
                    phase == SV_CONTACT_WAIT_SETUP ? SV_SOCKET_SETUP_ERROR : SV_SOCKET_PROTOCOL_ERROR;
            }
            if (state == SV_SOCKET_NEGOTIATING && sv_contact_state(connection->contact) == SV_CONTACT_READY) {
                connection->state = SV_SOCKET_READY;
                return sv_contact_socket_poll(connection);
            }
        } else if (received == 0) {
            if (state == SV_SOCKET_NEGOTIATING) {
                SvContactState phase = sv_contact_state(connection->contact);
                if (phase == SV_CONTACT_WAIT_VERIFY) return connection->state = SV_SOCKET_VERIFY_ERROR;
                if (phase == SV_CONTACT_WAIT_SETUP) return connection->state = SV_SOCKET_SETUP_ERROR;
            }
            return connection->state = SV_SOCKET_CLOSED;
        }
        else if (sv_would_block(sv_last_error)) break;
        else return connection->state = SV_SOCKET_CLOSED;
    }
    if (state == SV_SOCKET_READY) {
        SvStep step = sv_app_step(connection->app, 64);
        if (step.result != SV_OK && step.result != SV_WAITING &&
            step.result != SV_BACKPRESSURE && step.result != SV_RECOVERED)
            return connection->state = SV_SOCKET_PROTOCOL_ERROR;
    }
    return connection->state;
}
unsigned sv_contact_socket_rejection(const SvContactSocket *connection)
{
    return sv_contact_rejection(connection->contact);
}
const int *sv_contact_socket_version(const SvContactSocket *connection)
{
    return sv_contact_version(connection->contact);
}
void sv_contact_socket_stop(SvContactSocket *connection)
{
    if (!connection) return;
    SDL_SetAtomicInt(&connection->cancelled, 1);
    if (connection->mutex) SDL_LockMutex(connection->mutex);
    if (connection->socket != SV_BAD_SOCKET) sv_close(connection->socket);
    if (connection->mutex) SDL_UnlockMutex(connection->mutex);
    sv_contact_destroy(connection->contact);
    (void)sv_app_destroy(connection->app);
    if (connection->mutex) release(connection);
    else SDL_free(connection);
}
