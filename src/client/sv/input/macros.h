#ifndef SV_MACROS_H
#define SV_MACROS_H
#include "input/input.h"

#define SV_MACRO_LIMIT 512
#define SV_MACRO_TRIGGER 64
#define SV_MACRO_ACTION 1024
#define SV_MACRO_QUEUE 4096

typedef struct {
    unsigned char trigger[SV_MACRO_TRIGGER], action[SV_MACRO_ACTION];
    size_t trigger_size, action_size;
    SvMacroKind kind;
} SvMacroDefinition;
typedef struct {
    SvMacroDefinition definitions[SV_MACRO_LIMIT];
    size_t count;
} SvMacroSet;
typedef struct {
    unsigned char fresh[SV_MACRO_QUEUE], ready[SV_MACRO_QUEUE];
    unsigned char fresh_resolved[SV_MACRO_QUEUE];
    size_t fresh_count, ready_count;
    size_t action_index;
    const SvMacroDefinition *active;
    SvMacroDefinition direct;
    bool direct_pending;
    uint64_t match_deadline_ms, wait_deadline_ms;
    bool waiting, extended_wait, semaphore, confirmed;
    bool action_trigger;
} SvMacroRunner;

SvResult sv_macros_define(SvMacroSet *set, const unsigned char *trigger, size_t trigger_size,
                          const unsigned char *action, size_t action_size, SvMacroKind kind);
SvResult sv_macros_delete(SvMacroSet *set, const unsigned char *trigger, size_t trigger_size);
SvResult sv_macros_queue_action(SvMacroRunner *runner, const unsigned char *action, size_t size);
void sv_macros_reset(SvMacroRunner *runner);
SvResult sv_macros_feed(const SvMacroSet *set, SvMacroRunner *runner,
                        const unsigned char *bytes, size_t size, bool command,
                        bool message, bool shopping, bool allow_stores, uint64_t now_ms);
SvResult sv_macros_feed_resolved(const SvMacroSet *set, SvMacroRunner *runner,
                                unsigned char key, bool command, bool message,
                                bool shopping, bool allow_stores, uint64_t now_ms);
SvResult sv_macros_pump(const SvMacroSet *set, SvMacroRunner *runner, bool command,
                        bool message, bool shopping, bool allow_stores, uint64_t now_ms);
SvResult sv_macros_peek(const SvMacroRunner *runner, unsigned char *key);
void sv_macros_consume(SvMacroRunner *runner);
#endif
