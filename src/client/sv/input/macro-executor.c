#include "input/macro-executor.h"
#include <string.h>

void sv_macro_executor_reset(SvMacroExecutor *executor)
{
    sv_macros_reset(executor->runner);
    *executor->state = (SvMacroExecution){0};
}

bool sv_macro_executor_idle(const SvMacroExecutor *executor)
{
    const SvMacroRunner *runner = executor->runner;
    return !runner->active && !runner->fresh_count && !runner->ready_count && !runner->waiting;
}

bool sv_macro_executor_active(const SvMacroExecutor *executor)
{
    const SvMacroRunner *runner = executor->runner;
    return executor->macros->count || runner->direct_pending || runner->active ||
           runner->fresh_count || runner->ready_count;
}

bool sv_macro_executor_extended_waiting(const SvMacroExecutor *executor)
{
    return executor->runner->waiting && executor->runner->extended_wait;
}

bool sv_macro_executor_waiting(const SvMacroExecutor *executor)
{
    return executor->runner->waiting;
}

void sv_macro_executor_key_request(SvMacroExecutor *executor)
{
    if (executor->runner->waiting) executor->runner->semaphore = true;
}

SvResult sv_macro_executor_frame(SvMacroExecutor *executor, SvKeyRequest request,
    SvInputContext context, bool paused, uint64_t now_ms, size_t budget)
{
    SvMacroRunner *runner = executor->runner;
    SvInputRouter *input = executor->input;
    uint64_t *owner = &executor->state->confirmation_owner;
    SvResult result;
    if (*owner) {
        unsigned char command;
        result = sv_input_take_confirmation(input, *owner, &command);
        if (result == SV_OK) runner->confirmed = true;
        else if (result != SV_WAITING) return result;
        if (!runner->waiting || runner->confirmed || runner->semaphore ||
            now_ms >= runner->wait_deadline_ms) {
            result = sv_input_end_confirmation(input, *owner);
            if (result != SV_OK) return result;
            *owner = 0;
        }
    }
    result = sv_macros_pump(executor->macros, runner,
        !request.pending, executor->command->chat, false, true, now_ms);
    if (result != SV_OK && result != SV_WAITING) return result;
    if (runner->waiting && !*owner) {
        result = sv_input_begin_confirmation(input, owner);
        if (result != SV_OK) return result;
    }
    for (size_t i = 0; i < budget; ++i) {
        unsigned char key;
        if (sv_macros_peek(runner, &key) != SV_OK) break;
        if (request.pending) {
            if (input->count) break;
            result = sv_input_enqueue_resolved(input, request, request.sequence, key);
            if (result != SV_OK) return result;
        } else {
            if (paused || context != SV_CONTEXT_GAME) return SV_BUSY;
            result = sv_command_dispatch(executor->command, executor->protocol, key);
            if (result != SV_OK) return result;
        }
        sv_macros_consume(runner);
        if (request.pending) break;
    }
    return SV_OK;
}

SvMacroPhysicalRoute sv_macro_executor_physical(SvMacroExecutor *executor,
    const unsigned char *bytes, size_t size, bool prompt, uint64_t now_ms)
{
    SvMacroPhysicalRoute route = {0};
    unsigned char key;
    route.result = sv_input_physical(executor->bindings, bytes, size, prompt, &key);
    if (route.result != SV_OK) {
        if (route.result != SV_WAITING || !size || bytes[0] != 31) return route;
        const unsigned char *end = memchr(bytes, 13, size);
        if (!end) return route;
        size_t trigger_size = (size_t)(end - bytes) + 1;
        bool has_default = size == trigger_size + 3 && bytes[trigger_size] == 28 &&
            bytes[size - 1] == 28;
        if (size != trigger_size && !has_default) return route;
        bool mapped = false;
        for (size_t i = 0; i < executor->macros->count; ++i) {
            const SvMacroDefinition *entry = &executor->macros->definitions[i];
            if (entry->trigger_size == trigger_size &&
                (!prompt || entry->kind != SV_MACRO_COMMAND) &&
                (!executor->command->chat || entry->kind == SV_MACRO_NORMAL) &&
                !memcmp(entry->trigger, bytes, trigger_size)) {
                mapped = true; break;
            }
        }
        if (!mapped) {
            /* SDL's encoded special key may carry one default action between
             * 28 delimiters.  It is physical fallback, not macro expansion. */
            if (has_default) {
                unsigned char fallback = bytes[trigger_size + 1];
                const SvMacroRunner *runner = executor->runner;
                if (runner->waiting || runner->active || runner->direct_pending ||
                    runner->fresh_count || runner->ready_count) {
                    route.result = sv_macros_feed_resolved(executor->macros, executor->runner,
                        fallback, !prompt, executor->command->chat, false, true, now_ms);
                    route.pump = route.result == SV_OK || route.result == SV_WAITING;
                } else {
                    route.result = SV_OK;
                    route.key = fallback;
                }
            }
            return route;
        }
        route.result = sv_macros_feed(executor->macros, executor->runner, bytes, trigger_size,
            !prompt, executor->command->chat, false, true, now_ms);
        route.pump = route.result == SV_OK || route.result == SV_WAITING;
        return route;
    }
    if (sv_macro_executor_active(executor)) {
        route.result = sv_macros_feed(executor->macros, executor->runner, &key, 1,
            !prompt, executor->command->chat, false, true, now_ms);
        route.pump = route.result == SV_OK || route.result == SV_WAITING;
        return route;
    }
    route.key = key;
    return route;
}

SvResult sv_macro_executor_accept(SvMacroExecutor *executor, unsigned char key,
                                  uint64_t now_ms)
{
    return sv_macros_feed(executor->macros, executor->runner, &key, 1,
        false, false, false, true, now_ms);
}
