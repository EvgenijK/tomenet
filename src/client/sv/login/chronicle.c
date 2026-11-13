#include "login/chronicle.h"
#include <curl/curl.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SvChronicle {
    SDL_Mutex *mutex;
    SDL_AtomicInt refs, cancelled;
    SvChronicleSnapshot snapshot;
    unsigned delivered;
    /* Worker-owned source caches survive retries; successful refresh replaces
     * its source rather than duplicating yesterday's rows in the merged view. */
    SvChronicleEvent sources[9][SV_CHRONICLE_SOURCE_EVENTS];
    size_t source_counts[9];
};

static void release(SvChronicle *provider)
{
    if (SDL_AddAtomicInt(&provider->refs, -1) == 1) {
        SDL_DestroyMutex(provider->mutex);
        SDL_free(provider);
    }
}

static const char *find(const char *at, const char *end, const char *word)
{
    size_t length = strlen(word);
    for (; (size_t)(end - at) >= length; ++at)
        if (!SDL_strncasecmp(at, word, length)) return at;
    return NULL;
}

static bool valid_date(int year, int month, int day)
{
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (year < 1970 || year > 9999 || month < 1 || month > 12 || day < 1) return false;
    return day <= days[month - 1] + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

/* Text only: tags never reach the renderer. Entity expansion is bounded and
 * numeric references are restricted to Unicode scalar values. */
static bool append(char *line, size_t *length, Uint32 cp)
{
    if (cp < 32 || cp == 127) cp = ' ';
    if (cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) cp = 0xfffd;
    unsigned char bytes[4];
    size_t n;
    if (cp < 128) { bytes[0] = (unsigned char)cp; n = 1; }
    else if (cp < 2048) { bytes[0] = 0xc0 | (cp >> 6); bytes[1] = 0x80 | (cp & 63); n = 2; }
    else if (cp < 65536) { bytes[0] = 0xe0 | (cp >> 12); bytes[1] = 0x80 | ((cp >> 6) & 63); bytes[2] = 0x80 | (cp & 63); n = 3; }
    else { bytes[0] = 0xf0 | (cp >> 18); bytes[1] = 0x80 | ((cp >> 12) & 63); bytes[2] = 0x80 | ((cp >> 6) & 63); bytes[3] = 0x80 | (cp & 63); n = 4; }
    if (*length + n >= 1024) return false;
    memcpy(line + *length, bytes, n); *length += n;
    return true;
}

static void parse_line(char *line, size_t length, bool overflow, Uint32 color,
                       SvChronicleType type, unsigned server, int *year, int *previous_month,
                       SvChronicleEvent *events, size_t *count, unsigned order)
{
    if (overflow || *count == SV_CHRONICLE_SOURCE_EVENTS) return;
    line[length] = 0;
    char *text = line;
    while (isspace((unsigned char)*text)) ++text;
    length = strlen(text);
    while (length && isspace((unsigned char)text[length - 1])) text[--length] = 0;
    SvChronicleEvent event = {.type = type, .server = server, .color = color, .source_order = order};
    int y = *year, m = 0, d = 0, offset = 0;
    if (type == SV_CHRONICLE_DEATH) {
        static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        char month[4];
        if (sscanf(text, "%2d %3s - %n", &d, month, &offset) != 2 || offset != 9) return;
        for (int i = 0; i < 12; ++i) if (!memcmp(months + i * 3, month, 3)) m = i + 1;
        if (!m) return;
        if (m > *previous_month) --y;
        if (!valid_date(y, m, d)) return;
        *year = y; *previous_month = m;
        SDL_snprintf(event.date_label, sizeof(event.date_label), "%02d %.3s", d, month);
    } else {
        if (strlen(text) < 12 || sscanf(text, "%4d-%2d-%2d%n", &y, &m, &d, &offset) != 3 ||
            offset != 10 || text[4] != '-' || text[7] != '-' ||
            !isspace((unsigned char)text[10]) || !valid_date(y, m, d)) return;
        while (isspace((unsigned char)text[offset])) ++offset;
        if (text[offset] == ':') ++offset;
        while (isspace((unsigned char)text[offset])) ++offset;
    }
    text += offset;
    if (!*text || strlen(text) >= sizeof(event.text)) return;
    memcpy(event.text, text, strlen(text) + 1);
    SDL_snprintf(event.date, sizeof(event.date), "%04d-%02d-%02d", y, m, d);
    if (type != SV_CHRONICLE_DEATH) SDL_strlcpy(event.date_label, event.date, sizeof(event.date_label));
    else {
        const char *level = strchr(text, '(');
        unsigned value = 0;
        if (level) (void)sscanf(level + 1, "%u", &value);
        event.color = value < 20 ? 0x777777 : value < 40 ? 0xb70000 : value < 60 ? 0xff3030 : 0xff7043;
    }
    events[(*count)++] = event;
}

bool sv_chronicle_parse(const char *html, size_t size, SvChronicleType type,
                        unsigned server, int now_year, int now_month,
                        SvChronicleEvent events[SV_CHRONICLE_SOURCE_EVENTS], size_t *count)
{
    if (!count) return false;
    *count = 0;
    if (!html || !events || size > SV_CHRONICLE_BODY_LIMIT || memchr(html, 0, size) ||
        type > SV_CHRONICLE_SUPERUNIQUE || type < SV_CHRONICLE_DEATH || server > 2 ||
        !valid_date(now_year, now_month, 1)) return false;
    const char *end = html + size, *at = find(html, end, "<pre");
    if (at) {
        at = memchr(at, '>', (size_t)(end - at));
        if (!at) return false;
        ++at;
        end = find(at, end, "</pre>");
    } else {
        at = find(html, end, "</h2>");
        if (!at) return false;
        at += 5;
        end = find(at, end, "<div id=\"footer\"");
    }
    if (!end) return false;
    char line[1024]; size_t length = 0;
    Uint32 color = 0xc6c5b9;
    bool overflow = false;
    unsigned order = 0;
    int year = now_year, previous_month = now_month;
    while (at < end && *count < SV_CHRONICLE_SOURCE_EVENTS) {
        bool newline = false;
        if (*at == '<') {
            const char *close = memchr(at, '>', (size_t)(end - at));
            if (!close) return false;
            newline = close - at >= 3 && !SDL_strncasecmp(at, "<br", 3) &&
                      (at[3] == '>' || at[3] == '/' || isspace((unsigned char)at[3]));
            if (close - at >= 5 && !SDL_strncasecmp(at, "<span", 5)) {
                const char *value = find(at, close, "color:");
                if (value) {
                    value += 6;
                    while (value < close && isspace((unsigned char)*value)) ++value;
                    if (close - value >= 7 && *value++ == '#') {
                        Uint32 rgb = 0; bool valid = true;
                        for (int i = 0; i < 6; ++i) {
                            unsigned char c = (unsigned char)value[i];
                            if (!isxdigit(c)) { valid = false; break; }
                            rgb = (rgb << 4) | (Uint32)(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10);
                        }
                        if (valid) color = rgb;
                    }
                }
            }
            at = close + 1;
        } else if (*at == '\n') { ++at; newline = true; }
        else if (*at == '\r') ++at;
        else if (*at == '&') {
            const char *semi = memchr(at, ';', (size_t)(end - at) < 16 ? (size_t)(end - at) : 16);
            Uint32 cp = 0;
            if (semi) {
                char entity[16]; size_t n = (size_t)(semi - at - 1);
                memcpy(entity, at + 1, n); entity[n] = 0;
                if (!strcmp(entity, "amp")) cp = '&';
                else if (!strcmp(entity, "lt")) cp = '<';
                else if (!strcmp(entity, "gt")) cp = '>';
                else if (!strcmp(entity, "apos")) cp = '\'';
                else if (!strcmp(entity, "quot")) cp = '"';
                else if (!strcmp(entity, "nbsp")) cp = ' ';
                else if (entity[0] == '#') {
                    char *tail; bool hex = entity[1] == 'x' || entity[1] == 'X';
                    const char *digits = entity + (hex ? 2 : 1);
                    unsigned long value = strtoul(digits, &tail, hex ? 16 : 10);
                    if (*digits && !*tail && value && value <= 0x10ffff) cp = (Uint32)value;
                }
            }
            if (cp) { if (!append(line, &length, cp)) overflow = true; at = semi + 1; }
            else { if (!append(line, &length, '&')) overflow = true; ++at; }
        } else {
            unsigned char c = (unsigned char)*at++;
            if (length + 1 < sizeof(line)) line[length++] = c < 32 || c == 127 ? ' ' : (char)c;
            else overflow = true;
        }
        if (newline) {
            parse_line(line, length, overflow, color, type, server, &year, &previous_month, events, count, order++);
            length = 0; overflow = false; color = 0xc6c5b9;
        }
    }
    if (length) parse_line(line, length, overflow, color, type, server, &year, &previous_month, events, count, order);
    return *count != 0;
}

typedef struct {
    CURL *easy;
    SvChronicle *owner;
    char *body;
    size_t length;
    unsigned server;
    SvChronicleType type;
    char url[160];
} Source;

static size_t receive(char *data, size_t unit, size_t count, void *context)
{
    Source *source = context;
    if (SDL_GetAtomicInt(&source->owner->cancelled) ||
        (unit && count > SIZE_MAX / unit)) return 0;
    size_t size = unit * count;
    if (size > SV_CHRONICLE_BODY_LIMIT - source->length) return 0;
    memcpy(source->body + source->length, data, size);
    source->length += size;
    return size;
}

static int compare(const void *left, const void *right)
{
    const SvChronicleEvent *a = left, *b = right;
    int order = strcmp(b->date, a->date);
    if (order) return order;
    if (a->source_order != b->source_order) return a->source_order < b->source_order ? -1 : 1;
    if (a->server != b->server) return a->server < b->server ? -1 : 1;
    return (int)a->type - (int)b->type;
}

static void publish(SvChronicle *provider, Source *source, bool transport_ok,
                    int year, int month, bool finished)
{
    SvChronicleEvent rows[SV_CHRONICLE_SOURCE_EVENTS];
    size_t count = 0;
    bool ok = transport_ok && sv_chronicle_parse(source->body, source->length,
                    source->type, source->server, year, month, rows, &count);
    unsigned index = source->server * 3 + (unsigned)source->type;
    if (ok) {
        memcpy(provider->sources[index], rows, count * sizeof(*rows));
        provider->source_counts[index] = count;
    }
    SDL_LockMutex(provider->mutex);
    SvChronicleSnapshot *view = &provider->snapshot;
    if (ok) {
        /* Failed sources retain their last successful data. Rebuild from the
         * bounded source caches so changed/removed events cannot accumulate. */
        SvChronicleEvent merged[9 * SV_CHRONICLE_SOURCE_EVENTS];
        size_t total = 0;
        for (unsigned i = 0; i < 9; ++i) {
            size_t n = provider->source_counts[i];
            memcpy(merged + total, provider->sources[i], n * sizeof(*merged));
            total += n;
        }
        qsort(merged, total, sizeof(*merged), compare);
        view->count = total < SV_CHRONICLE_EVENTS ? total : SV_CHRONICLE_EVENTS;
        memcpy(view->events, merged, view->count * sizeof(*merged));
    } else ++view->failed_sources;
    ++view->completed_sources;
    view->loading = !finished;
    ++view->revision;
    SDL_UnlockMutex(provider->mutex);
    SDL_Log("SV Chronicle source=%s status=%s events=%zu bytes=%zu", source->url,
            ok ? "ready" : "unavailable", count, source->length);
}

static void refresh(SvChronicle *provider)
{
    static const char *hosts[] = {"europe.tomenet.eu", "na.tomenet.eu", "apac.tomenet.eu"};
    Source sources[9] = {{0}};
    CURLM *multi = curl_multi_init();
    SDL_Time now = 0; SDL_DateTime date = {0};
    (void)SDL_GetCurrentTime(&now);
    (void)SDL_TimeToDateTime(now, &date, false);
    unsigned completed = 0;
    for (unsigned i = 0; i < 9; ++i) {
        Source *source = &sources[i];
        source->owner = provider;
        source->server = i / 3;
        source->type = (SvChronicleType)(i % 3);
        const char *path = source->type == SV_CHRONICLE_DEATH ? "deaths.php" :
            source->type == SV_CHRONICLE_OCCURRENCE ? "legends.php" : "superuniques.php";
        SDL_snprintf(source->url, sizeof(source->url), "https://www.tomenet.eu/%s?server=%s", path, hosts[source->server]);
        source->body = SDL_malloc(SV_CHRONICLE_BODY_LIMIT);
        source->easy = multi && source->body ? curl_easy_init() : NULL;
        if (source->easy) {
#define SET(option, value) do { if (curl_easy_setopt(source->easy, option, value) != CURLE_OK) goto source_failed; } while (0)
            SET(CURLOPT_URL, source->url);
            SET(CURLOPT_USERAGENT, "TomeNET-SV/1.0 Chronicle");
            SET(CURLOPT_NOSIGNAL, 1L);
            SET(CURLOPT_CONNECTTIMEOUT_MS, 5000L);
            SET(CURLOPT_TIMEOUT_MS, 12000L);
            SET(CURLOPT_PROTOCOLS_STR, "https");
            SET(CURLOPT_FOLLOWLOCATION, 0L);
            SET(CURLOPT_FAILONERROR, 1L);
            SET(CURLOPT_WRITEFUNCTION, receive);
            SET(CURLOPT_WRITEDATA, source);
            SET(CURLOPT_PRIVATE, source);
#undef SET
            if (curl_multi_add_handle(multi, source->easy) == CURLM_OK) continue;
        }
source_failed:
        if (source->easy) curl_easy_cleanup(source->easy);
        source->easy = NULL;
        publish(provider, source, false, date.year, date.month, ++completed == 9);
    }
    int running = 0;
    CURLMcode result = CURLM_OK;
    Uint64 started = SDL_GetTicks();
    while (multi && completed < 9 && !SDL_GetAtomicInt(&provider->cancelled) &&
           SDL_GetTicks() - started < 13000) {
        result = curl_multi_perform(multi, &running);
        if (result != CURLM_OK) break;
        int remaining;
        CURLMsg *message;
        while ((message = curl_multi_info_read(multi, &remaining))) {
            if (message->msg != CURLMSG_DONE) continue;
            Source *source = NULL;
            (void)curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &source);
            if (!source) continue;
            long status = 0;
            (void)curl_easy_getinfo(source->easy, CURLINFO_RESPONSE_CODE, &status);
            publish(provider, source, message->data.result == CURLE_OK && status == 200,
                    date.year, date.month, ++completed == 9);
            (void)curl_multi_remove_handle(multi, source->easy);
            curl_easy_cleanup(source->easy);
            source->easy = NULL;
        }
        if (running) (void)curl_multi_poll(multi, NULL, 0, 100, NULL);
        else break;
    }
    for (unsigned i = 0; i < 9; ++i) {
        Source *source = &sources[i];
        if (source->easy) {
            if (!SDL_GetAtomicInt(&provider->cancelled))
                publish(provider, source, false, date.year, date.month, ++completed == 9);
            (void)curl_multi_remove_handle(multi, source->easy);
            curl_easy_cleanup(source->easy);
        }
        SDL_free(source->body);
    }
    if (multi) curl_multi_cleanup(multi);
}

static int worker(void *data)
{
    SvChronicle *provider = data;
    while (!SDL_GetAtomicInt(&provider->cancelled)) {
        Uint64 started = SDL_GetTicks();
        SDL_LockMutex(provider->mutex);
        provider->snapshot.loading = true;
        provider->snapshot.completed_sources = 0;
        provider->snapshot.failed_sources = 0;
        ++provider->snapshot.revision;
        SDL_UnlockMutex(provider->mutex);
        refresh(provider);
        /* One worker owns all requests. Slow batches never overlap; ordinary
         * batches begin every five seconds, including after source failures. */
        while (!SDL_GetAtomicInt(&provider->cancelled) &&
               SDL_GetTicks() - started < 5000) SDL_Delay(50);
    }
    release(provider);
    return 0;
}

SvChronicle *sv_chronicle_start(void)
{
    /* Called by the main owner before creating our worker. No global cleanup:
     * cancellation releases UI ownership immediately while resolver cleanup
     * may still be running in this detached, reference-counted worker. */
    static bool initialized; /* start is main-owner-only, before worker creation. */
    if (!initialized) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) return NULL;
        initialized = true;
    }
    SvChronicle *provider = SDL_calloc(1, sizeof(*provider));
    if (!provider) return NULL;
    provider->mutex = SDL_CreateMutex();
    if (!provider->mutex) { SDL_free(provider); return NULL; }
    provider->snapshot.loading = true;
    provider->snapshot.revision = 1;
    SDL_SetAtomicInt(&provider->refs, 2);
    SDL_Thread *thread = SDL_CreateThread(worker, "sv-chronicle", provider);
    if (!thread) { SDL_DestroyMutex(provider->mutex); SDL_free(provider); return NULL; }
    SDL_DetachThread(thread);
    return provider;
}

bool sv_chronicle_poll(SvChronicle *provider, SvChronicleSnapshot *snapshot)
{
    if (!provider || !snapshot) return false;
    SDL_LockMutex(provider->mutex);
    bool changed = provider->snapshot.revision != provider->delivered;
    if (changed) {
        *snapshot = provider->snapshot;
        provider->delivered = provider->snapshot.revision;
    }
    SDL_UnlockMutex(provider->mutex);
    return changed;
}

void sv_chronicle_stop(SvChronicle *provider)
{
    if (!provider) return;
    SDL_SetAtomicInt(&provider->cancelled, 1);
    release(provider);
}
