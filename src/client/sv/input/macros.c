#include "input/macros.h"
#include <string.h>

static bool active(const SvMacroDefinition *definition, bool command, bool message,
                   bool shopping, bool allow_stores)
{
    if (message || (shopping && !allow_stores))
        return definition->kind == SV_MACRO_NORMAL;
    return definition->kind != SV_MACRO_COMMAND || command;
}

SvResult sv_macros_define(SvMacroSet *set, const unsigned char *trigger, size_t trigger_size,
                          const unsigned char *action, size_t action_size, SvMacroKind kind)
{
    if (!set || !trigger || !action || !trigger_size || trigger_size > SV_MACRO_TRIGGER ||
        action_size > SV_MACRO_ACTION || kind < SV_MACRO_NORMAL || kind > SV_MACRO_COMMAND ||
        memchr(trigger, 0, trigger_size) || memchr(action, 0, action_size)) return SV_INVALID;
    size_t index = 0;
    for (; index < set->count; ++index)
        if (set->definitions[index].trigger_size == trigger_size &&
            !memcmp(set->definitions[index].trigger, trigger, trigger_size)) break;
    if (index == set->count) {
        if (set->count == SV_MACRO_LIMIT) return SV_INPUT_OVERFLOW;
        ++set->count;
    }
    SvMacroDefinition *entry = &set->definitions[index];
    memcpy(entry->trigger, trigger, trigger_size);
    memcpy(entry->action, action, action_size);
    entry->trigger_size = trigger_size;
    entry->action_size = action_size;
    entry->kind = kind;
    return SV_OK;
}

SvResult sv_macros_delete(SvMacroSet *set, const unsigned char *trigger, size_t trigger_size)
{
    if (!set || !trigger || !trigger_size || trigger_size > SV_MACRO_TRIGGER) return SV_INVALID;
    for (size_t i = 0; i < set->count; ++i) {
        if (set->definitions[i].trigger_size != trigger_size ||
            memcmp(set->definitions[i].trigger, trigger, trigger_size)) continue;
        memmove(&set->definitions[i], &set->definitions[i + 1],
                (set->count - i - 1) * sizeof(set->definitions[0]));
        --set->count;
        return SV_OK;
    }
    return SV_WAITING;
}

void sv_macros_reset(SvMacroRunner *runner)
{
    if (runner) *runner = (SvMacroRunner){0};
}

SvResult sv_macros_queue_action(SvMacroRunner *runner, const unsigned char *action, size_t size)
{
    if (!runner || !action || !size || memchr(action, 0, size)) return SV_INVALID;
    size_t start = runner->active == &runner->direct ? runner->action_index : 0;
    size_t remaining = runner->direct.action_size - start;
    if (size > SV_MACRO_ACTION - remaining) return SV_INPUT_OVERFLOW;
    memmove(runner->direct.action + size, runner->direct.action + start, remaining);
    memcpy(runner->direct.action, action, size);
    runner->direct.action_size = size + remaining;
    if (runner->active == &runner->direct) runner->action_index = 0;
    runner->direct_pending = true;
    return SV_OK;
}

static void consume_fresh(SvMacroRunner *runner, size_t count)
{
    runner->fresh_count -= count;
    memmove(runner->fresh, runner->fresh + count, runner->fresh_count);
    runner->match_deadline_ms = 0;
}

static SvResult ready(SvMacroRunner *runner, unsigned char key)
{
    if (runner->ready_count == SV_MACRO_QUEUE) return SV_INPUT_OVERFLOW;
    runner->ready[runner->ready_count++] = key;
    return SV_OK;
}

static unsigned decimal(const unsigned char *action, size_t size, size_t *index, unsigned digits)
{
    unsigned value = 0;
    for (unsigned i = 0; i < digits; ++i) {
        unsigned char byte = *index < size ? action[(*index)++] : '0';
        value = value * 10 + (byte >= '0' && byte <= '9' ? byte - '0' : 0);
    }
    return value;
}

SvResult sv_macros_pump(const SvMacroSet *set, SvMacroRunner *runner, bool command,
                        bool message, bool shopping, bool allow_stores, uint64_t now_ms)
{
    if (!set || !runner) return SV_INVALID;
    for (;;) {
        if (runner->waiting) {
            if (now_ms < runner->wait_deadline_ms && !runner->semaphore && !runner->confirmed)
                return SV_WAITING;
            runner->waiting = runner->extended_wait = runner->semaphore = runner->confirmed = false;
        }
        if (runner->active) {
            if (runner->action_index == runner->active->action_size) {
                if (runner->active == &runner->direct) {
                    runner->direct.action_size = 0;
                    runner->direct_pending = false;
                }
                runner->active = NULL;
                runner->action_index = 0;
                continue;
            }
            unsigned char byte = runner->active->action[runner->action_index++];
            if (byte == 29) {
                if (runner->active == &runner->direct) {
                    runner->direct.action_size = 0;
                    runner->direct_pending = false;
                }
                runner->active = NULL;
                continue;
            } /* completion sentinel */
            if (byte == 96 || byte == 30) {
                unsigned digits = byte == 96 ? 2 : 4;
                unsigned tenths = decimal(runner->active->action,
                    runner->active->action_size, &runner->action_index, digits);
                uint64_t duration = (uint64_t)tenths * 100;
                runner->wait_deadline_ms = duration > UINT64_MAX - now_ms ?
                    UINT64_MAX : now_ms + duration;
                runner->waiting = true;
                runner->extended_wait = byte == 30;
                runner->semaphore = runner->confirmed = false;
                continue;
            }
            SvResult result = ready(runner, byte);
            if (result != SV_OK) return result;
            continue;
        }
        if (runner->direct_pending) {
            runner->active = &runner->direct;
            runner->action_index = 0;
            continue;
        }
        if (!runner->fresh_count) return SV_OK;
        size_t best = set->count, best_size = 0;
        bool longer = false;
        for (size_t i = 0; i < set->count; ++i) {
            const SvMacroDefinition *entry = &set->definitions[i];
            if (!active(entry, command, message, shopping, allow_stores)) continue;
            size_t common = entry->trigger_size < runner->fresh_count ?
                entry->trigger_size : runner->fresh_count;
            if (memcmp(entry->trigger, runner->fresh, common)) continue;
            if (entry->trigger_size > runner->fresh_count) longer = true;
            else if (entry->trigger_size >= best_size) {
                best = i; best_size = entry->trigger_size;
            }
        }
        if (longer && (!runner->match_deadline_ms || now_ms < runner->match_deadline_ms)) {
            if (!runner->match_deadline_ms) runner->match_deadline_ms =
                now_ms > UINT64_MAX - 500 ? UINT64_MAX : now_ms + 500;
            return SV_WAITING;
        }
        if (best != set->count) {
            consume_fresh(runner, best_size);
            runner->active = &set->definitions[best];
            runner->action_index = 0;
            continue;
        }
        unsigned char key = runner->fresh[0];
        consume_fresh(runner, 1);
        SvResult result = ready(runner, key);
        if (result != SV_OK) return result;
    }
}

SvResult sv_macros_feed(const SvMacroSet *set, SvMacroRunner *runner,
                        const unsigned char *bytes, size_t size, bool command,
                        bool message, bool shopping, bool allow_stores, uint64_t now_ms)
{
    if (!set || !runner || (!bytes && size)) return SV_INVALID;
    if (runner->waiting && runner->extended_wait) {
        for (size_t i = 0; i < size; ++i) {
            if (bytes[i] == 27) { sv_macros_reset(runner); return SV_OK; }
            if (bytes[i] == ' ') {
                runner->waiting = runner->extended_wait = false;
                return sv_macros_pump(set, runner, command, message, shopping, allow_stores, now_ms);
            }
        }
    }
    if (size > SV_MACRO_QUEUE - runner->fresh_count) return SV_INPUT_OVERFLOW;
    memcpy(runner->fresh + runner->fresh_count, bytes, size);
    runner->fresh_count += size;
    return sv_macros_pump(set, runner, command, message, shopping, allow_stores, now_ms);
}

SvResult sv_macros_peek(const SvMacroRunner *runner, unsigned char *key)
{
    if (!runner || !key) return SV_INVALID;
    if (!runner->ready_count) return SV_WAITING;
    *key = runner->ready[0];
    return SV_OK;
}

void sv_macros_consume(SvMacroRunner *runner)
{
    if (!runner || !runner->ready_count) return;
    --runner->ready_count;
    memmove(runner->ready, runner->ready + 1, runner->ready_count);
}
