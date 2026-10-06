#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "input/metaserver.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SvSocket;
#define SV_BAD_SOCKET INVALID_SOCKET
#define sv_close closesocket
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
#endif

struct SvMetaserver {
    SDL_Mutex *mutex;
    SDL_AtomicInt refs;
    SDL_AtomicInt cancelled;
    SvServer servers[SV_SERVER_COUNT];
    size_t count;
    unsigned revision, delivered;
    bool failed;
    char hosts[3][SV_HOST_LIMIT + 1];
    uint16_t ports[3];
    size_t host_count;
};

static void release(SvMetaserver *provider)
{
    if (SDL_AddAtomicInt(&provider->refs, -1) == 1) {
        /* SDL mutexes and allocation are core facilities independent of the
         * video subsystem; a cancelled resolver can finish after its window. */
        SDL_DestroyMutex(provider->mutex);
        SDL_free(provider);
    }
}

static bool host_char(unsigned char c)
{
    return isalnum(c) || c == '.' || c == '-' || c == ':' || c == '_';
}

static bool attr(const char *begin, const char *end, const char *name,
                 char *value, size_t capacity)
{
    size_t wanted = strlen(name);
    for (const char *at = begin; at < end;) {
        while (at < end && isspace((unsigned char)*at)) ++at;
        const char *key = at;
        while (at < end && (isalnum((unsigned char)*at) || *at == '_')) ++at;
        size_t length = (size_t)(at - key);
        while (at < end && isspace((unsigned char)*at)) ++at;
        if (at >= end || *at != '=') {
            while (at < end && !isspace((unsigned char)*at)) ++at;
            continue;
        }
        ++at;
        while (at < end && isspace((unsigned char)*at)) ++at;
        if (at >= end || (*at != '"' && *at != '\'')) return false;
        char quote = *at++;
        const char *start = at;
        while (at < end && *at != quote) ++at;
        if (at == end) return false;
        size_t size = (size_t)(at - start);
        ++at;
        if (length == wanted && !memcmp(key, name, wanted)) {
            if (!size || size >= capacity) return false;
            memcpy(value, start, size);
            value[size] = 0;
            return true;
        }
    }
    return false;
}

bool sv_metaserver_parse(const char *xml, size_t size, SvServer *servers, size_t *count)
{
    if (!xml || !servers || !count || size > 80192) return false;
    if (memchr(xml, 0, size)) return false;
    *count = 0;
    const char *end = xml + size;
    for (const char *at = xml; at < end;) {
        if (*at++ != '<' || end - at < 7 || memcmp(at, "server", 6) ||
            !isspace((unsigned char)at[6])) continue;
        at += 6;
        const char *tag = at;
        char quote = 0;
        while (at < end) {
            if (quote) { if (*at == quote) quote = 0; }
            else if (*at == '"' || *at == '\'') quote = *at;
            else if (*at == '>') break;
            ++at;
        }
        if (at == end || *count == SV_SERVER_COUNT) return false;
        SvServer row = {.ping_ms = -1};
        char port[12], protocol[12];
        if (!attr(tag, at, "url", row.host, sizeof(row.host)) ||
            !attr(tag, at, "port", port, sizeof(port))) return false;
        if (row.host[0] == '-' || !row.host[0]) return false;
        for (const unsigned char *p = (const unsigned char *)row.host; *p; ++p)
            if (!host_char(*p)) return false;
        char *tail;
        unsigned long parsed = strtoul(port, &tail, 10);
        if (*tail || !parsed || parsed > 65535) return false;
        row.port = (uint16_t)parsed;
        if (attr(tag, at, "protocol", protocol, sizeof(protocol))) {
            long version = strtol(protocol, &tail, 10);
            if (*tail || version < 0 || version > 255) return false;
            row.protocol = (int)version;
        }
        memcpy(row.label, row.host, strlen(row.host) + 1);
        servers[(*count)++] = row;
        ++at;
    }
    return true;
}

static bool wait_socket(SvMetaserver *provider, SvSocket socket, bool write_ready)
{
    for (int attempt = 0; attempt < 10 && !SDL_GetAtomicInt(&provider->cancelled); ++attempt) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(socket, &fds);
        struct timeval timeout = {0, 200000};
        int ready = select((int)socket + 1, write_ready ? NULL : &fds,
                           write_ready ? &fds : NULL, NULL, &timeout);
        if (ready > 0) return true;
        if (ready < 0) return false;
    }
    return false;
}

static bool fetch(SvMetaserver *provider, const char *host, uint16_t port,
                  char *feed, size_t *length)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) return false;
#endif
    char service[8];
    SDL_snprintf(service, sizeof(service), "%u", (unsigned)port);
    struct addrinfo hints = {0}, *addresses = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    bool ok = false;
    if (getaddrinfo(host, service, &hints, &addresses)) goto finish;
    for (struct addrinfo *a = addresses; a && !SDL_GetAtomicInt(&provider->cancelled); a = a->ai_next) {
        SvSocket descriptor = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
        if (descriptor == SV_BAD_SOCKET) continue;
#ifdef _WIN32
        u_long nonblocking = 1;
        ioctlsocket(descriptor, FIONBIO, &nonblocking);
#else
        fcntl(descriptor, F_SETFL, fcntl(descriptor, F_GETFL, 0) | O_NONBLOCK);
#endif
        (void)connect(descriptor, a->ai_addr, (int)a->ai_addrlen);
        if (wait_socket(provider, descriptor, true)) {
            int error = 0;
#ifdef _WIN32
            int error_size = sizeof(error);
#else
            socklen_t error_size = sizeof(error);
#endif
            if (!getsockopt(descriptor, SOL_SOCKET, SO_ERROR, (char *)&error, &error_size) && !error) {
                *length = 0;
                uint64_t started = SDL_GetTicks();
                while (*length < 80192 && SDL_GetTicks() - started < 10000 &&
                       wait_socket(provider, descriptor, false)) {
                    int read = recv(descriptor, feed + *length, (int)(80192 - *length), 0);
                    if (read > 0) *length += (size_t)read;
                    else break;
                }
                ok = *length != 0 && *length < 80192 &&
                     !SDL_GetAtomicInt(&provider->cancelled);
            }
        }
        sv_close(descriptor);
        if (ok) break;
    }
finish:
    if (addresses) freeaddrinfo(addresses);
#ifdef _WIN32
    WSACleanup();
#endif
    return ok;
}

static int ping_host(SvMetaserver *provider, const char *host)
{
#ifdef _WIN32
    const char *args[] = {"ping", "-n", "1", "-w", "1000", host, NULL};
#else
    const char *args[] = {"ping", "-n", "-c", "1", "-W", "1", "--", host, NULL};
#endif
    SDL_Process *process = SDL_CreateProcess(args, true);
    if (!process) return -2;
    SDL_IOStream *pipe = SDL_GetProcessOutput(process);
    char output[4096] = {0};
    size_t size = 0;
    int status = -1, ping = -2;
    bool exited = false;
    uint64_t deadline = SDL_GetTicks() + 2000;
    while (pipe && !exited && !SDL_GetAtomicInt(&provider->cancelled) &&
           SDL_GetTicks() < deadline) {
        size_t got = SDL_ReadIO(pipe, output + size, sizeof(output) - size - 1);
        size += got;
        exited = SDL_WaitProcess(process, false, &status);
        if (!exited) SDL_Delay(10);
    }
    if (!exited) {
        SDL_KillProcess(process, true);
        (void)SDL_WaitProcess(process, true, &status);
    } else if (pipe && size + 1 < sizeof(output)) {
        size += SDL_ReadIO(pipe, output + size, sizeof(output) - size - 1);
    }
    output[size] = 0;
    if (exited && status == 0) {
        const char *at = strstr(output, "time=");
        if (at) {
            char *end;
            double ms = strtod(at + 5, &end);
            if (end != at + 5 && ms >= 0 && ms < 10000) ping = (int)(ms + .5);
        } else if (strstr(output, "time<")) ping = 1;
    }
    SDL_DestroyProcess(process);
    return ping;
}

static int worker(void *data)
{
    SvMetaserver *provider = data;
    char *feed = SDL_malloc(80192);
    size_t length = 0, count = 0;
    SvServer rows[SV_SERVER_COUNT] = {{0}};
    bool ok = false;
    for (size_t i = 0; feed && i < provider->host_count &&
            !SDL_GetAtomicInt(&provider->cancelled); ++i) {
        length = 0;
        if (fetch(provider, provider->hosts[i], provider->ports[i], feed, &length) &&
            sv_metaserver_parse(feed, length, rows, &count)) { ok = true; break; }
    }
    SDL_free(feed);
    if (SDL_GetAtomicInt(&provider->cancelled)) { release(provider); return 0; }
    SDL_LockMutex(provider->mutex);
    if (ok) {
        memcpy(provider->servers, rows, count * sizeof(*rows));
        provider->count = count;
    } else provider->failed = true;
    ++provider->revision;
    SDL_UnlockMutex(provider->mutex);
    for (size_t i = 0; ok && i < count && !SDL_GetAtomicInt(&provider->cancelled); ++i) {
        int ping = ping_host(provider, rows[i].host);
        if (SDL_GetAtomicInt(&provider->cancelled)) break;
        SDL_LockMutex(provider->mutex);
        provider->servers[i].ping_ms = ping;
        ++provider->revision;
        SDL_UnlockMutex(provider->mutex);
    }
    release(provider);
    return 0;
}

SvMetaserver *sv_metaserver_start(const char *host, uint16_t port)
{
    if (!host || !*host || strlen(host) > SV_HOST_LIMIT || !port) return NULL;
    SvMetaserver *provider = SDL_calloc(1, sizeof(*provider));
    if (!provider) return NULL;
    provider->mutex = SDL_CreateMutex();
    if (!provider->mutex) { SDL_free(provider); return NULL; }
    strcpy(provider->hosts[provider->host_count], host);
    provider->ports[provider->host_count++] = port;
    if (strcmp(host, "meta.tomenet.eu") || port != 8801) {
        strcpy(provider->hosts[provider->host_count], "meta.tomenet.eu");
        provider->ports[provider->host_count++] = 8801;
    }
    if (strcmp(host, "37.187.75.24") || port != 8801) {
        strcpy(provider->hosts[provider->host_count], "37.187.75.24");
        provider->ports[provider->host_count++] = 8801;
    }
    SDL_SetAtomicInt(&provider->refs, 2);
    SDL_Thread *thread = SDL_CreateThread(worker, "sv-metaserver", provider);
    if (!thread) { SDL_DestroyMutex(provider->mutex); SDL_free(provider); return NULL; }
    SDL_DetachThread(thread);
    return provider;
}

bool sv_metaserver_poll(SvMetaserver *provider, SvEndpoint *endpoint, bool *failed)
{
    if (!provider || !endpoint || !failed) return false;
    SDL_LockMutex(provider->mutex);
    bool changed = provider->revision != provider->delivered;
    if (changed) {
        provider->delivered = provider->revision;
        *failed = provider->failed || !sv_endpoint_servers(endpoint, provider->servers, provider->count);
    }
    SDL_UnlockMutex(provider->mutex);
    return changed;
}

void sv_metaserver_stop(SvMetaserver *provider)
{
    if (!provider) return;
    SDL_SetAtomicInt(&provider->cancelled, 1);
    release(provider);
}
